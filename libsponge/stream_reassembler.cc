#include "stream_reassembler.hh"
#include <iostream>
// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity):
       	_aux(capacity), _output(capacity), _capacity(capacity), _start_unread(0) {}

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

void StreamReassembler::write_to_aux(const string &data, const size_t index, const bool eof){
  cout<<data[0]<<endl;
  cout<<index<<endl;
  cout<<eof<<endl;
}

void StreamReassembler::read_from_aux(){
}

size_t StreamReassembler::unassembled_bytes() const { return {}; }

bool StreamReassembler::empty() const { 
	return unassembled_bytes() == 0;
}
