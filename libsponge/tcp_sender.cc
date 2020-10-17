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
           cout<<"CONSTRUCT XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" <<endl;
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
  cout << "bytes flying: " << _bytes_flying << endl;
  cout << "eof: " << _stream.eof() << endl;
  //if (_bytes_flying==0 && _window_size==0 && _ack_abs!=0){
    //cout << "0--->1" << endl;
    //_window_size=1;
    //}else{
    //cout<< "DO NOTHING"<<endl;
    //return;
    // }
    //Send SYN
  if (_next_seqno == 0){
    cout << "SYN" << endl;
    TCPSegment seg;
    seg.header().syn = true;
    seg.header().seqno = _isn;
    //change state vars
    _segments_out.push(seg);
    _segments_outstanding.push_back(seg);
    _bytes_flying++;
    _next_seqno++;
    _seqnos_abs_outstanding.push_back(_next_seqno);
    //plaeholder for timestamp                                                                                      
    _timestamps_outstanding.push_back(_time);
    return;
  }
  //if(_window_size==0){
  //if(_bytes_flying==0 && _ack_abs!=0){
  //  _window_size = 1;
  //  cout << "0--->1" << endl;
  //}else{
      //      _window_size = 1;
  //  return;
  //}

  //}
  //  if(_window_size==0 && !_stream.eof() && _stream.buffer_size()==0){
  //  //send empty;
    //  TCPSender::send_empty_segment(); 
  //return;
  //}else if(_window_size==0){
  //  return;
  //}
  
  //Send FIN
  if( _stream.eof() && !_fin_sent  &&
  (_window_size>_bytes_flying || (_window_size==0 && _bytes_flying==0))){
    cout << "FIN" << endl;
    TCPSegment seg;
    seg.header().fin = true;
    seg.header().seqno = next_seqno();
    //change state vars
    _segments_out.push(seg);
    _segments_outstanding.push_back(seg);
    _bytes_flying++;
    _next_seqno++;
    if(_window_size != 0){
      //_window_size--;
    }
    _seqnos_abs_outstanding.push_back(_next_seqno);
    //plaeholder for timestamp
    _timestamps_outstanding.push_back(_time);
    _fin_sent = true;
    return;
  }

  //Send size 1 payload
  if (_stream.buffer_size() > 0 && _window_size==0 && _bytes_flying==0){
    TCPSegment seg;
    seg.header().seqno = next_seqno();
    seg.payload() = static_cast<Buffer>(_stream.read(1));
    _segments_out.push(seg);
    _segments_outstanding.push_back(seg);
    _bytes_flying++;
    _next_seqno++;
    _seqnos_abs_outstanding.push_back(_next_seqno);
     //plaeholder for timestamp                                                                                     
    _timestamps_outstanding.push_back(_time);
    return;
  }
  while(_stream.buffer_size() > 0 && _window_size > _bytes_flying){
    cout << "PAY" << endl;
    TCPSegment seg;
    uint64_t size_seg = min(min(_stream.buffer_size(), _window_size - _bytes_flying), TCPConfig::MAX_PAYLOAD_SIZE);
    seg.header().seqno = next_seqno();
    seg.payload() = static_cast<Buffer>(_stream.read(size_seg));
    //cout <<"pay eof: "<< _stream.eof() << endl;
    //piggyback fin
    cout << "pay size: "<< size_seg<<endl;
    //_window_size -= size_seg;
    _bytes_flying+=size_seg;
    if(_stream.eof() && !_fin_sent && _window_size>_bytes_flying){
      cout << "piggy" << endl;
      seg.header().fin = true;
      _fin_sent = true;
      //_window_size--;
      _bytes_flying++;
      _next_seqno++;
    }
    //Change state vars
     _segments_out.push(seg);
     _segments_outstanding.push_back(seg);
     _next_seqno += size_seg;
     _seqnos_abs_outstanding.push_back(_next_seqno);
     //plaeholder for timestamp                                                                                     
    _timestamps_outstanding.push_back(_time);

  }
  
  
}


//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size Thte remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
  cout << "-----------------------" << endl;
  cout << "ACK CALLED" <<endl;
  uint64_t ack_abs = unwrap(ackno, _isn, _next_seqno);
  cout << "ack_abs: "<<ack_abs << endl;
  cout << "window size: " << window_size << endl;
  cout << "bytes flying:" << _bytes_flying << endl;
  _window_size = window_size;
  if (ack_abs <= _ack_abs){
    cout << "USELESS ACK RECEIVED" << endl;
    return;
  }
  _curr_RTO = _initial_retransmission_timeout;
  uint64_t bytes_ack = ack_abs - _ack_abs;
 _bytes_flying -= bytes_ack;
 _ack_abs = ack_abs;
 _consec_retrans = 0;
  //look thru outstanding acks and remove acknowledged ones
  while(!_seqnos_abs_outstanding.empty()){
    if(_ack_abs >= _seqnos_abs_outstanding.front()){
      _seqnos_abs_outstanding.pop_front();
      _segments_outstanding.pop_front();
      _timestamps_outstanding.pop_front();
    }else{
      break;
    }
  }
  //reset the leftover timestamps
  for (size_t i=0; i<_timestamps_outstanding.size(); i++){
    _timestamps_outstanding[i] = _time;
  }  
  if (_window_size > _bytes_flying){
    cout << "=====FROM ACK RECEIVED=====" << endl;
    TCPSender::fill_window();
  }else{
    //send empty segment
    //TCPSender::send_empty_segment(); 
  }
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
  if (_timestamps_outstanding.empty()){
      return;
    }
  cout << "=========================================================" << endl;
  cout << "TICK CALLED" << endl;
  cout << "ms_tick: "<<ms_since_last_tick << endl;
  _time += ms_since_last_tick;
  cout << "curr_RTO: " << _curr_RTO  <<endl;
  cout << "time: " << _time << endl;
  cout << "timestamp: " << _timestamps_outstanding[0] << endl;
  cout<<"time diff: " << _time - _timestamps_outstanding[0] << endl;
  cout <<"bytes_flying: " << _bytes_flying << endl;
  cout << "_window size: "<<_window_size << endl;
  for (size_t i=0; i<_timestamps_outstanding.size(); i++){
    cout << _seqnos_abs_outstanding[i] << endl;
  }
  if (_time - _timestamps_outstanding[0] >= _curr_RTO){
    cout << "resending" << endl;
    cout << "last seqno is: " << _seqnos_abs_outstanding[0] << endl;
    _segments_out.push(_segments_outstanding[0]);
    _timestamps_outstanding[0] = _time;
    if (_window_size!=0){
      cout << "doubling" << endl;
      _consec_retrans++;
      _curr_RTO *= 2;
    }
  }
  cout << "=========================================================" << endl;
}


unsigned int TCPSender::consecutive_retransmissions() const {
  cout << "--------------------------"<<endl;
  cout << "RETRANS CALLED: "<< _consec_retrans << endl;
  return _consec_retrans;
}

void TCPSender::send_empty_segment() {
  cout << "SEND EMPTY CALLED" << endl;
  TCPSegment seg;
  seg.header().seqno = next_seqno();
  seg.payload() = static_cast<Buffer>("");
  _segments_out.push(seg);
}
