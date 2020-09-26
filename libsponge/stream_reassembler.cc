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
    , first_unread(0), first_unasmb(0), n_unasmb_bytes(0), eof_set(false), last_byte(0) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    if (_output.input_ended()) {
        return;
    }

    size_t first_unacceptable = first_unasmb + _capacity - _output.buffer_size();
    if (eof) {
        eof_set = true;
        last_byte = index + data.size(); 
    }
    if (index + data.size() >= first_unasmb 
            && index < first_unacceptable) {
        // some part of new chunk of data is within the receiver advertised window

        Chunk new_chunk = { data, index }; // do all erasing first, then insert
        // if data too long to fit in window, chop off back bits
        if (new_chunk.index + new_chunk.data.size() > first_unacceptable) {
            new_chunk.data = new_chunk.data.substr(0, first_unacceptable - new_chunk.index);
        }
        // if data overlaps ByteStream, chop off front bits
        if (new_chunk.index < first_unasmb) {
            new_chunk.data = new_chunk.data.substr(first_unasmb - new_chunk.index);
            new_chunk.index = first_unasmb;
        }

        // [new0, new1) new chunk's byte number range
        // [cur0, cur1) cur chunk's byte number ramge
        size_t new0 = new_chunk.index; 
        size_t new1 = new_chunk.index + new_chunk.data.size();

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
            } else if (cur0 <= new0 && new0 <= cur1) {
                // new chunk overlaps end of current chunk, prepend cur chunk's data to new data
                new_chunk.data = it->data.substr(0, new0 - cur0) + new_chunk.data;
                new_chunk.index = cur0;
                n_unasmb_bytes -= it->data.size();
                it = unasmb.erase(it);
            } else if (new0 <= cur0 && cur0 <= new1) {
                // new chunk overlaps beginning of cur chunk, append
                new_chunk.data += it->data.substr(new1 - cur0);
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
    // add directly to ByteStream if data chunk starts at or before first_unasmb &
    auto next_chunk = unasmb.begin();
    if (!unasmb.empty() && next_chunk->index <= first_unasmb) {
        // can be assembled, add directly to ByteStream
        // substr to skip over any bytes already in the stream if not out of range
        _output.write(next_chunk->data.substr(first_unasmb - next_chunk->index));
        first_unasmb += next_chunk->data.size();
        n_unasmb_bytes -= next_chunk->data.size();
        unasmb.erase(unasmb.begin());
    }
    
    // after potentially pushing to ByteStream
    if (eof_set && last_byte <= first_unasmb) {
        _output.end_input();
    }
}

size_t StreamReassembler::unassembled_bytes() const { return n_unasmb_bytes; }

bool StreamReassembler::empty() const { return unasmb.empty(); }
