#ifndef SPONGE_LIBSPONGE_STREAM_REASSEMBLER_HH
#define SPONGE_LIBSPONGE_STREAM_REASSEMBLER_HH

#include "byte_stream.hh"

#include <cstdint>
#include <string>
#include <set>

// References: Nick Hirning (nhirning) for idea of storing aggregated bytes as
// one item in data structure and using a struct to store metadata

//! \brief A class that assembles a series of excerpts from a byte stream (possibly out of order,
//! possibly overlapping) into an in-order byte stream.
class StreamReassembler {
  private:
    // Your code here -- add private members as necessary.
      
    ByteStream _output;  //!< The reassembled in-order byte stream
    size_t _capacity;    //!< The maximum number of bytes
    struct Chunk {
        inline bool operator<(const Chunk &b) const { return index < b.index; };
        std::string data = "";
        size_t index = 0;
    }; // why no alias here?
    //struct cmp { bool operator() (struct Chunk a, struct Chunk b) const { return a.index < b.index; } };
    //bool cmp = [](struct Chunk &a, struct Chunk &b) { return a.index < b.index; };
    std::set<Chunk> unasmb; 
    size_t first_unread; // index of first byte in _output ByteStream wrt whole stream
    size_t n_unasmb_bytes;

    //! \brief Returns the offset index in unassembled ring buffer
    // todo: make const
    // const size_t rbi(const size_t i) { return (i + const_cast<const size_t>(unasmb_vbegin)) % _capacity; }
    // size_t rbi(size_t i) { return (i + unasmb_vbegin) % _capacity; }
  public:
    //! \brief Construct a `StreamReassembler` that will store up to `capacity` bytes.
    //! \note This capacity limits both the bytes that have been reassembled,
    //! and those that have not yet been reassembled.
    StreamReassembler(const size_t capacity);

    //! \brief Receive a substring and write any newly contiguous bytes into the stream.
    //!
    //! The StreamReassembler will stay within the memory limits of the `capacity`.
    //! Bytes that would exceed the capacity are silently discarded.
    //!
    //! \param data the substring
    //! \param index indicates the index (place in sequence) of the first byte in `data`
    //! \param eof the last byte of `data` will be the last byte in the entire stream
    void push_substring(const std::string &data, const uint64_t index, const bool eof);

    //! \name Access the reassembled byte stream
    //!@{
    const ByteStream &stream_out() const { return _output; }
    ByteStream &stream_out() { return _output; }
    //!@}

    //! The number of bytes in the substrings stored but not yet reassembled
    //!
    //! \note If the byte at a particular index has been pushed more than once, it
    //! should only be counted once for the purpose of this function.
    size_t unassembled_bytes() const;

    //! \brief Is the internal state empty (other than the output stream)?
    //! \returns `true` if no substrings are waiting to be assembled
    bool empty() const;
};

#endif  // SPONGE_LIBSPONGE_STREAM_REASSEMBLER_HH
