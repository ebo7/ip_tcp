#include "tcp_connection.hh"

#include <algorithm>
#include <iostream>
#include <limits>
using namespace std;

size_t TCPConnection::remaining_outbound_capacity() const { return _sender.stream_in().remaining_capacity(); }

size_t TCPConnection::bytes_in_flight() const { return _sender.bytes_in_flight(); }

size_t TCPConnection::unassembled_bytes() const { return _receiver.unassembled_bytes(); }

size_t TCPConnection::time_since_last_segment_received() const { return _time - _time_last_segment_received; }

void TCPConnection::segment_received(const TCPSegment &seg) {
    TCPHeader header = seg.header();
    // relay ack to sender
    if (header.ack) {
        WrappingInt32 ackno = header.ackno;
        uint16_t window_size = header.win;
        _sender.ack_received(ackno, window_size);
    }

    // pass seg to receiver
    if (seg.length_in_sequence_space() != 0) {
        _receiver.segment_received(seg);
    }

    // listen: syn receieved without ack
    if (header.syn && !header.ack && !_connect_called) {
        connect();
        return;
    }

    // reset connection
    if (header.rst && _connect_called && active()) {
        _rst_received = true;
        _sender.stream_in().set_error();
        _receiver.stream_out().set_error();
    }
    _time_last_segment_received = _time;

    // send empty
    if (  // from piazza
        (_receiver.ackno().has_value() and (seg.length_in_sequence_space() == 0) and
         seg.header().seqno == _receiver.ackno().value() - 1) or
        // handout
        (seg.length_in_sequence_space() > 0)) {
        _sender.send_empty_segment();
    }
    // linger condition
    if (header.fin && !_sender.stream_in().eof()) {
        _linger_after_streams_finish = false;
    }
    send_segments();
}

bool TCPConnection::active() const {
    // unclean shutdown
    if (_rst_received || _rst_to_send) {
        return false;
    }
    // clean shutdown
    if (_receiver.stream_out().eof() && bytes_in_flight() == 0 && _sender.stream_in().eof()) {
        // not linger when passive close(option b)
        if (!_linger_after_streams_finish) {
            return false;
        }
        // linger for inactivity (option a)
        if (time_since_last_segment_received() >= 10 * _cfg.rt_timeout) {
            return false;
        }
    }
    return true;
}

size_t TCPConnection::write(const string &data) {
    size_t ans = _sender.stream_in().write(data);
    _sender.fill_window();
    send_segments();
    return ans;
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) {
    _time += ms_since_last_tick;
    // tell time to sender
    _sender.tick(ms_since_last_tick);
    if (_sender.consecutive_retransmissions() > TCPConfig::MAX_RETX_ATTEMPTS) {
        _rst_to_send = true;
        _sender.stream_in().set_error();
        _receiver.stream_out().set_error();
    }
    send_segments();
}

void TCPConnection::end_input_stream() {
    _sender.stream_in().end_input();
    // send fin
    _sender.fill_window();
    send_segments();
}

void TCPConnection::connect() {
    // send syn or synack
    _connect_called = true;
    _sender.fill_window();
    send_segments();
}

// transfer segments from sender queue to connection queue
void TCPConnection::send_segments() {
    while (!_sender.segments_out().empty()) {
        TCPSegment seg = _sender.segments_out().front();
        if (_receiver.ackno().has_value()) {
            seg.header().ack = true;
            seg.header().ackno = _receiver.ackno().value();
            seg.header().win = min(_receiver.window_size(), static_cast<size_t>(numeric_limits<uint16_t>::max()));
        }
        if (_rst_to_send) {
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
            _rst_to_send = true;
            _sender.stream_in().set_error();
            _receiver.stream_out().set_error();
            _sender.send_empty_segment();
            send_segments();
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}
