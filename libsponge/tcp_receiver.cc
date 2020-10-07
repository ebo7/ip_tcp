#include "tcp_receiver.hh"
#include <string.h>
#include <iostream>
// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.


using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
  TCPHeader header = seg.header();

  //need handshake
  if (!_syn && !header.syn){
    return;
  }else if (header.syn){
    _syn = true;
    _isn = header.seqno;
  }

  uint64_t len_in_seq = seg.length_in_sequence_space();
  uint64_t len_payload;

  //determine len_payload
  if(header.syn && header.fin){
    len_payload = len_in_seq - 2;
  }else if(header.syn || header.fin){
    len_payload = len_in_seq - 1;
  }else{
    len_payload = len_in_seq;
  }
  
  
  WrappingInt32 seqno = header.seqno;
  uint64_t start_payload = unwrap(seqno, _isn, _ckpt) - 1;

  //determine complete length of stream
  bool eof = false;
  if (header.fin){
    _len_stream = start_payload + len_payload;
    eof = true;
  }
  
  string data = seg.payload().copy();
  _reassembler.push_substring(data, start_payload, eof);
  _ckpt = _reassembler.stream_out().bytes_written();
  if (_ckpt == _len_stream){
    _ack = wrap(_ckpt + 2, _isn);
    _reassembler.stream_out().end_input();
  }else{
    _ack = wrap(_ckpt + 1, _isn);
  }
  
}

optional<WrappingInt32> TCPReceiver::ackno() const {
  if (_syn){
    return _ack;
  }return{};
}

size_t TCPReceiver::window_size() const {
  return _capacity - _reassembler.stream_out().buffer_size();
}
