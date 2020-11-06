#include <iostream>
using namespace std;

class BST {
    int data;
    BST *left, *right;

  public:
    // Parameterized constructor.
    BST(int);

    // Insert function.
    BST *Insert(BST *, int);
};

// Parameterized Constructor definition.
BST ::BST(int value) {
    data = value;
    left = right = NULL;
}

// Insert function definition.
BST *BST ::Insert(BST *root, int prefix, int len, int ip) {
    if (!root) {
        // Insert the first node, if root is NULL.
        return new BST(value);
    }

    // Insert data.
    if (value > root->data) {
        // Insert right node data, if the 'value'
        // to be inserted is greater than 'root' node data.

        // Process right nodes.
        root->right = Insert(root->right, value);
    } else {
        // Insert left node data, if the 'value'
        // to be inserted is greater than 'root' node data.

        // Process left nodes.
        root->left = Insert(root->left, value);
    }

    // Return 'root' node, after insertion.
    return root;
}
