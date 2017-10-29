#include <memory>
#include <queue>
#include <vector>

using namespace std;

class Node {
    int capacity; // max numbers of keys that can be stored

  public:
    vector<int> keys;
    vector<unique_ptr<Node>> children;
    vector<unique_ptr<string>> vals;
    bool is_leaf;
    Node *parent;
    Node *next;

    Node(int n, bool is_leaf);
    Node(Node &node, queue<Node*> *qp = nullptr); // copies subtree
    int get_capacity();
    long find_index(Node *node); // find the index of the key that corresponds to node
    long find_index(int key);
};
