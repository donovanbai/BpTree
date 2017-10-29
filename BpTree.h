#include "Node.h"

#include <memory>
#include <queue>
#include <string>

using namespace std;

class BpTree {
    unique_ptr<Node> root;
    int n; // numbers of keys per node
    Node *locate(int key); // returns the node that contains key (null if key doesn't exist)
    void insertion_update(Node *node, int key, unique_ptr<Node> new_child); // used to fix the tree after splitting a node
    void removal_update(Node *node, long index); // used to fix the tree after coalescing 2 nodes

  public:
    explicit BpTree(int n);
    BpTree(const BpTree &tree);
    BpTree& operator=(const BpTree& tree);
    bool insert(int key, string value);
    bool remove(int key);
    string find(int key);
    void printKeys();
    void printValues();
};