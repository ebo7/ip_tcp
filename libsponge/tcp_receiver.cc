#include "tcp_receiver.hh"
#include <string.h>
#include <iostream>
// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.


using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
  TCPHeader header = seg.header();
  
  if (!_syn && !seg.header().syn){
    return;
  }else if (seg.header().syn){
    _syn = true;
    _isn = header.seqno;
  }
  WrappingInt32 seqno = header.seqno;
  uint64_t idx_stream = unwrap(seqno, _isn, _ckpt) - 1;
  bool eof = false;
  if (seg.header().fin){
    eof = true;
  }
  string data = seg.payload().copy();
  _reassembler.push_substring(data, idx_stream, eof);
  _ckpt = _reassembler.stream_out().bytes_written();
  
}

optional<WrappingInt32> TCPReceiver::ackno() const {
  if (_syn){
    return wrap(_ckpt + 1, _isn);
  }return{};
}

size_t TCPReceiver::window_size() const {
  return _capacity - _reassembler.stream_out().buffer_size();
}
