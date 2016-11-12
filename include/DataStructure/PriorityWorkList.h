#pragma once

#include <cassert>
#include <queue>
#include <unordered_set>

namespace ds {

// This worklist implementation is a priority queue with an additional set to
// help remove duplicate entries
template <typename T, typename Compare = std::less<T>,
          typename SetType = std::unordered_set<T>>
class PriorityWorkList {
public:
    using ElemType = T;

private:
    // The FIFO queue
    std::priority_queue<ElemType, std::vector<ElemType>, Compare> list;
    // Avoid duplicate entries in FIFO queue
    SetType set;

public:
    PriorityWorkList() {}
    PriorityWorkList(const Compare& cmp) : list(cmp) {}
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
        ElemType ret = list.top();
        list.pop();
        set.erase(ret);
        return ret;
    }

    ElemType front() const {
        assert(!list.empty() && "Trying to access front of an empty queue!");
        return list.top();
    }

    bool empty() const { return list.empty(); }
    size_t size() const { return list.size(); }
};
}
