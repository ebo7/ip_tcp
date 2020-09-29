#include "stream_reassembler.hh"

#include <algorithm>
#include <iostream>
// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity)
    : _aux(""), _output(capacity), _capacity(capacity), _start_aux(0), _empty(capacity, true), _bytes_unass(0) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    cout << "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
    cout << _aux.size() << endl;
    cout << _capacity << endl;
    cout << _aux[0] << endl;
    // If eof, signal input is ended
    if (eof) {
        _output.end_input();
    }
    write_to_aux(data, index, eof);
    write_to_bytestream();
}
// Writes data into the auxiliary storage object
void StreamReassembler::write_to_aux(const string &data, const size_t start_data, const bool eof) {
    // Index for the start of unacceptable bytes
    size_t end_aux = _start_aux + _capacity;
    size_t end_data = start_data + data.size();
    cout << start_data << endl;
    cout << end_aux << endl;
    if (start_data > end_aux || end_data < _start_aux) {
        return;
    }

    // Index to start copying data
    size_t start_copy = max(_start_aux, start_data);
    size_t end_copy = min(end_aux, end_data);
    // Size of data to copy
    size_t size = end_copy - start_copy;
    // Location inside data to start copying
    size_t loc_data = start_copy - start_data;
    // Location inside aux to start copying
    size_t loc_aux = (start_copy) % _capacity;

    _aux.replace(loc_aux, size, data.substr(loc_data, size));
    _bytes_unass += size;
    fill(_empty.begin() + loc_aux, _empty.begin() + loc_aux + size, false);
    cout << "empty0" << endl;
    cout << _empty[0] << endl;
    cout << _empty[1] << endl;
    cout << eof << endl;
    cout << "AUX: " << _aux << endl;
    cout << "data: " << data.substr(loc_data, size) << endl;
}

// Reads from auxiiary storage in a suitable format and writes to bytestream
void StreamReassembler::write_to_bytestream() {
    // Index to start writing to bytestream
    size_t start = _start_aux % _capacity;
    // Iterators for _empty
    auto it_start = _empty.begin() + (_start_aux % _capacity);
    auto it_end = find(it_start, _empty.end(), true);
    size_t size = it_end - it_start;
    cout << "MMMMMMMMMMMMMMMMMMMMMM" << endl;
    cout << size << endl;
    size_t num_wrote = _output.write(_aux.substr(start, size));
    // Mark sent bytes as empty
    fill(it_start, it_start + num_wrote, true);
    _start_aux += num_wrote;
    _bytes_unass -= num_wrote;
}

size_t StreamReassembler::unassembled_bytes() const { return _bytes_unass; }

bool StreamReassembler::empty() const { return unassembled_bytes() == 0; }
