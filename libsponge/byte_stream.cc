#include "byte_stream.hh"
#include <iostream>

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`
using namespace std;

//Init private members
ByteStream::ByteStream(const size_t capacity): cap(capacity), deq() {
}

size_t ByteStream::write(const string &data) {
	//size_t size_left = capacity - deq.size();
	//If data is empty, signal input ended
	if (data.size() == 0){
		_end = true;
	}
	//Data can be bigger than size_left
	size_t size = min(data.size(), remaining_capacity());
	bytes_wrote += size;
	for (size_t i=0; i<size; i++){
		deq.push_back(data[i]);
	}
	return size;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    string str_peeked;
    size_t size = min(len, deq.size());
    for (size_t i=0; i<size; i++){
    	str_peeked += deq[i];
    }
    //bytes_readed += size;
    return str_peeked;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
	size_t size = min(len, deq.size());
       for(size_t i=0; i<size; i++){
       //Bytes sent first are at the front
       deq.pop_front();
       }
	bytes_readed += size;       
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
	//Read
    	string str_readed = peek_output(len);
	//Pop
	pop_output(len);
    	return str_readed;
}

void ByteStream::end_input() {_end = true; }

bool ByteStream::input_ended() const { return _end; }

size_t ByteStream::buffer_size() const{ return deq.size(); }

bool ByteStream::buffer_empty() const { return deq.size() == 0; }

bool ByteStream::eof() const { return buffer_empty() && input_ended(); }

size_t ByteStream::bytes_written() const { return bytes_wrote; }

size_t ByteStream::bytes_read() const { return bytes_readed; }

size_t ByteStream::remaining_capacity() const { return cap - deq.size(); }
