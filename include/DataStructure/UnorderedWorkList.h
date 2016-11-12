#pragma once

#include <cassert>
#include <vector>

namespace ds {

template <typename T>
class UnorderedWorkList {
public:
    using ElemType = T;

private:
    using ListType = std::vector<T>;
    ListType currList, nextList;

    typename ListType::size_type currPos;

public:
    UnorderedWorkList() : currPos(0) {}

    bool enqueue(ElemType elem) {
        nextList.push_back(elem);
        return true;
    }

    ElemType dequeue() {
        assert(!empty() && "Cannot dequeue an empty worklist!");

        ++currPos;
        if (currPos > currList.size()) {
            currList.swap(nextList);
            nextList.clear();
            currPos = 1;
        }

        return currList[currPos - 1];
    }

    ElemType front() const {
        assert(!empty() && "Cannot access the front of an empty worklist!");
        if (currPos < currList.size())
            return currList[currPos];
        else
            return nextList.front();
    }

    bool empty() const {
        return (currPos >= currList.size() && nextList.empty());
    }
    std::size_t size() const {
        return currList.size() - currPos + nextList.size();
    }
};
}
