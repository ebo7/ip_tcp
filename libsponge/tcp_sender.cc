#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>
#include <iostream>
#include <algorithm>
// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity)
    , _curr_RTO(_initial_retransmission_timeout){
      cout<<"CONSTRUCT XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" <<endl;
}

//_bytes_flying is added up in fill_window()
uint64_t TCPSender::bytes_in_flight() const {
  cout << "-----------------------" << endl;
  cout << "BYTES FLYING:"<<_bytes_flying <<endl;
  cout << "seqno: " << next_seqno_absolute() << endl; 
  return _bytes_flying;
}


void TCPSender::fill_window() {
  cout << "-----------------------" << endl;
  cout<<"FILL CALLED"<<endl;
  cout << "buf: " << _stream.buffer_size() << endl;
  cout << "ack: "<<_ack_abs << endl;
  cout << "window:" << _window_size << endl;
  
  if(_window_size == 0 || _ack_abs < next_seqno_absolute()){
    cout<<"EMPTY WINDOW OR INVAL ACK" << endl;
    return;
  }
  //Send SYN
  if (_ack_abs == 0){

    cout << "1st" << endl;

    TCPSegment seg;
    seg.header().syn = true;
    seg.header().seqno = _isn;
    cout << seg.header().syn << endl;
    _segments_out.push(seg);
    _bytes_flying = 1;
    _next_seqno = 1;
    return;
  }
  
  if(_window_size == 0){
    cout<<"EMPTY WINDOW" << endl;
    return;
  }
  
  //Send FIN
  if(_stream.eof()){
    cout << "2nd" << endl;
    TCPSegment seg;
    seg.header().fin = true;
    seg.header().seqno = wrap(_next_seqno, _isn);
    _segments_out.push(seg);
    _next_seqno = _stream.bytes_read() + 2;
    return;
  }
  
  //Send Payload
  while(_stream.buffer_size() > 0){
    cout << "3rd" << endl;
     TCPSegment seg;
     cout << _stream.buffer_size() << endl;
     cout << _window_size << endl;
     uint64_t size_seg = min(min(_stream.buffer_size(), _window_size), TCPConfig::MAX_PAYLOAD_SIZE);
     cout << size_seg << endl;
     seg.header().seqno = wrap(_next_seqno, _isn);
     _next_seqno += size_seg;;
     seg.payload() = static_cast<Buffer>(_stream.read(size_seg));
     //Change state vars
     _segments_out.push(seg);
     _segments_outstanding.push(seg);
     _seqnos_abs_outstanding.push(_next_seqno);
     _window_size -= size_seg;
     _bytes_flying += size_seg;
  }
}


//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size Thte remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
cout << "-----------------------" << endl;
  cout << "ACK CALLED" <<endl;
  uint64_t ack_abs = unwrap(ackno, _isn, _next_seqno);
  cout << "ack_abs: "<<ack_abs << endl;
  cout << "window: " << window_size << endl;
  if (ack_abs == 0){
    return;
  }
  _window_size = window_size;
  
  //if (_ack_abs_prev == ack_abs){
    //check timer; if timed out, retransmit
    //_consec_retrans++;
    //return;
  //}
  
  
  uint64_t bytes_ack = ack_abs - _ack_abs;
  _bytes_flying -= bytes_ack;
  cout << "CHANGING PREV" << endl;
  _ack_abs = ack_abs;
  
  
  //look thru outstanding acks, remove or resend
  //while(true){
  //cout << _seqnos_abs_outstanding.front() << endl;
  //return;
  //}
if (_window_size > 1){
    TCPSender::fill_window();
  }
  //fill_window
  //_ack_abs_prev = 1;
  //return;
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
  DUMMY_CODE(ms_since_last_tick);
  cout << "TICK CALLED" << endl;
}


unsigned int TCPSender::consecutive_retransmissions() const {
  cout << "RETRANS CALLED"<< endl;
  return _consec_retrans;
}

void TCPSender::send_empty_segment() {
  cout << "SEND EMPTY CALLED" << endl;
}
