
#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <algorithm>
#include <iostream>
#include <random>

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity)
    , _curr_RTO(_initial_retransmission_timeout) {}

//_bytes_flying is increased as segments are sent first time and decreased with ACK
uint64_t TCPSender::bytes_in_flight() const { return _bytes_flying; }

// fills window with segments; sends size 1 segments if there is no window space and flying bytes
void TCPSender::fill_window() {
    // send empty ack
    //  if(_stream.eof() && _fin_sent){
    // TCPSegment seg;
    // seg.header().seqno =
    //}
    // cout<<"inside fill" << endl;
    // cout<<_stream.eof() <<endl;
    // cout<<_stream.input_ended()<<endl;
    // Send SYN
    if (_next_seqno == 0) {
        // cout<<"sending syn" << endl;
        TCPSegment seg;
        seg.header().syn = true;
        seg.header().seqno = _isn;
        // Change state vars
        _segments_out.push(seg);
        _segments_outstanding.push_back(seg);
        _bytes_flying++;
        _next_seqno++;
        _seqnos_abs_outstanding.push_back(_next_seqno);
        // Plaeholder for timestamp
        _timestamps_outstanding.push_back(_time);
        return;
    }

    // Send FIN
    if (_stream.eof() && !_fin_sent && (_window_size > _bytes_flying || (_window_size == 0 && _bytes_flying == 0))) {
        //  cout<<"send fin" << endl;
        TCPSegment seg;
        seg.header().fin = true;
        seg.header().seqno = next_seqno();
        seg.payload() = static_cast<Buffer>("");

        // Change state vars
        _segments_out.push(seg);
        _segments_outstanding.push_back(seg);
        _bytes_flying++;
        _next_seqno++;
        _seqnos_abs_outstanding.push_back(_next_seqno);
        // Plaeholder for timestamp
        _timestamps_outstanding.push_back(_time);
        _fin_sent = true;
        _time_fin_sent = _time;
        return;
    }

    // Window unavailable, send size 1 payload
    if (_stream.buffer_size() > 0 && _window_size == 0 && _bytes_flying == 0) {
        TCPSegment seg;
        seg.header().seqno = next_seqno();
        seg.payload() = static_cast<Buffer>(_stream.read(1));
        _segments_out.push(seg);
        _segments_outstanding.push_back(seg);
        _bytes_flying++;
        _next_seqno++;
        _seqnos_abs_outstanding.push_back(_next_seqno);
        // Plaeholder for timestamp
        _timestamps_outstanding.push_back(_time);
        return;
    }

    // Window available
    while (_stream.buffer_size() > 0 && _window_size > _bytes_flying) {
        TCPSegment seg;
        uint64_t size_seg = min(min(_stream.buffer_size(), _window_size - _bytes_flying), TCPConfig::MAX_PAYLOAD_SIZE);
        seg.header().seqno = next_seqno();
        seg.payload() = static_cast<Buffer>(_stream.read(size_seg));
        _bytes_flying += size_seg;
        // Piggyback FIN
        if (_stream.eof() && !_fin_sent && _window_size > _bytes_flying) {
            seg.header().fin = true;
            _fin_sent = true;
            _bytes_flying++;
            _next_seqno++;
            _time_fin_sent = _time;
        }
        // Change state vars
        _segments_out.push(seg);
        _segments_outstanding.push_back(seg);
        _next_seqno += size_seg;
        _seqnos_abs_outstanding.push_back(_next_seqno);
        // Plaeholder for timestamp
        _timestamps_outstanding.push_back(_time);
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size Thte remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    // cout << "ack: " << ackno << endl;
    // cout << "isn: " << _isn << endl;
    // cout << "next_seqno: " << _next_seqno << endl;
    // cout <<"stream read: " << _stream.bytes_read() << endl;

    // WrappingInt32 a = WrappingInt32(2);
    // WrappingInt32 b =WrappingInt32(0);
    // uint64_t c = unwrap(a , b, 0);
    // cout << c << endl;
    // cout << ackno << endl;
    uint64_t ack_abs = unwrap(ackno, _isn, _stream.bytes_read());
    // cout<<"ack abs" << ack_abs<<endl;
    // cout <<"isn: "
    _window_size = window_size;
    // cout << "ack_abs: " << ack_abs << endl;
    // invalid or useless ack received
    // cout <<ack_abs<<endl;
    // cout<<_ack_abs<<endl;
    if (ack_abs > _stream.bytes_read() + 2 || ack_abs <= _ack_abs ||
        (!_fin_sent && ack_abs > _stream.bytes_read() + 1)) {
        return;
    }
    // cout<<"useful"<<endl;
    //_window_size = window_size;
    // cout<<_window_size<<endl;
    // new ACK received
    _curr_RTO = _initial_retransmission_timeout;
    uint64_t bytes_ack = ack_abs - _ack_abs;
    _bytes_flying -= bytes_ack;
    _ack_abs = ack_abs;
    _consec_retrans = 0;

    // Looks thru outstanding acks and remove acknowledged ones
    while (!_seqnos_abs_outstanding.empty()) {
        if (_ack_abs >= _seqnos_abs_outstanding.front()) {
            _seqnos_abs_outstanding.pop_front();
            _segments_outstanding.pop_front();
            _timestamps_outstanding.pop_front();
        } else {
            break;
        }
    }

    // Reset the leftover timestamps and fill window
    for (size_t i = 0; i < _timestamps_outstanding.size(); i++) {
        _timestamps_outstanding[i] = _time;
    }
    if ((_window_size > _bytes_flying) || (!_window_size && !_bytes_flying)) {
        if (!_fin_sent) {
            // cout<<"fill"<<endl;
            TCPSender::fill_window();
        }
    }
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    if (_timestamps_outstanding.empty()) {
        return;
    }
    _time += ms_since_last_tick;

    // Retransmit earliest segment
    if (_time - _timestamps_outstanding[0] >= _curr_RTO) {
        _segments_out.push(_segments_outstanding[0]);
        _timestamps_outstanding[0] = _time;
        if (_window_size != 0) {
            _consec_retrans++;
            _curr_RTO *= 2;
        }
    }
}

// Signals if the connection is hopeless
unsigned int TCPSender::consecutive_retransmissions() const { return _consec_retrans; }

// Send empty segment for empty ACK
void TCPSender::send_empty_segment() {
    TCPSegment seg;
    seg.header().seqno = next_seqno();
    //    seg.payload() = static_cast<Buffer>("");
    _segments_out.push(seg);
}
