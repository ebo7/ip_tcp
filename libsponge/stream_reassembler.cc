#include "stream_reassembler.hh"
#include <iostream>
#include <algorithm>
// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity):
  _aux(""), _output(capacity), _capacity(capacity), _start_aux(0), _empty(capacity, true) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    	cout<<"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
	cout << _aux.size() << endl;
	cout<<_capacity<<endl;
	cout<<_aux[0]<<endl;
	//If eof, signal input is ended
	if (eof){
	    _output.end_input();
    	}
	write_to_aux(data, index, eof);
	read_from_aux();
		
}
// Writes data into the auxiliary storage object
void StreamReassembler::write_to_aux(const string &data, const size_t start_data, const bool eof){
  //Index for the start of unacceptable bytes
  size_t end_aux = _start_aux + _capacity;
  size_t end_data = start_data + data.size()-1;
  cout<<start_data<<endl;
  cout<< end_aux<<endl;
  if (start_data > end_aux || end_data < _start_aux){
   return;
  }

  //Index to start copying data
  size_t start_copy = max(_start_aux, start_data);
  size_t end_copy = min(end_aux, end_data);
  //Size of data to copy
  size_t size = end_copy - start_copy;
  //Location inside data to start copying
  size_t loc_data = start_copy - start_data;
    //Location inside aux to start copying
    size_t loc_aux = (start_copy) % _capacity;

    _aux.substr(loc_aux, size) = data.substr(loc_data, size);
    for (size_t i=loc_aux; i<loc_aux + size; i++){
      _empty[i] = false;
    }
    cout<<eof<<endl;
}

// Reads from auxiiary storage in a suitable format and writes to bytestream
void StreamReassembler::read_from_aux(){

}

size_t StreamReassembler::unassembled_bytes() const { return {}; }

bool StreamReassembler::empty() const { 
	return unassembled_bytes() == 0;
}
