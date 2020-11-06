Lab 6 Writeup
=============

My name: [Erdenebold Battulga]

My SUNet ID: [ebo2020]

This lab took me about [13] hours to do. I [did] attend the lab session.

Program Structure and Design of the Router:
[
Binary Search Tree is used as a Routing Table.
Adding to the Routing Table:
To add route to the table/bst, we look at the upper prefix_length bits and 0's are treated as traversing leftwards and
vice versa. For instace, given prefix_length=4 and upper 4 bits=0101, we would start at the root and move left, right,
left, right and end up at either a leaf node or a previously added node. After we reach our final node, we will
add the data about interface number and hop address to the node.
Using the Table to route datagram:
Using dgram's destination ip, we traverse the tree from the root in the same way we added to the table. Since all bits
won't match perfectly, we have to keep track of the most recent parent with data, which is equivalent to a route with
most prefix matches. For instance, if we were using only 4 bit ip's and the tree had 1100/2 (node-11) and 1111/4
(node-1111) and we're looking route for 1110, we should match 1100/2. If we didn't keep track of the most recent valid
parent, we would end up at node-111 (right,right,right) in our tree, which wouldn't contain any information. But, we
will reach node-111 from node-11 and since node-11 contains data, we can mark it as 111's valid parent and use the data
of node-11.
]

Implementation Challenges:
[
Choosing which nodes to store hop information-thought only leaf nodes would store data but earlier nodes can store too.
Getting the most recent node with valid data in case the route prefix is stuck between shorter and longer prefix nodes-assumed this wouldn't happen as I thought only leaf nodes would store data
Also had problem with implementing BST-class template, pointers, dereferencing, etc.
]

Remaining Bugs:
[Not sure]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
