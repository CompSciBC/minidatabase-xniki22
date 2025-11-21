#ifndef ENGINE_H
#define ENGINE_H

#include <iostream>   
#include <vector> 
#include <algorithm>
#include "BST.h"      
#include "Record.h"
//add header files as needed

using namespace std;

// Converts a string to lowercase (used for case-insensitive searches)
static inline string toLower(string s) {
    for (char &c : s) c = (char)tolower((unsigned char)c);
    return s;
}

// ================== Index Engine ==================
// Acts like a small "database engine" that manages records and two BST indexes:
// 1) idIndex: maps student_id → record index (unique key)
// 2) lastIndex: maps lowercase(last_name) → list of record indices (non-unique key)
struct Engine {
    vector<Record> heap;                  // the main data store (simulates a heap file)
    BST<int, int> idIndex;                // index by student ID
    BST<string, vector<int>> lastIndex;   // index by last name (can have duplicates)

    // Inserts a new record and updates both indexes.
    // Returns the record ID (RID) in the heap.
    int insertRecord(const Record &recIn) {
         //TODO
        Record rec = recIn;
        int rid = heap.size();       // RID = index in heap
        heap.push_back(rec);         // append to data store

        // update idIndex
        idIndex.insert(rec.id, rid);

        // update lastIndex (convert to lowercase for case-insensitive search)
        string lname = toLower(rec.last);
        vector<int> *vecPtr = lastIndex.find(lname);
        if (vecPtr)
            vecPtr->push_back(rid);
        else
            lastIndex.insert(lname, {rid});

        return rid;
    
       
    }

    // Deletes a record logically (marks as deleted and updates indexes)
    // Returns true if deletion succeeded.
    bool deleteById(int id) {
        //TODO
         idIndex.resetMetrics();
        int *ridPtr = idIndex.find(id);
        if (!ridPtr)
            return false;  // record not found

        int rid = *ridPtr;
        Record &rec = heap[rid];
        if (rec.deleted)
            return false;  // already deleted

        rec.deleted = true;  // soft delete

        // remove from idIndex
        idIndex.erase(id);

        // update lastIndex (remove this rid from list)
        string lname = toLower(rec.last);
        vector<int> *vecPtr = lastIndex.find(lname);
        if (vecPtr) {
            vecPtr->erase(remove(vecPtr->begin(), vecPtr->end(), rid), vecPtr->end());
            if (vecPtr->empty())
                lastIndex.erase(lname);  // clean up empty lists
        }

        return true;
    }

    // Finds a record by student ID.
    // Returns a pointer to the record, or nullptr if not found.
    // Outputs the number of comparisons made in the search.
    const Record *findById(int id, int &cmpOut) {
        //TODO    
         idIndex.resetMetrics();
        int *ridPtr = idIndex.find(id);
        cmpOut = idIndex.comparisons;

        if (!ridPtr)
            return nullptr;

        const Record &rec = heap[*ridPtr];
        return rec.deleted ? nullptr : &rec;
    }

    // Returns all records with ID in the range [lo, hi].
    // Also reports the number of key comparisons performed.
    vector<const Record *> rangeById(int lo, int hi, int &cmpOut) {
        //TODO
        vector<const Record *> result;
        idIndex.resetMetrics();

        idIndex.rangeApply(lo, hi, [&](int key, int rid) {
            const Record &rec = heap[rid];
            if (!rec.deleted)
                result.push_back(&rec);
        });

        cmpOut = idIndex.comparisons;
        return result;
        
    }

    // Returns all records whose last name begins with a given prefix.
    // Case-insensitive using lowercase comparison.
    vector<const Record *> prefixByLast(const string &prefix, int &cmpOut) {
        //TODO
         vector<const Record *> result;
        lastIndex.resetMetrics();

        string low = toLower(prefix);
        string high = low;
        high.back()++;  // next possible string (simple upper bound)

        lastIndex.rangeApply(low, high, [&](const string &lname, vector<int> &rids) {
            if (lname.rfind(low, 0) == 0) {  // prefix match
                for (int rid : rids) {
                    const Record &rec = heap[rid];
                    if (!rec.deleted)
                        result.push_back(&rec);
                }
            }
        });

        cmpOut = lastIndex.comparisons;
        return result;

    }
};

#endif
