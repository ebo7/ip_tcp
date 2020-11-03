#include "network_interface.hh"
#include "arp_message.hh"
#include "ethernet_frame.hh"

#include <iostream>

constexpr EthernetAddress ADDR_BROADCAST = {0, 0, 0, 0, 0, 0};
using namespace std;

//! \param[in] ethernet_address Ethernet (what ARP calls "hardware") address of the interface
//! \param[in] ip_address IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface(const EthernetAddress &ethernet_address, const Address &ip_address)
    : _ethernet_address(ethernet_address), _ip_address(ip_address) {
    cerr << "DEBUG: Network interface has Ethernet address " << to_string(_ethernet_address) << " and IP address "
         << ip_address.ip() << "\n";
}

//! \param[in] dgram the IPv4 datagram to be sent
//! \param[in] next_hop the IP address of the interface to send it to (typically a router or default gateway, but may also be another host if directly connected to the same network as the destination)
//! (Note: the Address type can be converted to a uint32_t (raw 32-bit IP address) with the Address::ipv4_numeric() method.)
void NetworkInterface::send_datagram(const InternetDatagram &dgram, const Address &next_hop) {
    // convert IP address of next hop to raw 32-bit representation (used in ARP header)
    const uint32_t next_hop_ip = next_hop.ipv4_numeric();
    // IP found
    if (ip2eth.count(next_hop_ip)) {
        EthernetFrame frame;
        frame.header().src = _ethernet_address;
        frame.header().dst = ip2eth[next_hop_ip];
        frame.header().type = frame.header().TYPE_IPv4;
        frame.payload() = dgram.serialize();
        _frames_out.push(frame);
        return;
    }
    // IP not found
    ip2dgrams[next_hop_ip].push(dgram);
    // arp sent and waiting
    if (timestamps_arp.count(next_hop_ip) and _time - timestamps_arp[next_hop_ip] < 5000) {
        return;
    }
    // send arp
    timestamps_arp[next_hop_ip] = _time;
    send_arp(ARPMessage::OPCODE_REQUEST, next_hop_ip, ADDR_BROADCAST);
}

//! \param[in] frame the incoming Ethernet frame
optional<InternetDatagram> NetworkInterface::recv_frame(const EthernetFrame &frame) {
    // frame is dgram
    if (frame.header().type == frame.header().TYPE_IPv4 and frame.header().dst == _ethernet_address) {
        InternetDatagram dgram;
        Buffer payload = frame.payload().concatenate();
        if (dgram.parse(payload) == ParseResult::NoError) {
            return dgram;
        }
        return {};
    }
    // frame is arp
    ARPMessage msg;
    Buffer payload = frame.payload().concatenate();
    if (msg.parse(payload) != ParseResult::NoError) {
        return {};
    }
    uint32_t ip = msg.sender_ip_address;
    EthernetAddress eth = msg.sender_ethernet_address;
    if (!ip2eth.count(ip)) {
        timestamps_pair.push(pair(pair(ip, eth), _time));
        ip2eth[ip] = eth;
    }
    if (timestamps_arp.count(ip)) {
        timestamps_arp.erase(ip);
        // send previously queued frames
        while (!ip2dgrams[ip].empty()) {
            EthernetFrame frame_out;
            InternetDatagram dgram = ip2dgrams[ip].front();
            frame_out.payload() = dgram.serialize();
            frame_out.header().src = _ethernet_address;
            frame_out.header().dst = ip2eth[ip];
            frame_out.header().type = EthernetHeader::TYPE_IPv4;
            _frames_out.push(frame_out);
            ip2dgrams[ip].pop();
        }
    }
    // arp is request for our ip address
    if (msg.opcode == ARPMessage::OPCODE_REQUEST and msg.target_ip_address == _ip_address.ipv4_numeric()) {
        send_arp(ARPMessage::OPCODE_REPLY, msg.sender_ip_address, msg.sender_ethernet_address);
    }
    return {};
}

//! \param[in] opcode of the arp message
//! \param[in] target_ip of the arp message
//! \param[in] target_eth of the arp message
void NetworkInterface::send_arp(uint16_t opcode, uint32_t target_ip, EthernetAddress target_eth) {
    EthernetFrame frame;
    frame.header().src = _ethernet_address;
    opcode == ARPMessage::OPCODE_REPLY ? frame.header().dst = target_eth : frame.header().dst = ETHERNET_BROADCAST;
    frame.header().type = EthernetHeader::TYPE_ARP;
    ARPMessage msg;
    msg.opcode = opcode;
    msg.sender_ip_address = _ip_address.ipv4_numeric();
    msg.sender_ethernet_address = _ethernet_address;
    msg.target_ip_address = target_ip;
    msg.target_ethernet_address = target_eth;
    frame.payload() = msg.serialize();
    _frames_out.push(frame);
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick(const size_t ms_since_last_tick) {
    _time += ms_since_last_tick;
    // delete ip's that's been saved for +30 sec
    while (!timestamps_pair.empty()) {
        auto curr = timestamps_pair.front();
        if (_time - curr.second > 30000) {
            ip2eth.erase(curr.first.first);
            timestamps_pair.pop();
        } else {
            break;
        }
    }
}
