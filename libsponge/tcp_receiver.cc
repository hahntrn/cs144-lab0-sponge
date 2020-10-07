#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    const TCPHeader &header = seg.header();
    if (header.syn) {
        syn_received = true;
        isn = header.seqno;
    }
    if (!syn_received) return;
    if (header.fin) {
        fin_received = true;
    }
    uint64_t abs_seqno = unwrap(header.seqno, isn, checkpoint);
    //if (abs_seqno > 0) abs_seqno--;
    const string data = seg.payload().copy();
    //cout << endl;
    //cout << "data: " << data << " at " << abs_seqno - !header.syn << endl;
    //cout << "unasmb bytes " << _reassembler.unassembled_bytes() << endl;
    //cout << "rem cap      " << _reassembler.stream_out().remaining_capacity() << endl;
    //cout << "t bytes writ " << _reassembler.stream_out().bytes_written() << endl;
    _reassembler.push_substring(data, abs_seqno - !header.syn, header.fin); // -1 for SYN
    checkpoint = _reassembler.stream_out().bytes_written() + 1;
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if (syn_received) {
        return wrap(_reassembler.stream_out().bytes_written() 
            + syn_received + _reassembler.stream_out().input_ended(), isn);
    }        
    return {};
}

size_t TCPReceiver::window_size() const { return _reassembler.stream_out().remaining_capacity(); }
