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
    , unasmb(capacity) 
    , first_unread(0), unasmb_vbegin(0) {
    
    for (Byte& b : unasmb) {
        b.filled = false;
    }    
}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    // TODO: if string is the next one we expect, directly add to ByteStream
    for (size_t i = 0; i < _capacity; i++) {
       // TODO: only assign if unfilled?
       unasmb[rbi(i)].data = data[i];
       unasmb[rbi(i)].filled = true;
}
   
   if (index <= first_unread + _output.buffer_size()) {
       // for (size_t i = 0; unasmb[rbi(i)].filled; i++) {
           
       first_unread += data.size();
    }
   std::cout << eof;
}

size_t StreamReassembler::unassembled_bytes() const { return 0; }

bool StreamReassembler::empty() const { return false; }
