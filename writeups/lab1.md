Lab 1 Writeup
=============

My name: [Erdenebold Battulga]

My SUNet ID: [ebo2020]

This lab took me about [16] hours to do. I [did not] attend the lab session.

Program Structure and Design of the StreamReassembler:
[
Push_substr is decomposed to write_to_aux and write_to_bytestream.
String is used for auxiliary storage and vector of bool is used to keep track
of which idxs are occupied
]

Implementation Challenges:
[
Using fixed length string as a sliding window for bytestream. Figuring out that
the later bytes can be in front of earlier bytes in the storage and having a tracker
for submitting actual bytestream
eof, _output.end_input(), push_subtr() usage weren't specific

]

Remaining Bugs:
[]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
