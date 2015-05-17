#ifndef UINT_MULTI_INDEX
#define UINT_MULTI_INDEX

#include <iostream>
#include <array>
#include <cassert>
#include <stdexcept>
#include <initializer_list>

#include "stdarray2stream.hpp"

namespace waveblocks {

template<class UINT, dim_t D>
class TinyMultiIndex
{
public:
    const static std::size_t BITS_PER_ENTRY = (8*sizeof(UINT))/D;
    
    /**
     * for a given axis: returns the largest value that this implementation is able to store
     */
    static int limit(dim_t axis) {
        (void) axis; //supress -Wunused-parameter
        return (UINT(1)<<BITS_PER_ENTRY)-1;
    }
    
    UINT values_;
    
    class Entry
    {
    private:
        UINT &values_;
        
        //care: real index is [index_ - (D-1)]
        std::size_t index_;
        
        inline UINT offset() const
        {
            return BITS_PER_ENTRY*index_;
        }
        
        inline UINT mask() const
        {
            return (UINT(1)<<BITS_PER_ENTRY)-1;
        }
        
        inline int get() const
        {
            return (values_ >> offset()) & mask();
        }
        
        inline void set(int value)
        {
            assert (value >= 0 && UINT(value) <= mask());
            
            values_ &= ~(mask() << offset());
            values_ |= (UINT(value) & mask()) << offset();
        }
        
    public:
        Entry(UINT &values, std::size_t index) : values_(values), index_(index) {}
        
        Entry &operator=(int value)
        {
            set(value);
            return *this;
        }
        
        Entry &operator=(const Entry &entry)
        {
            set(entry.get());
            return *this;
        }
        
        Entry &operator+=(int value)
        {
            value = get() + value;
            set(value);
            return *this;
        }
        
        Entry &operator-=(int value)
        {
            value = get() - value;
            set(value);
            return *this;
        }
        
        Entry &operator*=(int value)
        {
            value = get() * value;
            set(value);
            return *this;
        }
        
        Entry &operator/=(int value)
        {
            value = get() / value;
            set(value);
            return *this;
        }
        
        Entry &operator%=(int value)
        {
            value = get() % value;
            set(value);
            return *this;
        }
        
        operator int() const
        {
            return get();
        }
    };
    
    TinyMultiIndex()
        : values_(0)
    { }
    
    TinyMultiIndex(const TinyMultiIndex &that)
        : values_(that.values_)
    { }
    
    TinyMultiIndex(const std::array<int,D> &that)
    { 
        for (dim_t d = 0; d < D; d++)
            operator[](d) = that[d];
    }
    
    TinyMultiIndex(std::initializer_list<int> list)
    {
        dim_t axis = 0;
        for (typename std::initializer_list<int>::iterator it = list.begin(); it != list.end() && axis < D; it++) {
            if (*it > limit(axis))
                throw std::range_error("this multi-index implementation is unable to store a large value than " + std::to_string(limit(axis)));
            
            operator[](axis++) = *it;
        }
    }
    
    TinyMultiIndex &operator=(const TinyMultiIndex &that)
    {
        values_ = that.values_;
        
        return *this;
    }
    
    int operator[](dim_t index) const
    {
        return (values_ >> BITS_PER_ENTRY*((D-1)-index)) & ( (UINT(1)<<BITS_PER_ENTRY)-1 );
    }
    
    Entry operator[](dim_t index)
    {
        return Entry(values_, (D-1)-index);
    }
    
    bool operator==(const TinyMultiIndex &that) const
    {
        return values_ == that.values_;
    }
    
    bool operator!=(const TinyMultiIndex &that) const
    {
        return values_ != that.values_;
    }
    
    operator std::array<int,D>() const
    {
        std::array<int,D> copy;
        for (dim_t d = 0; d < D; d++) {
            copy[d] = operator[](d);
        }
        return copy;
    }
};

template<class UINT, dim_t D>
std::ostream &operator<<(std::ostream &out, const TinyMultiIndex<UINT, D> &index)
{
    std::cout << "(";
    for (dim_t i = 0; i < D-1; i++)
        std::cout << index[i] << ", ";
    if (D != 0)
        std::cout << index[D-1];
    std::cout << ")";
    return out;
}

}

namespace std {
    /**
     * provides less functor (compare) for STL (notable std::map)
     * specializes generic std::less<T>
     */
    template<class UINT, waveblocks::dim_t D>
    struct less< waveblocks::TinyMultiIndex<UINT,D> >
    {
    private:
        typedef waveblocks::TinyMultiIndex<UINT,D> MultiIndex;
        
    public:
        typedef MultiIndex first_argument_type;
        typedef MultiIndex second_argument_type;
        typedef bool result_type;
        
        bool operator()(const MultiIndex &first, const MultiIndex &second) const
        {
            return first.values_ < second.values_;
        }
    };
    
    /**
     * provides hash functor for STL (notable std::unordered_map)
     * specializes generic std::hash<T>
     */
    template<class UINT, waveblocks::dim_t D>
    struct hash< waveblocks::TinyMultiIndex<UINT,D> >
    {
    private:
        typedef waveblocks::TinyMultiIndex<UINT,D> MultiIndex;
        
    public:
        std::size_t operator()(const MultiIndex &index) const
        {
            return index.values_;
        }
    };
    
    /**
     * provides equality functor for STL (notable std::unordered_map)
     * specializes generic std::equal_to<T>
     */
    template<class UINT, waveblocks::dim_t D>
    struct equal_to< waveblocks::TinyMultiIndex<UINT,D> >
    {
    private:
        typedef waveblocks::TinyMultiIndex<UINT,D> MultiIndex;
        
    public:
        typedef MultiIndex first_argument_type;
        typedef MultiIndex second_argument_type;
        typedef bool result_type;
        
        bool operator()(const MultiIndex &first, const MultiIndex &second) const
        {
            return first.values_ == second.values_;
        }
    };
}

#endif