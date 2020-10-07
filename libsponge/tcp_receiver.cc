#include "tcp_receiver.hh"

#include <iostream>
#include <string.h>

using namespace std;

// sets ISN, comptues _ack, writes to stream_reassembler and sends input_ended signal
void TCPReceiver::segment_received(const TCPSegment &seg) {
    TCPHeader header = seg.header();
    WrappingInt32 seqno = header.seqno;
    uint64_t start_payload;

    // need handshake
    if (!_syn && !header.syn) {
        return;
    } else {
        // start_payload is set depending on SYN
        if (header.syn) {
            _syn = true;
            _isn = header.seqno;
            start_payload = unwrap(seqno, _isn, _ckpt);
        } else {
            start_payload = unwrap(seqno, _isn, _ckpt) - 1;
        }
    }

    uint64_t len_in_seq = seg.length_in_sequence_space();
    uint64_t len_payload;

    // determine len_payload
    if (header.syn && header.fin) {
        len_payload = len_in_seq - 2;
    } else if (header.syn || header.fin) {
        len_payload = len_in_seq - 1;
    } else {
        len_payload = len_in_seq;
    }

    // determine complete length of stream from len_payload
    bool eof = false;
    if (header.fin) {
        _len_stream = start_payload + len_payload;
        eof = true;
    }

    // write to _reassembler
    string data = seg.payload().copy();
    _reassembler.push_substring(data, start_payload, eof);
    _ckpt = _reassembler.stream_out().bytes_written();

    // send input_ended signal if needed and compute acknowledgement seqno
    if (_ckpt == _len_stream) {
        _ack = wrap(_ckpt + 2, _isn);
        _reassembler.stream_out().end_input();
    } else {
        _ack = wrap(_ckpt + 1, _isn);
    }
}

// returns acknowledgemnt seqno
optional<WrappingInt32> TCPReceiver::ackno() const {
    if (_syn) {
        return _ack;
    }
    return {};
}

// returns window size of acceptable bytes from the sender
size_t TCPReceiver::window_size() const { return _capacity - _reassembler.stream_out().buffer_size(); }
