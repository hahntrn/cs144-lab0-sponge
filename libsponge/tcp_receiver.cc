#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    if (seg.header().syn) {
        syn_received = true;
        isn = seg.header().seqno;
    }
    if (!syn_received) return;
    if (seg.header().fin) 
        fin_received = true;
    
    uint64_t abs_seqno = unwrap(seg.header().seqno, isn, _reassembler.stream_out().bytes_written() + 1);

    // subtract 1 from absolute sequence number if packet has SYN flag
    _reassembler.push_substring(seg.payload().copy(), abs_seqno - !seg.header().syn, seg.header().fin);
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if (syn_received) {
        // +1 for SYN flag, +1 for FIN flag if we've assembled it
        return wrap(_reassembler.stream_out().bytes_written() 
            + syn_received + _reassembler.stream_out().input_ended(), isn);
    }        
    return {};
}

size_t TCPReceiver::window_size() const { return _reassembler.stream_out().remaining_capacity(); }
