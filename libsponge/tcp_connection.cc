#include "tcp_connection.hh"
#include <iostream>

// Ha Tran
// CS 144 Lab 4
// 20201028W

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

size_t TCPConnection::remaining_outbound_capacity() const { return _sender.stream_in().remaining_capacity(); }

size_t TCPConnection::bytes_in_flight() const { return _sender.bytes_in_flight(); }

size_t TCPConnection::unassembled_bytes() const { return _receiver.unassembled_bytes(); }

size_t TCPConnection::time_since_last_segment_received() const { return _last_segm_recv_timer; }

void TCPConnection::try_switching_close_mode() {
    if (!_sender.stream_in().eof() && _receiver.stream_out().input_ended())
        _linger_after_streams_finish = false;
}

bool TCPConnection::syn_sent() { return _sender.next_seqno_absolute() > 0; }

void TCPConnection::segment_received(const TCPSegment &seg) {
    _receiver.segment_received(seg);
    _last_segm_recv_timer = 0;

    if (seg.header().rst) 
        reset();
    if (seg.header().ack)
        _sender.ack_received(seg.header().ackno, seg.header().win);
    
    if (syn_sent()) {
        // if segment non-empty and we need to return an ack 
        // but no segments are ready to be sent out
        // make sure at least one segment is sent in reply
        if (seg.length_in_sequence_space() > 0 && _sender.segments_out().empty()) {
            _sender.send_empty_segment();
        }
        send_segments();
    }

    if (seg.header().syn && !syn_sent())
        connect();
    try_switching_close_mode();
}

bool TCPConnection::active() const { return _active; }

void TCPConnection::send_segments(bool rst) {
    if (!active())
        return;

    while (!_sender.segments_out().empty()) {
        TCPSegment seg = _sender.segments_out().front();
        _sender.segments_out().pop();

        auto possible_ackno = _receiver.ackno();
        if (possible_ackno.has_value()) {
            seg.header().ack = true;
            seg.header().ackno = possible_ackno.value();
            seg.header().win = _receiver.window_size();
        }
        if (rst || _sender.consecutive_retransmissions() > TCPConfig::MAX_RETX_ATTEMPTS)
            seg.header().rst = true;

        _segments_out.push(seg);
    }
}

size_t TCPConnection::write(const string &data) {
    if (_sender.stream_in().input_ended() || !syn_sent())
        return 0;

    size_t n_bytes_written = _sender.stream_in().write(data);
    _sender.fill_window();
    send_segments();
    return n_bytes_written;
}

void TCPConnection::try_closing_connection() {
    if (!active())
        return;

    send_segments();
    bool active_close = _linger_after_streams_finish && _last_segm_recv_timer >= 10 * _cfg.rt_timeout;
    bool passive_close = !_linger_after_streams_finish;

    // prereq 1: inbound stream ended & fully assembled
    bool istream_done = _receiver.stream_out().input_ended() && _receiver.unassembled_bytes() == 0;

    // prereq 2 (and 3): outbound stream ended, fully sent (fully ack'd by remote peer)
    bool ostream_done = _sender.stream_in().eof() && _sender.next_seqno_absolute() == _sender.stream_in().bytes_written() + 2 &&
               _sender.bytes_in_flight() == 0;

    if ((active_close || passive_close) && istream_done && ostream_done)
        _active = false;
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) {
    if (!active())
        return;

    _sender.tick(ms_since_last_tick);
    _last_segm_recv_timer += ms_since_last_tick;

    // if there has been too many consec retx and we need to send a RESET to peer
    // but there are no segments ready to be sent out, make an empty segment to send out
    if (_sender.consecutive_retransmissions() > TCPConfig::MAX_RETX_ATTEMPTS && _sender.segments_out().empty()) 
        _sender.send_empty_segment();

    // resend a segment if _sender.tick timer expires or send a reset segment
    send_segments();

    if (_sender.consecutive_retransmissions() > TCPConfig::MAX_RETX_ATTEMPTS)
        reset();

    try_switching_close_mode();
    try_closing_connection();
}

void TCPConnection::end_input_stream() {
    _sender.stream_in().end_input();
    _sender.fill_window();
    try_closing_connection(); // send_segments() called in try_closing_connection()
}

void TCPConnection::connect() {
    _sender.fill_window();
    send_segments();
}

void TCPConnection::reset() {
    _sender.stream_in().set_error();
    _receiver.stream_out().set_error();
    _active = false;
}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";
            if (_sender.segments_out().empty())
                _sender.send_empty_segment();
            send_segments(true);
            reset();
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}
