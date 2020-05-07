#include <cassert>
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <utility>

using namespace std;

class LRUCache {
private:
    void _reEnqueue(int key) {
        // Ensure the key is at the back of the queue.
        list<int>::iterator it = q.begin();
	    for (; it != q.end() && *it != key; ++it) {}
        if (it != q.end()) {
	        q.erase(it);
        }
        q.push_back(key);
    }

public:
	LRUCache(int capacity) : _capacity(capacity) {};

	void put(int key, int value) {
        if (m.find(key) != m.end()) {
            _reEnqueue(key);
        } else {
            q.push_back(key);
        }
        m[key] = value;

	    if (q.size() > _capacity) {
	        m.erase(q.front());
            q.pop_front();
	    }
        assert(m.size() <= _capacity);
        assert(q.size() <= _capacity);
    }
	
	int get(int key) {
        map<int, int>::iterator it = m.find(key);
	    if (it == m.end()) {
			// Key not in the cache.
		    return -1;
	    }
	
        _reEnqueue(key);
	    return it->second;
    }

    void printQueue() {
        cout << "Queue:" << endl;
        string out = " <[ ";
        for (auto& k : q) {
            out.append(to_string(k) + " ");
        }
        out += "]<";
        cout << out << endl;
    }

    void printMap() {
        cout << "Map" << endl;
        string out = "[ ";
        for (auto& p : m) {
            out.append("(" + to_string(p.first) + ", " + to_string(p.second) + ") ");
        }
        out += "]";
        cout << out << endl;
    }

    void print() {
        printQueue();
        printMap();
    }

private:
	int _capacity;
	map<int, int> m;
	list<int> q;
};


int main() {
    int capacity = 2;
    LRUCache* cache = new LRUCache(capacity);
    
    cout << "cache->put(1, 1)" << endl;
    cache->put(1, 1);
    cache->print();
    cout << endl;

    cout << "cache->put(2, 2)" << endl;
    cache->put(2, 2);
    cache->print();
    cout << endl;

    cout << "cache->get(1): " << cache->get(1) << endl;
    
    cout << "cache->put(3, 3)" << endl;
    cache->put(3, 3);
    cache->print();
    cout << endl;
    
    cout << "cache->get(2): " << cache->get(2) << endl;
    
    cout << "cache->put(4, 4)" << endl;
    cache->put(4, 4);
    cache->print();
    cout << endl;
    
    cout << "cache->get(1): " << cache->get(1) << endl;
    cout << "cache->get(3): " << cache->get(3) << endl;
    cout << "cache->get(4): " << cache->get(4) << endl;
    return 0;
}

