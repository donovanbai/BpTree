#include "BpTree.h"

#include <algorithm>
#include <iostream>

using namespace std;


BpTree::BpTree(int n) {
    this->n = n;
    root = make_unique<Node>(n, true);
}

BpTree::BpTree(const BpTree &tree) {
    n = tree.n;
    root = make_unique<Node>(*tree.root);
}

BpTree::BpTree(BpTree&& tree) noexcept {
    n = tree.n;
    root = move(tree.root);
    tree.root = make_unique<Node>(tree.n, true); // make sure root is never null so printKeys() doesn't break
}

BpTree& BpTree::operator=(const BpTree& tree) {
    if (this != &tree) {
        n = tree.n;
        root = make_unique<Node>(*tree.root);
    }
    return *this;
}

bool BpTree::insert(int key, string value) {
    // check if key already exists
    if (locate(key) != nullptr) {
        return false;
    }
    auto new_str = make_unique<string>(value);
    Node *curr = root.get();
    long index = 0;

    // get to leaf
    while (!curr->is_leaf) {
        // follow the correct pointer
        if (index == curr->keys.size() || key < curr->keys[index]) {
            curr = curr->children[index].get();
            index = 0;
        }
        else {
            index++;
        }
    }
    // full leaf. split it
    if (curr->keys.size() == curr->get_capacity()) {
        auto new_node = make_unique<Node>(n, true); // create new leaf node
        int num_stay = (int)ceil((n + 1) / 2.0); // number of keys that stay in old node
        // find index where key supposed to be inserted
        while (index < curr->keys.size() && key > curr->keys[index]) {
            index++;
        }
        if (index < num_stay) { // if key belongs on old node
            // move some keys to new node
            for (long i = num_stay-1; i < curr->keys.size(); i++) {
                new_node->keys.insert(new_node->keys.begin() + i - (num_stay-1), curr->keys[i]);
                new_node->vals.insert(new_node->vals.begin() + i - (num_stay-1), move(curr->vals[i]));
            }
            unsigned long old_occupancy = curr->keys.size();
            curr->keys.erase(curr->keys.begin() + num_stay - 1, curr->keys.begin() + old_occupancy);
            curr->vals.erase(curr->vals.begin() + num_stay - 1, curr->vals.begin() + old_occupancy);
            //curr->occupancy = num_stay - 1;
            // shuffle_right keys in old node and insert key, value
            //curr->shuffle_right(index, num_stay - 2);
            curr->keys.insert(curr->keys.begin() + index, key);
            curr->vals.insert(curr->vals.begin() + index, move(new_str));
        }
        else { // if key belongs on new node
            // move some keys to new node
            for (long i = num_stay; i < curr->keys.size(); i++) {
                new_node->keys.insert(new_node->keys.begin() + i - num_stay, curr->keys[i]);
                new_node->vals.insert(new_node->vals.begin() + i - num_stay, move(curr->vals[i]));
            }
            unsigned long old_occupancy = curr->keys.size();
            curr->keys.erase(curr->keys.begin() + num_stay, curr->keys.begin() + old_occupancy);
            curr->vals.erase(curr->vals.begin() + num_stay, curr->vals.begin() + old_occupancy);
            //curr->keys.size() = num_stay;
            // shuffle_right keys in new node and insert key, value
            //new_node->shuffle_right(index - num_stay, new_node->keys.size() - 1);
            new_node->keys.insert(new_node->keys.begin() + index-num_stay, key);
            new_node->vals.insert(new_node->vals.begin() + index-num_stay, move(new_str));
        }
        // update pointers in both nodes
        new_node->next = curr->next;
        curr->next = new_node.get();
        // add key and pointer in parent
        insertion_update(curr->parent, new_node->keys[0], move(new_node));
        return true;
    }
    // leaf not full. just insert
    // scan keys
    while (index < curr->keys.size() && key > curr->keys[index]) {
        index++;
    }
    // if key is greater than all existing keys, insert at end
    if (index == curr->keys.size()) {
        curr->keys.insert(curr->keys.begin() + index, key);
        curr->vals.insert(curr->vals.begin() + index, move(new_str));
        return true;
    }
    // shuffle_right keys (and their pointers) that are greater
    //curr->shuffle_right(index, curr->keys.size() - 1);
    curr->keys.insert(curr->keys.begin() + index, key);
    curr->vals.insert(curr->vals.begin() + index, move(new_str));
    return true;
}

bool BpTree::remove(int key) {
    Node *node = locate(key);
    if (!node) {
        return false;
    }
    long index;
    // case 1: no redistributing/coalescing
    if (node->keys.size() > (int)floor((n + 1) / 2.0) || node == root.get()) {
        index = node->find_index(key);
        node->keys.erase(node->keys.begin() + index);
        node->vals.erase(node->vals.begin() + index);
        //node->shuffle_left(index + 1, node->keys.size() - 1);
        //node->keys.size()--;
        return true;
    }

    // case 2: redistribute values from left sibling
    long index_parent = node->parent->find_index(node);
    if (index_parent != -1) { // check that left sibling exists
        Node *left_sibling = node->parent->children[index_parent].get();
        if (left_sibling->keys.size() > (int) floor((n + 1) / 2.0)) { // check that left sibling has enough keys
            index = node->find_index(key);
            node->keys.erase(node->keys.begin() + index);
            node->vals.erase(node->vals.begin() + index);
            //node->shuffle_left(index + 1, node->keys.size() - 1);
            //node->keys.size()--;

            //node->shuffle_right(0, node->keys.size() - 1);

            node->keys.insert(node->keys.begin(), left_sibling->keys.back()); // CLion false positive error. ignore
            node->vals.insert(node->vals.begin(), move(left_sibling->vals.back()));
            left_sibling->keys.pop_back();
            left_sibling->vals.pop_back();
            //left_sibling->keys.size()--;

            node->parent->keys[index_parent] = node->keys[0];
            return true;
        }
    }

    // case 3: redistribute values from right sibling
    if (index_parent != node->parent->keys.size()-1) { // check that right sibling exists
        Node *right_sibling = node->parent->children[index_parent + 2].get();
        if (right_sibling->keys.size() > (int) floor((n + 1) / 2.0)) { // check that right sibling has enough keys
            index = node->find_index(key);
            node->keys.erase(node->keys.begin() + index);
            node->vals.erase(node->vals.begin() + index);
            //node->shuffle_left(index + 1, node->keys.size() - 1);
            //node->keys.size()--;

            node->keys.push_back(right_sibling->keys[0]);
            node->vals.push_back(move(right_sibling->vals[0]));
            right_sibling->keys.erase(right_sibling->keys.begin());
            right_sibling->vals.erase(right_sibling->vals.begin());
            //right_sibling->shuffle_left(1, right_sibling->keys.size() - 1);
            //right_sibling->keys.size()--;

            node->parent->keys[index_parent + 1] = right_sibling->keys[0];

            return true;
        }
    }

    // case 4: coalesce with left sibling
    if (index_parent != -1) {
        index = node->find_index(key);
        // delete string
        node->keys.erase(node->keys.begin() + index);
        node->vals.erase(node->vals.begin() + index);
        // shuffle keys and pointers
        //node->shuffle_left(index+1, node->keys.size()-1);
        //node->keys.size()--;
        // copy keys and pointers to left sibling
        Node *left_sibling = node->parent->children[index_parent].get();
        for (int i = 0; i < node->keys.size(); i++) {
            left_sibling->keys.push_back(node->keys[i]);
            //unique_ptr<string> s = make_unique<string>(*(string*)node->get_p(i));
            left_sibling->vals.push_back(move(node->vals[i]));
        }
        left_sibling->next = node->next;
        removal_update(left_sibling->parent, index_parent);
        return true;
    }

    // case 5: coalesce with right sibling
    index = node->find_index(key);
    node->keys.erase(node->keys.begin() + index);
    node->vals.erase(node->vals.begin() + index);
    Node *right_sibling = node->parent->children[index_parent+2].get();
    for (int i = 0; i < right_sibling->keys.size(); i++) {
        node->keys.push_back(right_sibling->keys[i]);
        node->vals.push_back(move(right_sibling->vals[i]));
    }
    node->next = right_sibling->next;
    removal_update(node->parent, index_parent+1);
    return true;
}

string BpTree::find(int key) {
    Node *node = locate(key);
    if (!node) {
        return "";
    }
    for (int i = 0; i < node->keys.size(); i++) {
        if (node->keys[i] == key) {
            return *node->vals[i];
        }
    }
    return "";
}

void BpTree::printKeys() {
    // breadth-first printing with a queue
    queue<Node*> nodes; // use pointers to save space and time
    nodes.push(root.get());
    nodes.push(nullptr); // use nullptr to indicate the end of a level
    while (!nodes.empty()) {
        // print keys in node
        Node *node = nodes.front();
        if (node == nullptr) {
            cout << endl;
            if (nodes.size() == 1) { // queue contains no more nodes
                break;
            }
            nodes.push(nullptr);
            nodes.pop();
            continue;
        }
        cout << '[';
        for (int i = 0; i < node->keys.size(); i++) {
            cout << node->keys[i];
            if (i < node->keys.size()-1) {
                cout << ',';
            }
        }
        cout << "] ";
        // if node is not a leaf, push children in queue
        if (!node->is_leaf) {
            for (int i = 0; i <= node->keys.size(); i++) {
                nodes.push(node->children[i].get());
            }
        }
        // remove node from queue
        nodes.pop();
    }
}

void BpTree::printValues() {
    Node *curr = root.get();
    // traverse to leftmost leaf
    while (!curr->is_leaf) {
        curr = curr->children[0].get();
    }
    // print all values in leaf and move to next leaf
    while (curr) {
        for (int i = 0; i < curr->keys.size(); i++) {
            cout << *curr->vals[i] << endl;
        }
        curr = curr->next;
    }
}

Node *BpTree::locate(int key) {
    Node *curr = root.get();
    int index = 0;
    // get to leaf
    while (!curr->is_leaf) {
        // follow the correct pointer
        if (index == curr->keys.size() || key < curr->keys[index]) {
            curr = curr->children[index].get();
            index = 0;
        }
        else {
            index++;
        }
    }
    // scan leaves for key
    while (curr != nullptr) {
        // scan keys from left to right
        while (index < curr->keys.size() && key > curr->keys[index]) {
            index++;
        }
        // if all keys are scanned, move to the next leaf
        if (index == curr->keys.size()) {
            curr = curr->next;
            index = 0;
            continue;
        }
        // key doesn't exist
        if (key < curr->keys[index]) {
            return nullptr;
        }
        return curr;
    }
    return nullptr;
}

void BpTree::insertion_update(Node *node, int key, unique_ptr<Node> new_child) {
    // base case: node is nullptr. make a new root
    if (node == nullptr) {
        unique_ptr<Node> new_root = make_unique<Node>(n, false);
        new_root->keys.push_back(key);
        root->parent = new_root.get();
        new_root->children.push_back(move(root));
        new_child->parent = new_root.get();
        new_root->children.push_back(move(new_child));
        root = move(new_root);
        return;
    }
    // put all the node's keys and the argument key in array, sort them
    int keys[node->keys.size()+1];
    for (int i = 0; i < node->keys.size(); i++) {
        keys[i] = node->keys[i];
    }
    keys[node->keys.size()] = key;
    sort(keys, keys+node->keys.size()+1);
    // put all the node's pointers and argument pointer in array, sort them
    unique_ptr<Node> children[node->keys.size()+2];
    children[0] = move(new_child);
    for (int i = 0; i < node->keys.size()+1; i++) {
        children[i+1] = move(node->children[i]);
        for (int j = i+1; j > 0; j--) {
            if (children[j]->keys[0] < children[j-1]->keys[0]) {
                swap(children[j], children[j-1]);
            }
            else {
                break;
            }
        }
    }
    unsigned long old_occupancy = node->keys.size();
    //node->keys.size() = 0;
    if (node->keys.size() + 1 <= node->get_capacity()) {
        for (int i = 0; i < old_occupancy + 1; i++) {
            if (i == node->keys.size()) {
                node->keys.push_back(keys[i]);
            }
            else {
                node->keys[i] = keys[i];
            }
            children[i]->parent = node;
            node->children[i] = move(children[i]);
        }
        children[node->keys.size()]->parent = node;
        node->children.push_back(move(children[node->keys.size()]));
        return;
    }
    int mid = (int)ceil(n/2.0);
    unique_ptr<Node> sibling = make_unique<Node>(n, false);
    for (int i = 0; i < mid; i++) {
        node->keys[i] = keys[i];
        children[i]->parent = node;
        node->children[i] = move(children[i]);
    }
    children[mid]->parent = node;
    node->children[mid] = move(children[mid]);
    node->keys.erase(node->keys.begin() + mid, node->keys.end());
    node->children.erase(node->children.begin() + mid + 1, node->children.end());
    for (int i = mid+1; i < n+1; i++) {
        sibling->keys.push_back(keys[i]);
        children[i]->parent = sibling.get();
        sibling->children.push_back(move(children[i]));
    }
    children[n+1]->parent = sibling.get();
    sibling->children.push_back(move(children[n+1]));
    insertion_update(node->parent, keys[mid], move(sibling));
}

void BpTree::removal_update(Node *node, long index) {
    if (!node) {
        return;
    }

    // if root only has 1 key, then make one of the children the new root
    if (node == root.get() && node->keys.size() == 1) {
        node->keys.erase(node->keys.begin() + index);
        node->children.erase(node->children.begin() + index + 1);
        unique_ptr<Node> p = move(node->children[0]);
        root = move(p);
        root->parent = nullptr;
        return;
    }

    // case: no redistribution or coalescing
    if (node->keys.size() > (int)ceil((n+1)/2.0)-1 || node == root.get()) { // check that node has enough keys
        //node->shuffle_left(index+1, node->keys.size()-1);
        //node->keys.size()--;
        node->keys.erase(node->keys.begin() + index);
        node->children.erase(node->children.begin() + index + 1);
        return;
    }

    long index_parent = node->parent->find_index(node);

    // case: redistribute with left sibling
    if (index_parent != -1) { // check that left sibling exists
        Node *left_sibling = node->parent->children[index_parent].get();
        if (left_sibling->keys.size() > (int)ceil((n+1)/2.0)-1) { // check that left sibling has enough keys
            node->keys.erase(node->keys.begin() + index);
            node->children.erase(node->children.begin() + index + 1);
            //node->shuffle_right(-1, node->keys.size() - 1);
            // get a key from parent, a pointer from left sibling
            node->keys.insert(node->keys.begin(), node->parent->keys[index_parent]);
            node->children.insert(node->children.begin(), move(left_sibling->children[left_sibling->keys.size()]));
            // update child
            node->children[0]->parent = node;
            // update parent's key
            node->parent->keys[index_parent] = left_sibling->keys[left_sibling->keys.size()-1];
            // update left sibling
            left_sibling->keys.pop_back();
            left_sibling->children.pop_back();
            return;
        }
    }

    // case: redistribute with right sibling
    if (index_parent != node->parent->keys.size()-1) { // check that right sibling exists
        Node *right_sibling = node->parent->children[index_parent + 2].get();
        if (right_sibling->keys.size() > (int)ceil((n+1)/2.0)-1) { // check that right sibling has enough keys
            node->keys.erase(node->keys.begin() + index);
            node->children.erase(node->children.begin() + index + 1);
            //node->shuffle_left(index + 1, node->keys.size() - 1);
            //node->keys.size()--;
            // get a key from parent, a pointer from right sibling
            node->keys.push_back(node->parent->keys[index_parent+1]);
            node->children.push_back(move(right_sibling->children[0]));
            // update child
            node->children.back()->parent = node;
            // update parent's key
            node->parent->keys[index_parent + 1] = right_sibling->keys[0];
            // update right sibling
            right_sibling->keys.erase(right_sibling->keys.begin());
            right_sibling->children.erase(right_sibling->children.begin());
            //right_sibling->shuffle_left(0, right_sibling->keys.size()-1);
            //right_sibling->keys.size()--;
            return;
        }
    }

    // case: coalesce with left sibling
    if (index_parent != -1) { // check that left sibling exists
        // shuffle keys and pointers
        node->keys.erase(node->keys.begin() + index);
        node->children.erase(node->children.begin() + index + 1);
        //node->shuffle_left(index+1, node->keys.size()-1);
        //node->keys.size()--;
        Node *left_sibling = node->parent->children[index_parent].get();
        // insert keys and pointers to left sibling
        left_sibling->keys.push_back(node->children[0]->keys[0]);
        for (int i = 0; i <= node->keys.size(); i++) {
            left_sibling->children.push_back(move(node->children[i]));
        }
        for (int key : node->keys) {
            left_sibling->keys.push_back(key);
        }
        removal_update(left_sibling->parent, index_parent);
        return;
    }

    // case: coalesce with right sibling
    node->keys.erase(node->keys.begin() + index);
    node->children.erase(node->children.begin() + index + 1);
    //node->shuffle_left(index+1, node->keys.size()-1);
    //node->keys.size()--;
    Node *right_sibling = node->parent->children[index_parent+2].get();
    // get keys and pointers
    node->keys.push_back(right_sibling->children[0]->keys[0]);
    for (int i = 0; i <= right_sibling->keys.size(); i++) {
        node->children.push_back(move(right_sibling->children[i]));
    }
    for (int key : right_sibling->keys) {
        node->keys.push_back(key);
    }
    removal_update(node->parent, index_parent+1);
}