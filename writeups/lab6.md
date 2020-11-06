Lab 6 Writeup
=============

My name: [Erdenebold Battulga]

My SUNet ID: [ebo2020]

This lab took me about [12] hours to do. I [did] attend the lab session.

Program Structure and Design of the Router:
[
Binary Search Tree is used as a Routing Table.
Adding to the Routing Table:
To add route to the table, we look at the upper prefix_length bits and 0's are treated as traversing leftwards and
vice versa. For instace, given prefix_length=4 and upper 4 bits=0101, we would start at the root and move left, right,
left, right and end up at either a leaf node or a previously added node. After we reach the final node, we will
add the data about interface number and hop address to the node.
Using the Table to route datagram:
Using dgram's destination ip, we traverse the tree from the root in the same way we added to the table. 
]

Implementation Challenges:
[
Wasn't sure at first whether all nodes and not just leaf nodes should store hop information.
Was on a wrong track and wondered how to get the most recent node with valid data in case the route prefix only
partially matches with an ip down the tree.
Had problem with pointers, dereferencing, implementing BST and returning root of BST.
]

Remaining Bugs:
[]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
