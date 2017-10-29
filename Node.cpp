#include "Node.h"

using namespace std;

Node::Node(int n, bool is_leaf) {
    this->is_leaf = is_leaf;
    capacity = n;
    parent = nullptr;
    next = nullptr;
}

Node::Node(Node &node, queue<Node*> *qp) {
    keys = node.keys;
    capacity = node.capacity;
    is_leaf = node.is_leaf;
    parent = nullptr;
    next = nullptr;
    queue<Node*> q;
    if (!node.is_leaf) {
        for (unique_ptr<Node> const& child : node.children) {
            unique_ptr<Node> n;
            if (!qp) {
                n = make_unique<Node>(*child, &q);
            }
            else {
                n = make_unique<Node>(*child, qp);
            }
            n->parent = this;
            children.push_back(move(n));
        }
        next = nullptr;
    }
    else {
        for (unique_ptr<string> const& s : node.vals) {
            vals.push_back(make_unique<string>(*s));
        }
        if (qp) {
            qp->push(this);
        }
    }
    // connect leaves
    if (!qp) {
        while (!q.empty()) {
            Node *n = q.front();
            q.pop();
            if (!q.empty()) {
                n->next = q.front();
            }
        }
    }
}

long Node::find_index(Node *node) { // returns the index of the key that corresponds to node

    for(long i = 0; i <= keys.size(); i++) {
        if (children[i].get() == node) {
            return i-1;
        }
    }
    return -2;
}

long Node::find_index(int key) {
    for (long i = 0; i < keys.size(); i++) {
        if (keys[i] == key) {
            return i;
        }
    }
    return -1;
}

int Node::get_capacity() {
    return capacity;
}