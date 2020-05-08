#include <iostream>
#include <list>
#include <unordered_map>
#include <utility>

using namespace std;

struct Entry {
    int key;
    int value;
    int numHits;
    int lastHit;

    bool operator<(Entry const & rhs) const {
        // The "lower" entry is the one with more priority to be evicted:
        // * least frequently used;
        // * least recently used.
        if (numHits < rhs.numHits) {
            return true;
        } else if (numHits > rhs.numHits) {
            return false;
        } else {
            return lastHit < rhs.lastHit;
        }
    }
};

typedef list<Entry>::iterator entryIt;

class LFUCache {
public:
    LFUCache(int capacity) : _capacity(capacity), _count(0) {}
    int get(int key);
    void put(int key, int value);

private:
    void _update(entryIt& it);
    
    int _capacity;
    int _count;

    // Stores all the elements in the cache in priority order: the front is always
    // the next element to be evicted.
    list<Entry> _l;

    // Maps a key to a pointer in the list.
    unordered_map<int, entryIt> _cache;
};

// Update the position of the element pointed by the provided iterator
// in the list.
void LFUCache::_update(entryIt& it) {
    // When we erase we get an iterator to the next element, but we invalidate
    // the erased iterator: make a copy of the entry first.
    Entry entry(*it);
    entryIt nextIt = _l.erase(it);
    
    // Where should we insert this element back?
    for (; nextIt != _l.end() && *nextIt < entry; ++nextIt) {}
    
    it = _l.insert(nextIt, entry);
}

int LFUCache::get(int key) {
    ++_count;
    unordered_map<int, entryIt>::iterator mIt = _cache.find(key);
    if (mIt == _cache.end()) {
       // Cache miss.
        return -1;
    }

    // Element found: retrieve it, update it and return it.
    entryIt& lIt = mIt->second;
    lIt->numHits++;
    lIt->lastHit = _count;
    _update(lIt);
    return lIt->value;
}

void LFUCache::put(int key, int value) {
    ++_count;
    if (_capacity == 0) {
        return;
    }

    unordered_map<int, entryIt>::iterator mIt = _cache.find(key);
    if (mIt != _cache.end()) {
        // Update existing value.
        entryIt& lIt = mIt->second;
        lIt->value = value;
        lIt->numHits++;
        lIt->lastHit = _count;
        _update(lIt);
    } else {
        // Insert a new value.
        if (_cache.size() == _capacity && _capacity > 0) {
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
        _l.push_front(entry);
        entryIt lIt = _l.begin();
        _update(lIt);

        // ...and in the cache.
        _cache.insert(pair<int, entryIt>(key, lIt));
    }
}


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

