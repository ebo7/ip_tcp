#include "tcp_connection.hh"

#include <iostream>
#include <limits>
#include <algorithm>
using namespace std;

size_t TCPConnection::remaining_outbound_capacity() const {
  return _remaining_outbound_capacity;
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
    _linger_after_streams_finish = false;
    _sender.stream_in().set_error();
    _receiver.stream_out().set_error();
  }
    
  //receive
  _receiver.segment_received(seg);
  
  //ack set, set ackno window size
  if (header.ack){
    WrappingInt32 ackno = header.ackno;
    uint16_t window_size = header.win;
    //change ack_abs
    _sender.ack_received(ackno, window_size);
  }

  //queue ack
  if (seg.length_in_sequence_space() > 0){
    _sender.send_empty_segment();
  }
  
  //send
  send_segments();
  
  //timestamp for inbound ends
  if (_receiver.eof() && !_inbound_ended){
    _time_inbound_ended = _time;
    _inbound_ended = true;
  }
  
  if (_inbound_ended && !_sender.fin_sent()){
    _linger_after_streams_finish = false;
  }
  if (_sender.fin_sent()){
    _outbound_ackd = _sender.ack_abs() == _sender.stream_in().bytes_read() + 2;
  }
  //  cout << "-------------------------------" << endl;
  //  cout << "ack: " << header.ack << endl;
  //cout << "ack_abs: " << _sender.ack_abs() << endl;
  cout << "inbound ended: " << _inbound_ended <<endl;
  cout << "outbound sent fin: " << _sender.fin_sent() << endl;
  cout << "outbound ackd: " << _outbound_ackd << endl;
  cout << "linger: " << _linger_after_streams_finish << endl;
  cout << "sender bytes sent?: " << _sender.stream_in().bytes_read() << endl;
  cout <<"ack_abs in connect: " << _sender.ack_abs() << endl;
  cout << "-------------------------------" << endl;

}

bool TCPConnection::active()const {
  //both streams ended
  if (_outbound_ackd && _inbound_ended){
    //not linger when passive close or bool_ackd is true
    if (!_linger_after_streams_finish){
      return false;
    }
    cout << time_since_last_segment_received() << endl;
    //linger for inactivity (option a)
    if(time_since_last_segment_received() >= 10 * _cfg.rt_timeout){
      return false;
    }
  }
  return true;
}

size_t TCPConnection::write(const string &data) {
  
  return _sender.stream_in().write(data);
  
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) {
  _time += ms_since_last_tick;
  //tell time to sender
  _sender.tick(ms_since_last_tick);
  send_segments();
  //abort
  if(_sender.consecutive_retransmissions() > TCPConfig::MAX_RETX_ATTEMPTS){
    handle_send_rst();
  }

  //end cleanly-look linger time?
}

void TCPConnection::end_input_stream() {
  _sender.stream_in().end_input();
  //send fin
  _sender.fill_window();
  send_segments();
}

void TCPConnection::connect() {
  //send syn
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
//handler for rst=true
void TCPConnection::handle_send_rst(){
  _rst = true;
  _sender.stream_in().set_error();
  _receiver.stream_out().set_error();
  _sender.send_empty_segment();
  send_segments();
  _linger_after_streams_finish = false;
}
//Destruct
TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";
	    handle_send_rst();
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}
