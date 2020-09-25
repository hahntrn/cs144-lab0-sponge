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
        // if new chunk of data overlaps with any existing unassembled chunk,
        // merge them
        for (struct Chunk chunk : unasmb) {
            // (b1, e1) current chunk's byte range
            // (b2, e2) new chunk's byte ramge
            size_t b1 = chunk.index;
            size_t e1 = chunk.index + chunk.data.size();
            size_t b2 = index;              
            size_t e2 = index + data.size();

            // new data already contained in an existing chunk
            if (b2 >= b1 && e2 <= e1) return; 
            // new chunk encapsulates an existing chunk
            if (b1 >= b2 && e1 <= e2) {
                // delete existing chunk, insert new one
                // so new chunk can replace multiple existing ones
                //struct Chunk c = { data, index };
                unasmb.insert(Chunk {data, index});
            }
        }
    }
   std::cout << eof;
}

size_t StreamReassembler::unassembled_bytes() const { return 0; }

bool StreamReassembler::empty() const { return false; }
