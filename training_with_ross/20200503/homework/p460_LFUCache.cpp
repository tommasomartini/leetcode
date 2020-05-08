#include <iostream>
#include <list>
#include <unordered_map>
#include <utility>

#define DBG 0

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

#ifdef DBG
    cout << "  Start update(" << it->key << ")" << endl;
    int initSize = _l.size();
#endif

    // When we erase we get an iterator to the next element, but we invalidate
    // the erased iterator: make a copy of the entry first.
    Entry entry(*it);
    entryIt nextIt = _l.erase(it);
    
#ifdef DBG
    int newSize = _l.size();
    cout << "  erased entry " << entry.key << " (_l's size went from " << initSize << " to " << newSize << ")" << endl;
#endif

    // Where should we insert this element back?
    for (; nextIt != _l.end() && *nextIt < entry; ++nextIt) {}
    
#ifdef DBG
    if (nextIt == _l.end()) {
        cout << "  nextIt points to _l.end()" << endl;
    } else {
        cout << "  nextIt points to entry with key " << nextIt->key << endl;
    }
#endif

    it = _l.insert(nextIt, entry);

#ifdef DBG
    cout << "  entry inserted. Size of _l is " << _l.size() << endl;
    cout << "  End update(" << it->key << ")" << endl;
#endif
}

int LFUCache::get(int key) {
#ifdef DBG
    cout << "Start get(" << key << ")" << endl;
#endif
    
    ++_count;

#ifdef DBG
    cout << " _count=" << _count << endl;
#endif

    unordered_map<int, entryIt>::iterator mIt = _cache.find(key);
    if (mIt == _cache.end()) {
       // Cache miss.
#ifdef DBG
        cout << " Cache miss" << endl;
        cout << "End get(" << key << "): return -1" << endl;
#endif
        return -1;
    }

    // Element found: retrieve it, update it and return it.
    entryIt& lIt = mIt->second;
    lIt->numHits++;
    lIt->lastHit = _count;
    _update(lIt);

#ifdef DBG
    cout << "End get(" << key << "): return " << lIt->value << endl;
#endif

    return lIt->value;
}

void LFUCache::put(int key, int value) {
#ifdef DBG
    cout << "Start put(" << key << ", " << value << ")" << endl;
#endif
    
    ++_count;

#ifdef DBG
    cout << " _count=" << _count << endl;
#endif

    if (_capacity == 0) {
#ifdef DBG
        cout << " capacity is 0: do nothing" << endl;
        cout << "End put(" << key << ", " << value << ")" << endl;
#endif
        return;
    }

    unordered_map<int, entryIt>::iterator mIt = _cache.find(key);
    if (mIt != _cache.end()) {

#ifdef DBG
        cout << " key in cache" << endl;
#endif

        // Update existing value.
        entryIt& lIt = mIt->second;
        lIt->value = value;
        lIt->numHits++;
        lIt->lastHit = _count;
        _update(lIt);

#ifdef DBG
        cout << " updated entry: key=" << lIt->key << ", ";
        cout << "value=" << lIt->value << ", ";
        cout << "numHits=" << lIt->numHits << ", ";
        cout << "lastHit=" << lIt->lastHit << endl;
#endif

    } else {
        // Insert a new value.

#ifdef DBG
        cout << " key NOT in cache" << endl;
#endif

        if (_cache.size() == _capacity && _capacity > 0) {
            
#ifdef DBG
            cout << " cache at capacity" << endl;
#endif
            
            // We are at max capacity: remove the LFU element before inserting a new value.
            int lfuKey = _l.front().key;
            _cache.erase(lfuKey);
            _l.pop_front();

#ifdef DBG
            cout << " erased entry with key " << lfuKey << " (cache size " << _cache.size() << ")" << endl;
#endif
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

#ifdef DBG
        cout << " inserted new entry in list" << endl;
#endif

        // ...and in the cache.
        _cache.insert(pair<int, entryIt>(key, lIt));

#ifdef DBG
        cout << " inserted new entry in map" << endl;
#endif
    }

#ifdef DBG
    cout << "End put(" << key << ", " << value << ")" << endl;
#endif
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

