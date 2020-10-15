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
    , _curr_RTO(_initial_retransmission_timeout){}

//_bytes_flying is added up in fill_window()
uint64_t TCPSender::bytes_in_flight() const {
  return _bytes_flying;
}


void TCPSender::fill_window() {

  //Send SYN
  if (_next_seqno == 0){
    TCPSegment seg;
    TCPHeader header = seg.header();
    header.syn = true;
    header.seqno = _isn;
    _segments_out.push(seg);
    return;
  }
  
  if(_window_size == 0){
    return;
  }
  
  //Send FIN
  if(_stream.eof()){
    TCPSegment seg;
    TCPHeader header = seg.header();
    header.fin = true;
    header.seqno = wrap(_next_seqno, _isn);
    _segments_out.push(seg);
    return;
  }
  
  //Send Payload
  while(_window_size > 0){
     TCPSegment seg;
     TCPHeader header = seg.header();
     uint64_t size_seg = min(min(_stream.buffer_size(), _window_size), TCPConfig::MAX_PAYLOAD_SIZE); 
     _next_seqno += size_seg;
     header.seqno = wrap(_next_seqno, _isn);
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
  uint64_t tmp = _next_seqno;
  _next_seqno = unwrap(ackno, _isn, _next_seqno);
  _window_size = window_size;
  if (tmp == _next_seqno){
    //check timer; if timed out, retransmit
    _consec_retrans++;
    //
    return;
  }
  if (_next_seqno == 1){return;}
  
  
  //look thru outstanding acks, remove or resend
  

  //fill_window
    
  return;
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) { DUMMY_CODE(ms_since_last_tick); }


unsigned int TCPSender::consecutive_retransmissions() const {
  return _consec_retrans;
}

void TCPSender::send_empty_segment() {}
