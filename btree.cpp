#include <iostream>
#include <cstdlib>
#include <fstream>
#include <list>
#include <queue>
using namespace std;

// each node can have at max 'order' keys and 'order + 1' pointers
unsigned int order, numBuffers, bufferSize;

class BTree {
    bool isLeaf;
    list<int> keys;
    list<BTree *> pointers;

    public:
    BTree(bool internal) {
        if(internal) {
            isLeaf = false;
        }
        else {
            isLeaf = true;
            pointers.push_back(NULL);
        }
    }

    void getKeyPointer(BTree *node, int x, list<int>::iterator &it, list<BTree *>::iterator &itp) {
        itp = node->pointers.begin();
        for(it = node->keys.begin(); it != node->keys.end(); it++) {
            // it is sufficient to check (x <= keys[i]) for every pointer except last one
            // equality condition is to handle duplicates correctly
            if(x <= *it) {
                break;
            }
            itp++;
        }
        // if no pointer was followed till now, then itp will point to the last pointer
    }

    BTree * traverse(int x) {
        BTree *node = this;
        list<BTree *>::iterator itp;
        list<int>::iterator it;
        while(!node->isLeaf) {
            getKeyPointer(node, x, it, itp);
            node = *itp;
        }
        return node;
    }

    BTree * insert(int x) {
        BTree *root = this, *retnode;
        retnode = insertion(x);
        // if retnode is not null, then new root has to be added
        if(retnode) {
            // insert the key, two pointers for root
            BTree *node = new BTree(true);
            node->pointers.push_back(root);
            node->pointers.push_back(retnode);
            node->keys.push_back(retnode->keys.front());
            // remove the middle key inserted in the transfer process
            if(!retnode->isLeaf) {
                retnode->keys.pop_front();
            }
            root = node;
        }
        return root;
    }

    void transferElements(BTree *node, list<int>::iterator &it, list<BTree *>::iterator &itp) {
        int k;
        if(isLeaf) {
            k = keys.size() - (order + 1)/2;
        }
        else {
            k = keys.size() - (order + 2)/2;
        }
        while(k--) {
            it--;
            itp--;
            node->keys.push_front(*it);
            node->pointers.push_front(*itp);
            keys.erase(it);
            pointers.erase(itp);
        }
    }

    BTree * insertion(int x) {
        list<BTree *>::iterator itp;
        list<int>::iterator it;
        if(isLeaf) {
            // insert the new (key, pointer) pair in the node
            getKeyPointer(this, x, it, itp);
            pointers.insert(itp, NULL);
            keys.insert(it, x);

            // create new node if more keys than 'order'
            BTree *node = NULL;
            if(keys.size() > order) {
                node = new BTree(false);
                it = keys.end();
                itp = pointers.end();
                // adjust the linked list pointers
                *(node->pointers.begin()) = *(--itp);
                *itp = node;
                transferElements(node, it, itp);
            }
            return node;
        }

        // recursively traverse to the leaf node
        getKeyPointer(this, x, it, itp);
        BTree *retnode = (*itp)->insertion(x);

        // make changes in the parent internal nodes if needed
        if(retnode) {
            // insert the new (key, retnode) pair in the node
            pointers.insert(++itp, retnode);
            keys.insert(it, retnode->keys.front());
            // remove the middle key inserted in the transfer process
            if(!retnode->isLeaf) {
                retnode->keys.pop_front();
            }

            // create new node if more keys than 'order'
            BTree *node = NULL;
            if(keys.size() > order) {
                node = new BTree(true);
                it = keys.end();
                itp = pointers.end();
                transferElements(node, it, itp);
            }
            return node;
        }
        return retnode;
    }

    bool find(int x) {
        BTree *node = traverse(x);
        // for a leaf node
        bool flag = 0;
        do {
            for(list<int>::iterator it = node->keys.begin(); it != node->keys.end(); it++) {
                if(*it == x) {
                    return true;
                }
                if(*it > x) {
                    flag = 1;
                    break;
                }
            }
            node = node->pointers.back();
        // while we have not reached end of linked list or found a number > x
        } while(!flag && node);
        return false;
    }

    int count(int x) {
        BTree *node = traverse(x);
        // for a leaf node
        bool flag = 0;
        int cnt = 0;
        do {
            for(list<int>::iterator it = node->keys.begin(); it != node->keys.end(); it++) {
                if(*it == x) {
                    cnt++;
                }
                if(*it > x) {
                    flag = 1;
                    break;
                }
            }
            node = node->pointers.back();
        // while we have not reached end of linked list or found a number > x
        } while(!flag && node);
        return cnt;
    }

    int range(int x, int y) {
        // if not a valid range
        if(x > y) {
            return 0;
        }
        BTree *node = traverse(x);
        // for a leaf node
        bool flag = 0;
        int cnt = 0;
        do {
            for(list<int>::iterator it = node->keys.begin(); it != node->keys.end(); it++) {
                if(*it >= x && *it <= y) {
                    cnt++;
                }
                if(*it > y) {
                    flag = 1;
                    break;
                }
            }
            node = node->pointers.back();
        // while we have not reached end of linked list or found a number > x
        } while(!flag && node);
        return cnt;
    }

    void print() {
        if(isLeaf) {
            cout << "Leaf";
        }
        else {
            cout << "Internal";
        }
        for(list<int>::iterator it = keys.begin(); it != keys.end(); it++) {
            cout << ' ' << *it;
        }
        cout << '\n';
        for(list<BTree *>::iterator it = pointers.begin(); it != pointers.end() && *it; it++) {
            (*it)->print();
        }
        //cout << '\n';
    }

};
BTree *root = new BTree(false);

// Buffered IO
// assuming the size of Input is 14, as the largest size of string is 6 characters and there are 2 integers
class Input {
    public:
    string s;
    int x, y;

    friend istream &operator>>(istream &input, Input &I) { 
        input >> I.s >> I.x;
        if(I.s == "RANGE") {
            input >> I.y;
        }
        return input;            
    }
    
};

// asuming output buffer is of size 4, as it only has either one integer or a string of max length 3
class Output {
    public:
    string s;
    int x;

    Output() {
        s = "";
    }
    
    friend ostream &operator<<(ostream &output, const Output &O) {
        if(O.s == "") {
            output << O.x;
        }
        else {
            output << O.s;
        }
        return output;            
    }
    
};

queue<Input> inputBuffer;
queue<Output> outputBuffer;
unsigned int inputSize, outputSize;

void clearOutput() {
    Output op;
    while(!outputBuffer.empty()) {
        op = outputBuffer.front();
        outputBuffer.pop();
        cout << op << '\n';
    }
}

void clearInput() {
    Input inp;
    while(!inputBuffer.empty()) {
        inp = inputBuffer.front();
        inputBuffer.pop();
        Output o;
        
        if(inp.s == "INSERT") {
            root = root->insert(inp.x);
            //root->print();
            continue;
        }
        else if(inp.s == "FIND") {
            if(root->find(inp.x)) {
                o.s = "YES";
            }
            else {
                o.s = "NO";
            }
        }
        else if(inp.s == "COUNT") {
            o.x = root->count(inp.x);
        }
        else if(inp.s == "RANGE") {
            o.x = root->range(inp.x, inp.y);
        }
        if(outputBuffer.size() == outputSize) {
            clearOutput();
        }
        outputBuffer.push(o);
    }
}

int main(int argc, char *argv[]) {
    if(argc < 4) {
        cout << "Usage : ./a.out <filename> <numBuffers> <bufferSize>\n";
        exit(-1);
    }
    ifstream input(argv[1]);
    numBuffers = atoi(argv[2]);
    bufferSize = atoi(argv[3]);
    if(numBuffers < 2 || bufferSize < 20) {
        cout << "Ensure that M>=2 and B>=20\n";
        exit(-1);
    }

    // 4*order + 8*(order + 1) <= B, as block size = buffer size and memory required for keys and pointers
    order = (bufferSize - 8) / 12;

    // as sizeof(Input) is 14
    inputSize = (numBuffers - 1) * (bufferSize / 14);
    outputSize = bufferSize / 4;
    Input inp;
    while(input >> inp) {
        if(inputBuffer.size() < inputSize) {
            inputBuffer.push(inp);
        }
        else {
            clearInput();
            inputBuffer.push(inp);
        }
    }
    clearInput();
    clearOutput();
    return 0;
}
