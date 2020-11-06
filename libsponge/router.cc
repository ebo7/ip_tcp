#include "router.hh"

#include <bitset>
#include <iostream>
using namespace std;

//! \param[in] route_prefix The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
//! \param[in] prefix_length For this route to be applicable, how many high-order (most-significant) bits of the route_prefix will need to match the corresponding bits of the datagram's destination address?
//! \param[in] next_hop The IP address of the next hop. Will be empty if the network is directly attached to the router (in which case, the next hop address should be the datagram's final destination).
//! \param[in] interface_num The index of the interface to send the datagram out on.
void Router::add_route(const uint32_t route_prefix,
                       const uint8_t prefix_length,
                       const optional<Address> next_hop,
                       const size_t interface_num) {
    cerr << "DEBUG: adding route " << Address::from_ipv4_numeric(route_prefix).ip() << "/" << int(prefix_length)
         << " => " << (next_hop.has_value() ? next_hop->ip() : "(direct)") << " on interface " << interface_num << "\n";
    // add route to BST routing table
    BST::Insert(_bst.root(), route_prefix, prefix_length, interface_num, next_hop);
}

//! \param[in] dgram The datagram to be routed
void Router::route_one_datagram(InternetDatagram &dgram) {
    uint32_t ip = dgram.header().dst;
    Node *curr = _bst.root();
    // keep track of most recent valid parent node
    Node *valid_parent = _bst.root();
    for (int i = 31; i >= 0; i--) {
        // go left
        if ((ip & (1 << i)) == 0) {
            if (!curr->left) {
                // not a leaf node; prefix doesnt match fully; use most recent valid parent
                if (curr->right) {
                    curr = valid_parent;
                }
                break;
            } else {
                curr = curr->left;
                // update valid parent
                if (curr->has_data) {
                    valid_parent = curr;
                }
            }
        }
        // go right
        else {
            if (!curr->right) {
                if (curr->left) {
                    curr = valid_parent;
                }
                break;
            } else {
                curr = curr->right;
                if (curr->has_data) {
                    valid_parent = curr;
                }
            }
        }
    }
    size_t num_interface = curr->num_interface;
    if (dgram.header().ttl < 2) {
        return;
    }
    dgram.header().ttl--;
    optional<Address> next_hop = curr->data;
    // Indirect
    if (next_hop.has_value()) {
        interface(num_interface).send_datagram(dgram, next_hop.value());
    }
    // Direct
    else {
        interface(num_interface).send_datagram(dgram, Address::from_ipv4_numeric(ip));
    }
}

//! Goes through all the interfaces, and route every incoming datagram to its proper outgoing interface.
void Router::route() {
    for (auto &interface : _interfaces) {
        auto &queue = interface.datagrams_out();
        while (not queue.empty()) {
            route_one_datagram(queue.front());
            queue.pop();
        }
    }
}

//! BST Constructor
BST::BST() : _root(new Node) {}

//! Inserts interface number and Address information for a given prefix and its length
//! \param[in] root Pointer to the current node
//! \param[in] prefix Route prefix to put in the Routing Table/BST
//! \param[in] len Remaining length of route prefix to parse
//! \param[in] num Interface Number to store in the final leaf node
//! \param[in] addr Address to store for indirect routes
void BST::Insert(Node *root, uint32_t prefix, uint32_t len, size_t num, optional<Address> addr) {
    if (len == 0) {
        root->num_interface = num;
        root->data = addr;
        root->has_data = true;
        return;
    }
    // go left
    if (prefix >> 31 == 0) {
        // create node
        if (!root->left) {
            root->left = new Node();
            Insert(root->left, prefix << 1, len - 1, num, addr);
        } else {
            Insert(root->left, prefix << 1, len - 1, num, addr);
        }
    }
    // go right
    else {
        if (!root->right) {
            root->right = new Node();
            Insert(root->right, prefix << 1, len - 1, num, addr);
        } else {
            Insert(root->right, prefix << 1, len - 1, num, addr);
        }
    }
}
