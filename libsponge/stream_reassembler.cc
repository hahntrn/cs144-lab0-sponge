#include "stream_reassembler.hh"
#include <iostream>

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) :
    _output(capacity), _capacity(capacity)
    , unasmb() //? why empty init???
    , first_unread(0), n_unasmb_bytes(0) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    // add directly to ByteStream if data chunk starts at or before first_unassembled &
    size_t first_unassembled = first_unread + _output.buffer_size();
    if (index <= first_unassembled && index + data.size() > first_unassembled) {
        // can be assembled, add directly to ByteStream
        // substr to skip over any bytes already in the stream if not out of range
        first_unread += _output.write(data.substr(first_unassembled - index));
    } else if (index > first_unassembled && index + data.size() < first_unread + _capacity) {
        // can't be assembled yet but
        // new chunk of data is within the receiver advertised window

        Chunk new_chunk = { data, index }; // do all erasing first, then insert
        // [new0, new1) new chunk's byte number range
        // [cur0, cur1) cur chunk's byte number ramge
        size_t new0 = index;              
        size_t new1 = index + data.size();

        // if new chunk of data overlaps with any existing unassembled chunk, merge them
        auto it = unasmb.begin(); 
        while (it != unasmb.end()) {
            size_t cur0 = it->index;
            size_t cur1 = it->index + it->data.size();
            // TODO: exit early if byte number past the end of new chunk since set is in order 
            if (cur0 <= new0 && new1 <= cur1) {
                // new data already contained in an existing chunk, exit early
                if (eof) _output.end_input();
                return; 
            } else if (new0 <= cur0 && cur1 <= new1) {
                // new chunk encapsulates an existing chunk, remove to replace w/ new
                n_unasmb_bytes -= it->data.size();
                it = unasmb.erase(it);
            } else if (cur0 <= new0 && cur1 <= new1) {
                // new chunk overlaps end of current chunk, prepend cur chunk's data to new data
                new_chunk.data = it->data.substr(0, new0 - cur0) + data;
                new_chunk.data = cur0;
                n_unasmb_bytes -= it->data.size();
                it = unasmb.erase(it);
            } else if (new0 <= cur0 && new1 <= cur1) {
                // new chunk overlaps beginning of cur chunk, append
                new_chunk.data = data + it->data.substr(new1 - cur0);
                n_unasmb_bytes -= it->data.size();
                it = unasmb.erase(it);
            } else {
                // no overlap
                it++;
            }
        }
        // done erasing, now insert
        n_unasmb_bytes += new_chunk.data.size();
        unasmb.insert(new_chunk);
    }
    if (eof) {
        _output.end_input();
    }
}

size_t StreamReassembler::unassembled_bytes() const { return n_unasmb_bytes; }

bool StreamReassembler::empty() const { return unasmb.empty(); }
