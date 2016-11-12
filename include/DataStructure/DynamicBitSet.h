#pragma once

#include "DataStructure/Detail.h"

#include <algorithm>
#include <limits>
#include <memory>
#include <vector>

namespace ds {

// This is basically a stripped-down version of boost::dynamic_bitset

template <typename Block = unsigned long,
          typename Allocator = std::allocator<Block>>
class DynamicBitSet {
private:
    using BufferType = std::vector<Block, Allocator>;

public:
    using block_type = Block;
    using allocator_type = Allocator;
    using size_type = std::size_t;
    using block_width_type = typename BufferType::size_type;

    static constexpr size_type npos = static_cast<size_type>(-1);
    static constexpr block_width_type bits_per_block =
        std::numeric_limits<Block>::digits;

private:
    BufferType bits;
    size_type numBits;

    static constexpr block_width_type ulong_width =
        std::numeric_limits<unsigned long>::digits;

    size_type calcNumBlocks(size_type numBits) noexcept {
        return numBits / bits_per_block +
               static_cast<size_type>(numBits % bits_per_block != 0);
    }

    static size_type blockIndex(size_type pos) noexcept {
        return pos / bits_per_block;
    }
    static block_width_type bitIndex(size_type pos) noexcept {
        return static_cast<block_width_type>(pos % bits_per_block);
    }
    static Block bitMask(size_type pos) noexcept {
        return Block(1) << bitIndex(pos);
    }
    block_width_type countExtraBits() const noexcept {
        return bitIndex(numBits);
    }
    void zeroUnusedBits() {
        assert(bits.size() == calcNumBlocks(numBits));

        // if != 0 this is the number of bits used in the last block
        auto extraBits = countExtraBits();

        if (extraBits != 0)
            bits.back() &= ~(~static_cast<Block>(0) << extraBits);
    }

    static size_type shiftRight(size_type v, int amount, int width) noexcept {
        return amount >= width ? 0 : (v >>= amount);
    }

    size_type findFrom(size_type firstBlock) const {
        auto i = firstBlock;
        while (i < num_blocks() && bits[i] == 0)
            ++i;
        if (i >= num_blocks())
            return npos;
        else
            return i * bits_per_block +
                   static_cast<size_type>(detail::lowestBit(bits[i]));
    }

public:
    class reference {
        friend class DynamicBitSet<Block, Allocator>;

        // the one and only non-copy ctor
        reference(block_type& b, block_type pos)
            : m_block(b),
              m_mask((assert(pos < bits_per_block), block_type(1) << pos)) {}

        void operator&(); // left undefined

    public:
        // copy constructor: compiler generated

        operator bool() const { return (m_block & m_mask) != 0; }
        bool operator~() const { return (m_block & m_mask) == 0; }

        reference& flip() {
            do_flip();
            return *this;
        }

        reference& operator=(bool x) {
            do_assign(x);
            return *this;
        } // for b[i] = x
        reference& operator=(const reference& rhs) {
            do_assign(rhs);
            return *this;
        } // for b[i] = b[j]

        reference& operator|=(bool x) {
            if (x)
                do_set();
            return *this;
        }
        reference& operator&=(bool x) {
            if (!x)
                do_reset();
            return *this;
        }
        reference& operator^=(bool x) {
            if (x)
                do_flip();
            return *this;
        }
        reference& operator-=(bool x) {
            if (x)
                do_reset();
            return *this;
        }

    private:
        block_type& m_block;
        const block_type m_mask;

        void do_set() { m_block |= m_mask; }
        void do_reset() { m_block &= ~m_mask; }
        void do_flip() { m_block ^= m_mask; }
        void do_assign(bool x) { x ? do_set() : do_reset(); }
    };
    using const_reference = bool;

    explicit DynamicBitSet(const Allocator& alloc = Allocator())
        : bits(alloc), numBits(0) {}

    explicit DynamicBitSet(size_type numBits, unsigned long value = 0,
                           const Allocator& alloc = Allocator())
        : bits(alloc), numBits(numBits) {
        bits.resize(calcNumBlocks(numBits));

        // zero out all bits at pos >= numBits, if any;
        // note that: numBits == 0 implies value == 0
        using ULong = unsigned long;
        if (numBits < static_cast<size_type>(ulong_width)) {
            unsigned long mask = (ULong(1) << numBits) - 1;
            value &= mask;
        }

        for (auto itr = bits.begin(); value; ++itr) {
            *itr = static_cast<block_type>(value);
            value = shiftRight(value, bits_per_block, ulong_width);
        }
    }

    void swap(DynamicBitSet<Block, Allocator>& rhs) {
        std::swap(bits, rhs.bits);
        std::swap(numBits, rhs.numBits);
    }
    allocator_type get_allocator() const { return bits.get_allocator(); }

    size_type num_blocks() const { return bits.size(); }
    size_type size() const { return numBits; }
    bool empty() const { return numBits == 0; }

    void resize(size_type sz, bool value) {
        auto oldNumBlocks = num_blocks();
        auto requiredBlocks = calcNumBlocks(sz);

        block_type v = value ? ~Block(0) : Block(0);

        if (requiredBlocks != oldNumBlocks)
            bits.resize(requiredBlocks, v); // s.g. (copy)

        // At this point:
        //
        //  - if the buffer was shrunk, we have nothing more to do,
        //    except a call to m_zero_unused_bits()
        //
        //  - if it was enlarged, all the (used) bits in the new blocks have
        //    the correct value, but we have not yet touched those bits, if
        //    any, that were 'unused bits' before enlarging: if value == true,
        //    they must be set.
        if (value && (sz > numBits)) {
            auto extraBits = countExtraBits();
            if (extraBits) {
                assert(oldNumBlocks >= 1 && oldNumBlocks <= bits.size());
                bits[oldNumBlocks - 1] |= (v << extraBits);
            }
        }
        numBits = sz;
        zeroUnusedBits();
    }
    void clear() {
        bits.clear();
        numBits = 0;
    }
    void push_back(bool b) {
        auto sz = size();
        resize(sz + 1);
        set(sz, b);
    }
    void pop_back() {
        auto oldNumBlocks = num_blocks();
        auto requiredBlocks = calcNumBlocks(numBits - 1);

        if (requiredBlocks != oldNumBlocks) {
            bits.pop_back();
        }

        --numBits;
        zeroUnusedBits();
    }

    void append(Block value) {
        auto r = countExtraBits();

        if (r == 0) {
            // the buffer is empty, or all blocks are filled
            bits.push_back(value);
        } else {
            bits.push_back(value >> (bits_per_block - r));
            bits[bits.size() - 2] |= (value << r); // bits.size() >= 2
        }

        numBits += bits_per_block;
    }

    DynamicBitSet<Block, Allocator>&
    operator&=(const DynamicBitSet<Block, Allocator>& rhs) {
        assert(size() == rhs.size());
        for (size_type i = 0; i < num_blocks(); ++i)
            bits[i] &= rhs.bits[i];
        return *this;
    }
    DynamicBitSet<Block, Allocator>&
    operator|=(const DynamicBitSet<Block, Allocator>& rhs) {
        assert(size() == rhs.size());
        for (size_type i = 0; i < num_blocks(); ++i)
            bits[i] |= rhs.bits[i];
        return *this;
    }
    DynamicBitSet<Block, Allocator>&
    operator^=(const DynamicBitSet<Block, Allocator>& rhs) {
        assert(size() == rhs.size());
        for (size_type i = 0; i < num_blocks(); ++i)
            bits[i] ^= rhs.bits[i];
        return *this;
    }
    DynamicBitSet<Block, Allocator>&
    operator-=(const DynamicBitSet<Block, Allocator>& rhs) {
        assert(size() == rhs.size());
        for (size_type i = 0; i < num_blocks(); ++i)
            bits[i] &= ~rhs.bits[i];
        return *this;
    }

    DynamicBitSet<Block, Allocator>& reset(size_type pos) {
        bits[blockIndex(pos)] &= ~bitMask(pos);
        return *this;
    }
    DynamicBitSet<Block, Allocator>& set(size_type pos, bool val = true) {
        assert(pos < numBits);

        if (val)
            bits[blockIndex(pos)] |= bitMask(pos);
        else
            reset(pos);

        return *this;
    }
    DynamicBitSet<Block, Allocator>& set() {
        std::fill(bits.begin(), bits.end(), ~Block(0));
        zeroUnusedBits();
        return *this;
    }
    DynamicBitSet<Block, Allocator>& reset() {
        std::fill(bits.begin(), bits.end(), Block(0));
        return *this;
    }
    DynamicBitSet<Block, Allocator>& flip(size_type pos) {
        assert(pos < numBits);
        bits[blockIndex(pos)] ^= bitMask(pos);
        return *this;
    }
    DynamicBitSet<Block, Allocator>& flip() {
        for (auto& elem : bits)
            elem = ~elem;
        zeroUnusedBits();
        return *this;
    }

    bool test(size_type pos) const {
        assert(pos < numBits);
        return (bits[blockIndex(pos)] & bitMask(pos)) != 0;
    }
    bool test_set(size_type pos, bool val) {
        auto b = test(pos);
        if (b != val) {
            set(pos, val);
        }
        return b;
    }
    bool any() const {
        return std::any_of(bits.begin(), bits.end(),
                           [](auto elem) { return elem != 0; });
    }
    bool none() const { return !any(); }
    bool all() const {
        if (empty())
            return true;

        auto extraBits = countExtraBits();
        auto allOnes = ~static_cast<Block>(0);

        if (extraBits == 0) {
            return std::all_of(bits.begin(), bits.end(), [allOnes](auto elem) {
                return elem == allOnes;
            });
        } else {
            for (size_type i = 0, e = num_blocks() - 1; i < e; ++i) {
                if (bits[i] != allOnes) {
                    return false;
                }
            }
            auto mask = ~(~static_cast<Block>(0) << extraBits);
            if (bits.back() != mask) {
                return false;
            }
        }
        return true;
    }

    size_type find_first() const { return findFrom(0); }
    size_type find_next(size_type pos) const {
        size_type sz = size();
        if (pos >= (sz - 1) || sz == 0)
            return npos;

        ++pos;
        auto blk = blockIndex(pos);
        auto idx = bitIndex(pos);
        auto fore = bits[blk] >> idx;
        return fore ? pos + static_cast<size_type>(detail::lowestBit(fore))
                    : findFrom(blk + 1);
    }

    bool operator==(const DynamicBitSet<Block, Allocator>& rhs) const {
        return numBits == rhs.numBits && bits == rhs.bits;
    }
    bool operator!=(const DynamicBitSet<Block, Allocator>& rhs) const {
        return !(*this == rhs);
    }
    bool operator<(const DynamicBitSet<Block, Allocator>& rhs) const {
        assert(size() == rhs.size());

        // Since we are storing the most significant bit
        // at pos == size() - 1, we need to do the comparisons in reverse.
        //
        for (size_type ii = num_blocks(); ii > 0; --ii) {
            size_type i = ii - 1;
            if (bits[i] < rhs.bits[i])
                return true;
            else if (bits[i] > rhs.bits[i])
                return false;
        }
        return false;
    }
    bool operator>(const DynamicBitSet<Block, Allocator>& rhs) const {
        return rhs < *this;
    }
    bool operator>=(const DynamicBitSet<Block, Allocator>& rhs) const {
        return !(*this < rhs);
    }
    bool operator<=(const DynamicBitSet<Block, Allocator>& rhs) const {
        return !(rhs < *this);
    }
};
}
