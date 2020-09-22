#include "libsponge/byte_stream.hh"
#include "libsponge/byte_stream.cc"
#include <iostream>

int main(int argc, char *argv[]) {
    ByteStream bs(5);
    // bool r = argv[1] == "-r";
    // if (r) {
    //     std::cout << bs.read(size_t(argv[2])) << std::endl;
    //     bs.end_input();
    // } else {
    //     std::cout << bs.write(argv[2]) << std::endl;
    // }

    std::cout << bs.write("black cat") << std::endl;
    bs.end_input();
    std::cout << bs.read(3) << std::endl;
    std::cout << bs.write("asdf") << std::endl;
    std::cout << bs.read(7) << std::endl;
    std::cout << "err? " << bs.read(1) << std::endl;
    std::cout 
        << "Input ended:   " << bs.input_ended() << std::endl
        << "Buffer empty:  " << bs.buffer_empty() << std::endl
        << "EOF:           " << bs.eof() << std::endl
        << "Bytes read:    " << bs.bytes_read() << std::endl
        << "Bytes written: " << bs.bytes_written() << std::endl
        << "Remaining cap: " << bs.remaining_capacity() << std::endl
        << "Buffer size:   " << bs.buffer_size() << std::endl;
    return 0;
}
