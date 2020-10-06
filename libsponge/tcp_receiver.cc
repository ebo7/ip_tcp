#include "tcp_receiver.hh"
#include <string.h>
#include <iostream>
// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.


using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
  TCPHeader header = seg.header();
  
  if (!_syn && !seg.header.syn){
    return;
  }
  else if (seg.header.syn){
    _syn = true;
    _isn = header.seqno;
  }
  
  string buffer = seg.payload().copy();
  cout << buffer << endl;
}

optional<WrappingInt32> TCPReceiver::ackno() const { return {}; }

size_t TCPReceiver::window_size() const { return {}; }
