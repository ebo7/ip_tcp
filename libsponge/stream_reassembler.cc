#include "stream_reassembler.hh"

#include <algorithm>
#include <iostream>
#include <numeric>

using namespace std;

// Initialization of private members
StreamReassembler::StreamReassembler(const size_t capacity)
    : _aux("")
    , _output(capacity)
    , _capacity(capacity)
    , _start_aux(0)
    , _empty(capacity, true)
    , _bytes_unass(0)
    , _end_stream(-1) {
    for (size_t i = 0; i < capacity; i++) {
        _aux.append(" ");
    }
}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    // if(!_eof_seen){
    //_eof_seen = eof;
    //}
    write_to_aux(data, index, eof);
    write_to_bytestream();

    //        Last byte could have been stitched previously, end_input without eof if necessary
    if (!_eof_seen) {
        _eof_seen = eof;
    }
    if (_start_aux == _end_stream) {  // && unassembled_bytes()==0) {
        _output.end_input();
    }
}
// Write data into the auxiliary storage object
void StreamReassembler::write_to_aux(const string &data, const size_t start_data, const bool eof) {
    if (data.size() == 0) {
        if (eof) {
            // Bytestream finished already
            _output.end_input();
        }
        return;
    }

    // Index for the start of unacceptable bytes
    size_t end_aux = _start_aux + _capacity;

    // Last index of data + 1
    size_t end_data = start_data + data.size();

    if (start_data > end_aux || end_data < _start_aux) {
        return;
    }

    // Index to start and end copying data
    size_t start_copy = max(_start_aux, start_data);
    size_t end_copy = min(end_aux, end_data);

    // Size of data to copy
    size_t size = end_copy - start_copy;
    // cout << "SIZE TO COPY: "<< size<<endl;
    // Location inside data to start copying
    size_t loc_data = start_copy - start_data;

    // Location inside aux to start copying
    size_t loc_aux = (start_copy) % _capacity;

    // Depending on size of data to capy, we might wrap around _aux
    if (loc_aux + size <= _capacity) {
        // Check how many empty spaces to fill
        size_t count_empty = accumulate(_empty.begin() + loc_aux, _empty.begin() + loc_aux + size, 0);

        // cout<< (_empty.begin() + _capacity + 10 > _empty.end()) << endl;

        _aux.replace(loc_aux, size, data.substr(loc_data, size));
        _bytes_unass += count_empty;
        fill(_empty.begin() + loc_aux, _empty.begin() + loc_aux + size, false);
    }  // Wrapping around buffer
    else {
        // lefthand part size
        size_t size_left = loc_aux + size - _capacity;
        size_t count_empty = accumulate(_empty.begin() + loc_aux, _empty.end(), 0) +
                             accumulate(_empty.begin(), _empty.begin() + size_left, 0);
        // righthand part size
        size_t size_right = _capacity - loc_aux;
        _aux.replace(loc_aux, size_right, data.substr(loc_data, size_right));
        _aux.replace(0, size_left, data.substr(size_right, size));
        _bytes_unass += count_empty;
        fill(_empty.begin() + loc_aux, _empty.end(), false);
        fill(_empty.begin(), _empty.begin() + size_left, false);
    }

    if (eof) {
        _end_stream = end_data;
    }
}

// Read from auxiiary storage using _empty and write to bytestream
void StreamReassembler::write_to_bytestream() {
    // Index to start writing to bytestream
    size_t start = _start_aux % _capacity;

    // Iterators for _empty
    auto it_start = _empty.begin() + (_start_aux % _capacity);
    auto it_end = find(it_start, _empty.end(), true);
    size_t size = it_end - it_start;
    size_t num_wrote = _output.write(_aux.substr(start, size));

    // Mark sent byte idxs as empty
    fill(it_start, it_start + num_wrote, true);

    _start_aux += num_wrote;
    _bytes_unass -= num_wrote;
    // Wrapping around
    if (it_end == _empty.end()) {
        it_end = find(_empty.begin(), it_start, true);
        size = it_end - _empty.begin();
        num_wrote = _output.write(_aux.substr(0, size));
        fill(_empty.begin(), it_start, true);

        _start_aux += num_wrote;
        _bytes_unass -= num_wrote;
    }
}

// Return count of currently unassembled bytes
size_t StreamReassembler::unassembled_bytes() const { return _bytes_unass; }

// Return if there is no unassembled bytes
bool StreamReassembler::empty() const { return unassembled_bytes() == 0; }
