Lab 4 Writeup
=============

My name: [Erdenebold Battulga]

My SUNet ID: [ebo2020]

This lab took me about [30] hours to do. I [did not] attend the lab session.

I worked with or talked about this assignment with: [please list other sunetids]

Program Structure and Design of the TCPConnection:
[
TCPConnection's segment_received() looks into the header of an incoming segment and hadles/makes decision to:
1) pass on the segment to TCPReceiver
2) pass on the ackno and window size to TCPSender::ack_received
3) check RST and reset the connection
4) determine linger condition
5) listen()
6) sending empty segments
TCPConnection::write() writes the incoming app data into _sender.stream_in() and _sender.fill_window() will
use the bytestream data to output segments.
TCPConnection::send_segments() will look into _sender's segment queue and can add ACK, ACKNO, WIN, RST flags
and add them to its own queue. 
]

Implementation Challenges:
[
Determining linger condition - was using _time at first
Determining active() condition
Handling incoming segment and deciding what TCPConnection should be looking inside incoming segments.
Deciding when to send empty segments
Sending RST segment with or without data, what to send after RST segment.
]

Remaining Bugs:
[
My check condition for active() doesn't include unassembled_bytes==0 as it was causing timeout.
I'm not sure how _receiver.stream_out.eof() can be true while unassembled_bytes!=0 false and TA earlier said
my stream_reassembler is growing indefinitely as I was using string but I don't understanding it.
Didn't have enough time to look at this but as later labs depend on this, might have to rework on it
]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
