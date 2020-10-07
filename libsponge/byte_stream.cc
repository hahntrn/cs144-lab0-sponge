#include "byte_stream.hh"

#include <iostream>

/** Ha Tran
 * 2020-09-21
 * Ring buffer queue implementation from CS106B Section 5
 *
 */
template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity)
    : done_writing(false)
    , buffer(capacity)
    , data_begin(0)
    , n_bytes_read(0)
    , n_bytes_written(0)
    , cur_size(0)
    , max_capacity(capacity)
    , _error(false) {}

size_t ByteStream::write(const string &data) {
    //if (eof()) {
    //    set_error();
    //}
    const size_t n_bytes_to_write = std::min(data.length(), remaining_capacity());
    //cout << "BS: writing " << n_bytes_to_write << endl;
    //cout << "data begin: " << data_begin << endl;
    //cout << "cur size    " << cur_size << endl;
    //cout << "before: "; for(size_t i=0; i<buffer.size();i++) cout << buffer[i] << ", "; cout << endl;
    for (size_t i = 0; i < n_bytes_to_write; i++) {
        buffer[(data_begin + cur_size + i) % max_capacity] = data[i];
    }
    n_bytes_written += n_bytes_to_write;
    cur_size += n_bytes_to_write;
    //cout << "Done writing n bytes to BS: " << peek_output(n_bytes_to_write) << endl;
    //cout << "after: "; for(size_t i=0; i<buffer.size();i++) cout << buffer[i] << ", "; cout << endl;
    //cout << "data begin: " << data_begin << endl;
    //cout << "cur size    " << cur_size << endl;
    return n_bytes_to_write;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    // if requesting more bytes than what's in the buffer,
    // return as many as possible (i.e. all bytes written in buffer)

    // num bytes to return, max is num bytes in buffer
    size_t len_ret = std::min(len, cur_size);

    // data is in one contiguous block in buffer
    if (data_begin + len_ret <= max_capacity) {
        std::string s(buffer.begin() + data_begin, buffer.begin() + data_begin + len_ret);
        return s;
    }

    // data is wrapped around the buffer, broken up in the middle
    std::string s1(buffer.begin() + data_begin, buffer.end());
    std::string s2(buffer.begin(), buffer.begin() + ((data_begin + len_ret) 
            % max_capacity));
    //cout << "end of s2 " << (data_begin + len_ret) % cur_size << endl;
    //cout << "s1: " << s1 << endl;
    //cout << "s2: " << s2 << endl;
    s1.append(s2);
    //cout << "returning: " << len_ret << " bytes: " << s1 << endl;
    return s1;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    data_begin = (data_begin + len) % max_capacity;
    n_bytes_read += std::min(len, cur_size);
    cur_size = std::max(size_t(0), cur_size - len);
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    //if (eof()) {
    //    set_error();
    //}
    std::string ret = peek_output(len);
    pop_output(ret.length());
    return ret;
}

void ByteStream::end_input() { done_writing = true; }

bool ByteStream::input_ended() const { return done_writing; }

size_t ByteStream::buffer_size() const { return cur_size; }

bool ByteStream::buffer_empty() const { return cur_size == 0; }

bool ByteStream::eof() const { return input_ended() && buffer_empty(); }

size_t ByteStream::bytes_written() const { return n_bytes_written; }

size_t ByteStream::bytes_read() const { return n_bytes_read; }

size_t ByteStream::remaining_capacity() const { return max_capacity - cur_size; }
