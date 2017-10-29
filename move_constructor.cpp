#include "BpTree.h"

using namespace std;

int main() {
    BpTree tree(3);
    tree.insert(2, "2");
    tree.insert(11, "11");
    tree.insert(21, "21");
    tree.insert(8, "8");
    tree.insert(64, "64");
    tree.insert(5, "5");
    tree.insert(23, "23");
    tree.insert(6, "6");
    tree.insert(9, "9");
    tree.insert(19, "19");
    tree.insert(7, "7");
    BpTree tree2(move(tree));
    tree.printKeys();
    tree.printValues();
    tree2.printKeys();
    tree2.printValues();
    return 0;
}
