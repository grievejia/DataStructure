#pragma once

#include <cassert>
#include <queue>
#include <unordered_set>

namespace ds {

// This worklist implementation is a simple FIFO queue with an additional set to
// help remove duplicate entries
template <typename T, typename SetType = std::unordered_set<T>>
class FIFOWorkList {
public:
    using ElemType = T;

private:
    // The FIFO queue
    std::queue<ElemType> list;
    // Avoid duplicate entries in FIFO queue
    SetType set;

public:
    FIFOWorkList() {}
    bool enqueue(ElemType elem) {
        if (!set.count(elem)) {
            list.push(elem);
            set.insert(elem);
            return true;
        } else
            return false;
    }

    ElemType dequeue() {
        assert(!list.empty() && "Trying to dequeue an empty queue!");
        ElemType ret = list.front();
        list.pop();
        set.erase(ret);
        return ret;
    }

    ElemType front() const {
        assert(!list.empty() && "Trying to dequeue an empty queue!");
        return list.front();
    }

    bool empty() const { return list.empty(); }
    size_t size() const { return list.size(); }
};
}
