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
  cout << "Index: " << index << " data size: " << data.size() << " data: " <<data <<" EOF: " << eof << endl;
  write_to_aux(data, index, eof);
    //write_to_bytestream();
}
// Writes data into the auxiliary storage object
void StreamReassembler::write_to_aux(const string &data, const size_t start_data, const bool eof) {
   if (data.size() == 0){
      //Bytestream ended
      if (eof){
	cout<<"----------1st EOF"<<endl;
	_output.end_input();
      }
      cout<<"---------------------------------------------------------"<<endl;
      return;
      
    }
  // Index for the start of unacceptable bytes
    size_t end_aux = _start_aux + _capacity;
    //Index data ends+1
    size_t end_data = start_data + data.size();
    //cout << "Index: " << start_data << " data: " << data << " EOF: " << eof << endl;
    cout<<"1st START_AUX: " <<_start_aux<<endl;
    cout <<"end data:" <<end_data<<endl;
    if (start_data > end_aux || end_data < _start_aux) {
      cout<<"INVAL"<<endl;
      cout<<"----------------------------------------------------------"<<endl;
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
    cout<< "DATA: "<< data.substr(loc_data, size) <<endl;
    cout << _aux.size() << endl;
    _aux.replace(loc_aux, size, data.substr(loc_data, size));
    _bytes_unass += size;
    fill(_empty.begin() + loc_aux, _empty.begin() + loc_aux + size, false);
    cout << "AUX: " << _aux << endl;
    write_to_bytestream();
    //Bytestream ended
    if (eof && (_start_aux  == end_data) ) {
      cout<<"----------2nd EOF" <<endl;
      _output.end_input();
    }
    cout<<"----------------------------------------------------------"<<endl;
}

// Reads from auxiiary storage in a suitable format and writes to bytestream
void StreamReassembler::write_to_bytestream() {
    // Index to start writing to bytestream
    size_t start = _start_aux % _capacity;
    // Iterators for _empty
    auto it_start = _empty.begin() + (_start_aux % _capacity);
    auto it_end = find(it_start, _empty.end(), true);
    size_t size = it_end - it_start;
    size_t num_wrote = _output.write(_aux.substr(start, size));
    cout<<"NUM BYTES WRITTEN: "<<num_wrote<<endl;
    // Mark sent bytes as empty
    fill(it_start, it_start + num_wrote, true);
    _start_aux += num_wrote;
    _bytes_unass -= num_wrote;
    cout<<"2nd START_AUX: " << _start_aux<<endl;

}

size_t StreamReassembler::unassembled_bytes() const { return _bytes_unass; }

bool StreamReassembler::empty() const { return unassembled_bytes() == 0; }
