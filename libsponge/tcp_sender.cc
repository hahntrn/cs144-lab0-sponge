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
    , _initial_retransmission_timeout{retx_timeout} // why is it {} and not ()
    , _stream(capacity), _outstanding_segments()
    , _timer { 0, retx_timeout, false } {}

uint64_t TCPSender::bytes_in_flight() const { return _next_seqno - _abs_ackno; }
//uint64_t TCPSender::unfilled_window_size() const { return _abs_ackno + _window_size - _next_seqno; }

void TCPSender::fill_window() {
    _done_sending = false;
    if (!_timer.running) _timer.start(_initial_retransmission_timeout);
    uint16_t n_bytes_to_send = min(_window_size - bytes_in_flight(), TCPConfig::MAX_PAYLOAD_SIZE); //(_window_size < TCPConfig::MAX_PAYLOAD_SIZE ? _window_size : TCPConfig::MAX_PAYLOAD_SIZE);
    //if (n_bytes_to_send <= bytes_in_flight()) { _done_sending = true; return; } 
    //n_bytes_to_send -= bytes_in_flight();
    if (_window_size == 0 && bytes_in_flight() == 0) { cout<<"    ---- empty win! "<<endl; n_bytes_to_send = 1; }

    TCPHeader hdr;
    hdr.seqno = wrap(_next_seqno, _isn);
    if (_next_seqno == 0 && n_bytes_to_send > 0) {
        hdr.syn = true;
        n_bytes_to_send--;
    }
    //if (_stream.buffer_empty() && !hdr.syn && !hdr.fin) return;   
    TCPSegment seg;
    seg.payload() = _stream.buffer_empty() ? "" : _stream.read(n_bytes_to_send);
    n_bytes_to_send -= seg.payload().size();
    if (_stream.eof() && n_bytes_to_send > 0) {
        hdr.fin = true;
        n_bytes_to_send--;
    }
    seg.header() = hdr;
    if (seg.length_in_sequence_space() == 0) { _done_sending = true; return; }
    _segments_out.push(seg);
    _outstanding_segments[_next_seqno] = seg;
    //_window_size -= n_bytes_to_send;
    _next_seqno += seg.length_in_sequence_space();
    cout<<endl;
    cout<<"time elapsd "<<_timer.time_elapsed<<endl;
    cout<<"next seqno: " << _next_seqno << endl;
    cout<<"abs ackno:  "<<_abs_ackno<<endl;
    cout<<"window sz:  "<<_window_size<<endl;
    cout<<"bytes2send: "<<n_bytes_to_send<<endl;
    cout<<"bytes in flight: "<<bytes_in_flight()<<endl;
    cout<<"bytes just sent: "<<seg.length_in_sequence_space()<<endl;
    cout<<"bytes left in stream: " <<_stream.buffer_size()<<endl;
    cout<<"header: "<<seg.header().summary()<<endl;

}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    _window_size = window_size; // TODO: check this
    uint64_t new_abs_ackno = unwrap(ackno, _isn, _abs_ackno);
    if (new_abs_ackno <= _abs_ackno) return;
    _abs_ackno = new_abs_ackno;

    cout<<"ack received: "<<_abs_ackno<<endl;
    cout<<"bytes in flight: " << bytes_in_flight()<<endl;

    auto it = _outstanding_segments.begin();
    while (it != _outstanding_segments.end() &&
            it->first + it->second.length_in_sequence_space() <= _abs_ackno) {
        //cout<<"ack'd "<< it->second.payload().size()<<" bytes: "<<it->second.payload().str()<<endl;
        it = _outstanding_segments.erase(it);
    }
    _timer.start(_initial_retransmission_timeout);
    _n_consec_retransmissions = 0;
    while (_next_seqno <= _abs_ackno + window_size && _done_sending) fill_window();
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    cout<<"-tick!-"<<endl;
    // if timer hasn't started do we still increment?
    //if (_timer.running) _timer.time_elapsed += ms_since_last_tick;
    _timer.time_elapsed += ms_since_last_tick;
    if (_timer.expired()) {
        // retransmit earliest segment not fully acknowledged
        if (!_outstanding_segments.empty()) {
            // TODO: decompose -> get earliest unacknowledged segment
            auto it = _outstanding_segments.begin();
            //while (it != _outstanding_segments.end() && it->first + it->second.length_in_sequence_space() < _next_seqno) {
            //    it++;
            //}
            cout<<"resending segment at "<<it->first<<" with data "<<endl;
            _segments_out.push(it->second); // dereference bad?
        }
        if (_window_size > 0) {
            _n_consec_retransmissions++;
            _timer.timeout *= 2;
            cout<<"new rto: "<<_timer.timeout<<endl;
        }
        _timer.start(_timer.timeout);
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
