#include "tcp_connection.hh"

#include <iostream>
#include <limits>
#include <algorithm>
// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

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
  TCPHeader header = seg.header();
  //rst set, end
  if (header.rst){
    _rst = true;
    //send rst- not needed as it receieved, the other is already terminated
    //set error on inbound and outbound
    //acitve-false
    return;
  }
  if (_lingering){
    _seg_received_lingering = true;
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

  //send ack
  if (seg.length_in_sequence_space() > 0){
    _sender.send_empty_segment();
  }
  
  //timestamp for inbound ends
  _time_last_segment_received = _time;
  if (_receiver.eof() && !_inbound_ended){
    _time_inbound_ended = _time;
    _inbound_ended = true;
  }
}

bool TCPConnection::active()const {
  //4 prereqs
  bool outbound_ackd = _sender.ack_abs() == _sender.stream_in().bytes_read() + 2;
  cout << outbound_ackd << endl;
  if (_inbound_ended && _outbound_fin_sent){
    //3 preqs satisfied explicitly
    if (outbound_ackd){
      return false;
    }//option b
    if (_time_inbound_ended < _sender.time_fin_sent()){
      return false;
    }//optian a
    if(_lingering){
      if (_time - _time_linger_started > 10 * _cfg.rt_timeout){
	if (_seg_received_lingering){
	  //lingered enough and segment receieved, start over
	  _lingering = false;
	  _seg_received_lingering = false;
	  return true;
	}
	else{
	  //lingered enough and segment not received, end
	  return false;
	}
      }
      //lingering started, but enough time passed to know for sure
      return false;
    }
    //Not lingering
    _lingering = true;
    _time_linger_started = _time;
  }
  return true;
}

size_t TCPConnection::write(const string &data) {
  //add ack info from receiver
  if (_receiver.ackno().has_value()){
    while(!_sender.segments_out().empty() ){
      TCPSegment seg = _sender.segments_out().front();
      seg.header().ack = true;
      seg.header().ackno = _receiver.ackno().value();
      seg.header().win = min(_receiver.window_size(), numeric_limits<uint64_t>::max());
      _segments_out.push(seg);
      _sender.segments_out().pop();
    }
  }
  return _sender.stream_in().write(data);
  
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) {
  //tell time to sender
  _sender.tick(ms_since_last_tick);

  //abort
  if(_sender.consecutive_retransmissions() > TCPConfig::MAX_RETX_ATTEMPTS){
    //send RST
  }

  //end cleanly
}

void TCPConnection::end_input_stream() {
  _sender.stream_in().end_input();
}

void TCPConnection::connect() {
  //send syn
  _sender.fill_window();
}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";

            //send a RST segment to the peer
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}
