#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity): 
    buffer(capacity), begin(buffer.begin()), curSize(0), maxCapacity(capacity) {}

size_t ByteStream::write(const string &data) {
    buffer[0] = data[0];
    return 1;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    // const std::vector<char>::iterator last_item = buffer.begin() 
    //     + ((const_cast<const std::vector<char>::iterator>(begin) - buffer.begin()) 
    //     + curSize) % buffer.size();
    // return std::string(last_item - len, last_item);
    return std::string(begin, begin + len);
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) { begin += len; }

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    DUMMY_CODE(len);
    return {};
}

void ByteStream::end_input() {}

bool ByteStream::input_ended() const { return {}; }

size_t ByteStream::buffer_size() const { return {}; }

bool ByteStream::buffer_empty() const { return {}; }

bool ByteStream::eof() const { return false; }

size_t ByteStream::bytes_written() const { return {}; }

size_t ByteStream::bytes_read() const { return {}; }

size_t ByteStream::remaining_capacity() const { return {}; }
