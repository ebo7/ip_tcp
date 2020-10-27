#include "tcp_connection.hh"

#include <iostream>
#include <limits>
#include <algorithm>
using namespace std;

size_t TCPConnection::remaining_outbound_capacity() const {
  return _sender.stream_in().remaining_capacity();
}

size_t TCPConnection::bytes_in_flight() const {
  return _sender.bytes_in_flight();
}

size_t TCPConnection::unassembled_bytes() const {
  return _receiver.unassembled_bytes();
}

size_t TCPConnection::time_since_last_segment_received() const {
  return _time - _time_last_segment_received;
}

void TCPConnection::segment_received(const TCPSegment &seg) {
    
  _time_last_segment_received = _time;
  TCPHeader header = seg.header();

  //rst set, end
  if (header.rst){
    _rst = true;
    _sender.stream_in().set_error();
    _receiver.stream_out().set_error();
  }
  if (header.fin){


  }
  //receive
  _receiver.segment_received(seg);
  
  //listen
   if (header.syn && !header.ack && !_connect_called){
     connect();
     _syn_is_seen = true;
       return;
  }
  //ack set, set ackno window size
  if (header.ack){
    WrappingInt32 ackno = header.ackno;
    uint16_t window_size = header.win;
    _sender.ack_received(ackno, window_size);
  }

  //send empty
  if (_receiver.ackno().has_value() and (seg.length_in_sequence_space() == 0) and
      seg.header().seqno == _receiver.ackno().value() - 1) {
    _sender.send_empty_segment();
  }
      else if (seg.length_in_sequence_space() > 0){
  _sender.send_empty_segment();
  }
  
    
  //inbound ends
  if (_receiver.stream_out().input_ended() && !_inbound_ended && _connect_called){
    _inbound_ended = true;
  }
  if (header.fin && !_sender.stream_in().eof() && _connect_called){
    _linger_after_streams_finish = false;
  }

  if (_sender.fin_sent()){
    _outbound_ackd = _sender.ack_abs() == _sender.stream_in().bytes_read() + 2;
  }

  send_segments();
  
}

bool TCPConnection::active()const {
  //unclean shutdown
  if (_rst){
    return false;
  }
  //clean shutdown
  //  if(_sender.sent_fin inbound ended byte fly=0)
  if (_outbound_ackd && _inbound_ended){// && unassembled_bytes()==0){
    //not linger when passive close or bool_ackd is true
    if (!_linger_after_streams_finish){
      return false;
    }

    //linger for inactivity (option a)
    if(time_since_last_segment_received() >= 10 * _cfg.rt_timeout){
      return false;
    }
  }
  return true;
}

size_t TCPConnection::write(const string &data) {
  if (data.size()==0){
    return 0;
  }  
  size_t ans = _sender.stream_in().write(data);
    _sender.fill_window();
  send_segments();
    return ans;
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) {
  _time += ms_since_last_tick;
  //tell time to sender
  _sender.tick(ms_since_last_tick);
  if(_sender.consecutive_retransmissions() > TCPConfig::MAX_RETX_ATTEMPTS){
      _rst = true;
    _sender.stream_in().set_error();
    _receiver.stream_out().set_error();
  }
  send_segments();
}

void TCPConnection::end_input_stream() {
  _sender.stream_in().end_input();
  //send fin
  _sender.fill_window();
  send_segments();
}

void TCPConnection::connect() {
  //send syn
  _connect_called = true;
  _sender.fill_window();
  send_segments();
}

//transfer segments from sender q to connection q
void TCPConnection::send_segments(){
  while(!_sender.segments_out().empty() ){
    TCPSegment seg = _sender.segments_out().front();
    if (_receiver.ackno().has_value()){
      seg.header().ack = true;
      seg.header().ackno = _receiver.ackno().value();
      seg.header().win = min(_receiver.window_size(), static_cast<size_t>(numeric_limits<uint16_t>::max()));
    }
    if(_rst){
      seg.header().rst = true;
    }
    _segments_out.push(seg);
    _sender.segments_out().pop();
  }
}

TCPConnection::~TCPConnection() {
  try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";
	    _rst = true;
	    _sender.stream_in().set_error();
	    _receiver.stream_out().set_error();
	    _sender.send_empty_segment();
	    send_segments();
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}
