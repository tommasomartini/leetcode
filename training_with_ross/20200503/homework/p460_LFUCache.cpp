#include <iostream>
#include <list>
#include <map>
#include <utility>

using namespace std;

struct Entry {
    int key;
    int value;
    int numHits;
    int lastHit;

    bool operator<(Entry const & rhs) const {
        // The "lower" entry is the one with more priority to be evicted:
        // * least freqeuntly used;
        // * lest recently used.
        if (numHits < rhs.numHits) {
            return true;
        } else if (numHits > rhs.numHits) {
            return false;
        } else {
            return lastHit < rhs.lastHit;
        }
    }
};

class LFUCache {
public:
    LFUCache(int capacity) : _capacity(capacity) {}

    // Update the position of the element pointed by the provided iterator
    // in the list.
    void _update(list<Entry>::iterator& it) {
        // When we erase we get an iterator to the next element.
        list<Entry>::iterator nextIt = _l.erase(it);
        
        // Where should we insert this element back?
        for (; nextIt != _l.end() && *nextIt < *it; ++nextIt) {}
        
        it = _l.insert(nextIt, *it);
    }

    int get(int key) {
        _count++;

        map<int, list<Entry>::iterator>::iterator mIt = _cache.find(key);
        if (mIt == _cache.end()) {
            // Cache miss.
            return -1;
        }

        // Element found: retrieve it, update it and return it.
        list<Entry>::iterator lIt = mIt->second;
        lIt->numHits++;
        lIt->lastHit = _count;
        _update(lIt);
        return lIt->value;
    }

    void put(int key, int value) {
        _count++;

        map<int, list<Entry>::iterator>::iterator mIt = _cache.find(key);
        if (mIt != _cache.end()) {
            // Update existing value.
            list<Entry>::iterator lIt = mIt->second;
            lIt->value = value;
            lIt->numHits++;
            lIt->lastHit = _count;
            _update(lIt);
        } else {
            // Insert a new value.

            if (_cache.size() == _capacity) {
                // We are at max capacity: remove the LFU element before inserting a new value.
                int lfuKey = _l.front().key;
                _cache.erase(lfuKey);
                _l.pop_front();
            }

            // Insert the new value in the list...
            Entry entry;
            entry.key = key;
            entry.value = value;
            entry.numHits = 1;
            entry.lastHit = _count;
            list<Entry>::iterator lIt = _l.insert(_l.begin(), entry);
            _update(lIt);

            // ...and in the cache.
            _cache.insert(pair<int, list<Entry>::iterator>(key, lIt));
        }
    }

private:
    int _capacity;
    int _count;

    // Stores all the elements in the cache in priority order: the front is always
    // the next element to be evicted.
    list<Entry> _l;

    // Maps a key to a pointer in the list.
    map<int, list<Entry>::iterator> _cache;
};

int main() {
    int capacity = 2;
    LFUCache* obj = new LFUCache(capacity);

    cout << "obj->put(1, 1);" << endl;
    obj->put(1, 1);

    cout << "obj->put(2, 2);" << endl;
    obj->put(2, 2);

    int param_1 = obj->get(1);
    cout << "int param_1 = obj->get(1); " << param_1 << endl;

    cout << "obj->put(3, 3);" << endl;
    obj->put(3, 3);

    int param_2 = obj->get(2);
    cout << "int param_2 = obj->get(2); " << param_2 << endl;

    int param_3 = obj->get(3);
    cout << "int param_3 = obj->get(3); " << param_3 << endl;

    cout << "obj->put(4, 4);" << endl;
    obj->put(4, 4);

    int param_4 = obj->get(1);
    cout << "int param_4 = obj->get(1); " << param_4 << endl;

    int param_5 = obj->get(3);
    cout << "int param_5 = obj->get(3); " << param_5 << endl;

    int param_6 = obj->get(4);
    cout << "int param_6 = obj->get(4); " << param_6 << endl;

    return 0;
}

