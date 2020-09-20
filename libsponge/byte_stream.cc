#include "byte_stream.hh"
#include <iostream>

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity): 
    buffer(capacity), data_begin(0), 
    n_bytes_read(0), cur_size(0), max_capacity(capacity) {}

size_t ByteStream::write(const string &data) {
    size_t n_bytes_to_write = std::min(data.length(), max_capacity - cur_size);
    if (n_bytes_to_write > max_capacity - cur_size) {
        n_bytes_to_write = max_capacity - cur_size;
        // std::cerr << "Not enough space left in buffer for data" << std::endl;
    }

    for (size_t i = 0; i < n_bytes_to_write; i++) {
        buffer[(data_begin + i) % max_capacity] = data[i];
    }
    n_bytes_read += n_bytes_to_write;
    cur_size += n_bytes_to_write;

    return 1;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    // if requesting more bytes than what's in the buffer,
    // return as many as possible (i.e. all bytes written in buffer)
    
    // num bytes to return, max is num bytes in buffer
    size_t len_ret = std::min(len, cur_size); 

    if (data_begin + len_ret < cur_size) {
        std::string s(buffer.begin() + data_begin, 
                      buffer.begin() + data_begin + len_ret);
        return s;
    }
    std::string s1(buffer.begin() + data_begin, buffer.end());
    std::string s2(buffer.begin(), 
                   buffer.begin() + (data_begin + len_ret) % cur_size);
    s1.append(s2);
    return s1;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) { 
    std::cout << len;
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    DUMMY_CODE(len);
    return {};
}

void ByteStream::end_input() {}

bool ByteStream::input_ended() const { return {}; }

size_t ByteStream::buffer_size() const { return cur_size; }

bool ByteStream::buffer_empty() const { return cur_size == 0; }

bool ByteStream::eof() const { return false; }

size_t ByteStream::bytes_written() const { return {}; }

size_t ByteStream::bytes_read() const { return {}; }

size_t ByteStream::remaining_capacity() const { return {}; }
