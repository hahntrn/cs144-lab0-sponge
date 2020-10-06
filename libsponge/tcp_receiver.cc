#include "tcp_receiver.hh"
#include <iostream>

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    cout << endl << "seg len: " << seg.length_in_sequence_space() << endl;
    cout << "header summary: " << endl << seg.header().summary() << endl;

    if (fin_received) return;
    const TCPHeader &header = seg.header();
    if (header.syn) {
        syn_received = true;
        isn = header.seqno + 1;
    }
    if (!syn_received) return;
    if (header.fin) {
        fin_received = true;
        //_reassembler.stream_out().input_ended();
    }
    const uint64_t abs_seqno = unwrap(header.seqno + 1, isn, checkpoint);
    const string data = seg.payload().copy();
    //if (abs_seqno < ackno().value().raw_value() || abs_seqno + data.size() >= window_size()) return;
    cout << "pushing " << data << " to index " << abs_seqno << " with offset " << isn << endl << endl;
    _reassembler.push_substring(data, abs_seqno, header.fin); // -1 for SYN
    checkpoint = abs_seqno;
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if (syn_received) {
        // if (fin_received) {
        //     return isn + _reassembler.stream_out().bytes_written() + 2;
        // }
        // return isn + _reassembler.stream_out().bytes_written() + 1;
        return isn + _reassembler.stream_out().bytes_written() + syn_received + fin_received;
    }        
    return {};
}

size_t TCPReceiver::window_size() const { return _reassembler.stream_out().remaining_capacity(); }
