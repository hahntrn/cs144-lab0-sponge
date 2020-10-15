#include "tcp_sender.hh"
#include "tcp_config.hh"
#include <random>
#include <iostream>
#include <algorithm>

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}, _rto{retx_timeout} // why is it {} and not ()
    , _stream(capacity), _outstanding_segments()
    , _timer { 0, retx_timeout, false } {}

uint64_t TCPSender::bytes_in_flight() const { return _n_bytes_in_flight; }

void TCPSender::fill_window() {
    if (_stream.buffer_empty() && _next_seqno > 0) return;
    //if (!_timer.started()) _timer.start();
    if (!_timer.running) _timer.start(_initial_retransmission_timeout);
    
    uint16_t n_bytes_to_send = _window_size == 0 ? 1 : min(_stream.buffer_size(), min(_window_size, TCPConfig::MAX_PAYLOAD_SIZE));

    TCPHeader hdr;
    cout << "next seqno: " << _next_seqno << endl;
    cout << "bytes left: " <<_stream.buffer_size()<<endl;
    hdr.seqno = wrap(_next_seqno, _isn);
    if (_next_seqno == 0) {
        cout << "set SYN" << endl;
        hdr.syn = true;
        if (n_bytes_to_send > 0) n_bytes_to_send--;
    }
    // TODO: when to set fin?
    cout << "n bytes to send: "<<n_bytes_to_send<<endl;
    if (_stream.eof()) {
        cout << "set FIN" << endl;
        hdr.fin = true;
        if (n_bytes_to_send > 0) n_bytes_to_send--;
    }
    // when to use TCPSegment::length_in_sequence_space()?
    TCPSegment seg;
    seg.header() = hdr;
    seg.payload() = _stream.read(n_bytes_to_send);
    cout<<"header: "<<seg.header().summary()<<endl;
    _segments_out.push(seg);
    _outstanding_segments[_next_seqno] = seg;
    _n_bytes_in_flight += n_bytes_to_send;
    _window_size -= n_bytes_to_send;
    cout<<"bytes in flight" <<_n_bytes_in_flight<<endl;
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    DUMMY_CODE(ackno, window_size); 
    _next_seqno = unwrap(ackno, _isn, _next_seqno);
    _window_size = window_size; // TODO: check this
    // TODO: remove from outstanding segments or resend?
    auto it = _outstanding_segments.begin();
    while (it != _outstanding_segments.end() &&
            it->first + it->second.length_in_sequence_space() >= _next_seqno) {
        _n_bytes_in_flight -= it->second.payload().size();
        it = _outstanding_segments.erase(it);
    }

    if (_outstanding_segments.empty()) {
        _rto = _initial_retransmission_timeout;
        if (_stream.buffer_size() > 0) _timer.start(_initial_retransmission_timeout);
        _n_consec_retransmissions = 0;
    }
    while (_window_size > 0) fill_window();
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    DUMMY_CODE(ms_since_last_tick); // TODO: implement in separate class?
    // if timer hasn't started do we still increment?
    if (_timer.running) _timer.time_elapsed += ms_since_last_tick;
    if (_timer.expired()) {
        // retransmit earliest segment not fully acknowledged
        if (!_outstanding_segments.empty()) {
            // TODO: decompose -> get earliest unacknowledged segment
            auto it = _outstanding_segments.begin();
            while (it != _outstanding_segments.end() && it->first + it->second.length_in_sequence_space() < _next_seqno) {
                it++;
            }
            _segments_out.push(it->second); // dereference bad?
        }
        if (_window_size != 0) {
            _n_consec_retransmissions++;
            _rto *= 2;
        }
        _timer.start(_timer.timeout*2);
    }

}

unsigned int TCPSender::consecutive_retransmissions() const { return _n_consec_retransmissions; }

//
//void TCPSender::send_empty_segment() { 
//    TCPHeader hdr;
//    hdr.seqno = wrap(_next_seqno, _isn);
//    TCPSegment seg;
//    seg.header() = hdr;
//    _segments_out.push(seg);
//}
//
//RetransmissionTimer::RetransmissionTimer(uint16_t timeout):
//    rto(timeout), _time_elapsed(0), _expired(true) {} 
//    // TODO: move _rto into retransmission timer, make an "expired" mode
//
//
//void RetransmissionTimer::reset() { _expired= false; _time_elapsed = 0; }
//bool RetransmissionTimer::expired() { return _expired; }
//
//void RetransmissionTimer::update(size_t time_diff) { _time_elapsed += time_diff; }
//
//size_t RetransmissionTimer::time_elapsed() { return _time_elapsed; }
