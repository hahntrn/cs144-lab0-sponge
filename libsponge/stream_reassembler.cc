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
    , unasmb() 
    , first_unread(0) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    // add directly to ByteStream if data chunk starts at or before first_unassembled &
    size_t first_unassembled = first_unread + _output.buffer_size();
    if (index <= first_unassembled) {
        // if data contains bytes already in ByteStream, return
        if (index + data.size() <= first_unassembled) return;
       // if we can't fit this chunk of data in the ByteStream,
       // we can't fit it into the unassembled buffer either...
       //
       // substr to skip over any bytes already in the stream
       // already checked that it won't be out of range (would've returned)
       first_unread += _output.write(data.substr(first_unread + _output.buffer_size() - index));
    } else {
        // if new chunk of data overlaps with any existing unassembled chunk, merge them
        Chunk new_chunk = { data, index }; // do all erasing first, then insert
        auto it = unasmb.begin(); 
        // [new0, new1) new chunk's byte range
        // [cur0, cur1) cur chunk's byte ramge
        size_t new0 = index;              
        size_t new1 = index + data.size();
        while (it != unasmb.end()) {
            size_t cur0 = it->index;
            size_t cur1 = it->index + it->data.size();
            // TODO: exit early if byte number past the end of new chunk since set is in order 
            if (cur0 <= new0 && new1 <= cur1) {
                // new data already contained in an existing chunk, exit early
                return; 
            } else if (new0 <= cur0 && cur1 <= new1) {
                // new chunk encapsulates an existing chunk, remove to replace w/ new
                it = unasmb.erase(it);
            } else if (cur0 <= new0 && cur1 <= new1) {
                // new chunk overlaps end of current chunk, prepend cur chunk's data to new data
                new_chunk.data = it->data.substr(0, new0 - cur0) + data;
                new_chunk.data = cur0;
                it = unasmb.erase(it);
            } else if (new0 <= cur0 && new1 <= cur1) {
                // new chunk overlaps beginning of cur chunk, append
                new_chunk.data = data + it->data.substr(new1 - cur0);
                it = unasmb.erase(it);
            } else {
                // no overlap
                it++;
            }
        }
        // done erasing, now insert
        unasmb.insert(new_chunk);
    }
   std::cout << eof; //TODO
}

size_t StreamReassembler::unassembled_bytes() const { return 0; }

bool StreamReassembler::empty() const { return false; }
