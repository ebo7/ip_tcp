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
    write_to_aux(data, index, eof);
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

    // Location inside data to start copying
    size_t loc_data = start_copy - start_data;

    // Location inside aux to start copying
    size_t loc_aux = (start_copy) % _capacity;

    // Check how many empty spaces to fill
    size_t count_empty = accumulate(_empty.begin() + loc_aux, _empty.begin() + loc_aux + size, 0);

    _aux.replace(loc_aux, size, data.substr(loc_data, size));
    _bytes_unass += count_empty;
    fill(_empty.begin() + loc_aux, _empty.begin() + loc_aux + size, false);
    write_to_bytestream();

    if (eof) {
        _end_stream = end_data;
    }
    // Last byte could have been stitched previously, end_input without eof if necessary
    if (_start_aux == _end_stream) {
        _output.end_input();
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

    // Implementation where it waits until all assemebled bytes are written to bytestream but was looping indefinitely..
    // while (true){
    // num_wrote += _output.write(_aux.substr(start + num_wrote, size - num_wrote));
    // if (num_wrote == size){
    // break;
    //}
    // if(cont==10){
    // throw;
    //}
    //}

    // Mark sent byte idxs as empty
    fill(it_start, it_start + num_wrote, true);
    
    _start_aux += num_wrote;
    _bytes_unass -= num_wrote;
}

// Return count of currently unassembled bytes
size_t StreamReassembler::unassembled_bytes() const { return _bytes_unass; }

// Return if there is no unassembled bytes
bool StreamReassembler::empty() const { return unassembled_bytes() == 0; }
