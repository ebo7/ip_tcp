Lab 3 Writeup
=============

My name: [Erdenebold Battulga]

My SUNet ID: [ebo2020]

This lab took me about [15] hours to do. I [did] attend the lab session.

I worked with or talked about this assignment with: [please list other sunetids]

Program Structure and Design of the TCPSender:
[
deque<TCPSegment> _segments_out is used to track outstanding TCP segments.
deque<uint64_t> _seqnos_abs_outstanding is used to track (last seqno+1) of segments.
deque<uint64_t> _timestamps_outstanding is used to store timestamps when segments are sent or resent.
When ACK is received, corresponding segments in all deques are popped in ack_received()
_timestamps_oustanding can store all outstanding timestamps in case we have to re-send multiple timed out segments at once.
_time is used to indicate overall time and to check whether earliest outstanding segment is timed out,
we compare current RTO value with _time - timestamp.
_window_size is initialized at 1 and only changed with incoming window_size and _window_size-_bytes_flying is
used to see how much space should fill_window fill right now. fill_window never sends the same segment twice as
and state variables _seqno, _window_size, _bytes_flying, _fin_sent are used to make sure fill_window sends
correct segments.

]

Implementation Challenges:
[
Different cases when _window_size=0, depending on bytes_in_flight() etc.
Timer was confusing as I could have single timer for earleist segment or multiple timers for each segment with
different RTOs and timestamp.
Piggyback FIN with payload
State variables keeping track of the progress of TCPSender 
]

Remaining Bugs:
[Not sure]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
