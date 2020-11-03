Lab 5 Writeup
=============

My name: [Erdenebold Battulga]

My SUNet ID: [ebo2020]

This lab took me about [10] hours to do. I [did not] attend the lab session.

Program Structure and Design of the NetworkInterface:
[
NetworkInterface::send_datagram:
-Sends known ip datagrams and and queues unknown ip datagrams using ip-to-datagram map.
-Broadcasts ARPRequest using NetworkInterface::send_arp.
NetworkInterface::recv_frame:
-Parses incoming IPv4 datagram
-Makes ip-ethernet pairing from any incoming ARPMessage using ip2eth map and stores the time in timestamps_pair queue.
-Sends previously queued datagrams if the ip becomes known.
-Sends ARPReply using NetworkInterface::send_arp if incoming ARPRequest is asking for our ip.
NetworkInterface::send_arp: Sends ARPMessage given the parameters opcode, target ip address and target ethernet address.
NetworkInterface::tick: Keeps track of the current time using _time. Checks through queue of timestamps for ip-ethernet pairings and deletes pairings that has been saved for +30sec.
]

Implementation Challenges:
[
I didn't get why the expected ARPRequest ethernet destination was different from ARPMessage target ethernet address.
]

Remaining Bugs:
[]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
