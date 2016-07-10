/*
 * Copyright (C) 2016 damu/gawag
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
*/

#ifndef STK_MEMORY_PLAIN_H
#define STK_MEMORY_PLAIN_H

/// \file
/// \brief This file contains various classes to manage plain memory.

#include <stdlib.h>
#include <string.h>
#include <new>

namespace stk
{

/// \brief Wrapps malloc and free in a RAII fashion. It allocates count*sizeof(T).
template<typename T=char>
struct memory_plain
{
    T* ptr=0;
    size_t count_=0;

    memory_plain(size_t count=0)
    {
        if(count>=0)
        {
            ptr=(T*)malloc(count*sizeof(T));
            count_=count;
        }
    }
    memory_plain(const memory_plain& o) : memory_plain(o.size())
    {
        memcpy((void*)data(),(void*)o.data(),o.size()*sizeof(T));
    }
    memory_plain& operator=(const memory_plain& o)
    {
        *this=memory_plain(o.size());
        memcpy((void*)data(),(void*)o.data(),o.size()*sizeof(T));
        return *this;
    }

    memory_plain(memory_plain&& o)
    {
        ptr=o.ptr;
        o.ptr=0;
        count_=o.count_;
        o.count_=0;
    }
    memory_plain& operator=(memory_plain&& o)
    {
        ptr=o.ptr;
        o.ptr=0;
        count_=o.count_;
        o.count_=0;
        return *this;
    }

    ~memory_plain()
    {
        if(ptr)
        {
            free(ptr);
            ptr=0;
            count_=0;
        }
    }

    T& operator[](size_t i){return ptr[i];}
    const T& operator[](size_t i) const {return ptr[i];}
    size_t size() const {return count_;}
    T* data() const {return ptr;}
    T* begin() const {return ptr;}
    T* end() const {return ptr+count_;}
    void fill(uint8_t v)
    {
        memset(data(),v,size()*sizeof(T));
    }
};

/// \brief This class creates "aligned memory" by using malloc with additional space. There should be an aligned_alloc with C11 but that wasn't available on my MinGW 4.8.2.
template<typename T=unsigned char,int alignment=16>
struct memory_aligned
{
    void* ptr_;
    T* data_;
    size_t size_;

    memory_aligned() : data_(0),size_(0) {}
    memory_aligned(size_t size) : size_(size)
    {
        ptr_=(T*)malloc(size*sizeof(T)+alignment);
        if(!ptr_)
            throw std::bad_alloc();
        data_=(T*)((((size_t)ptr_)/alignment+1)*alignment);
    }
    memory_aligned(const memory_aligned&)=delete;
    memory_aligned& operator=(const memory_aligned&)=delete;
    memory_aligned(memory_aligned&& o)
    {
        ptr_=o.ptr_;
        data_=o.data_;
        size_=o.size_;
        o.ptr_=0;
        o.data_=0;
        o.size_=0;
    }
    memory_aligned& operator=(memory_aligned&& o)
    {
        data_=o.data_;
        size_=o.size_;
        o.data_=0;
        o.size_=0;
    }
    ~memory_aligned()
    {
        if(ptr_)
            free(ptr_);
    }

    size_t size() const {return size_;}
    T* data() const {return data_;}
    T* begin() const {return data_;}
    T* end() const {return data_+size_;}
    T& operator[](size_t index) {return *(data_+index);}
    const T& operator[](size_t index) const {return *(data_+index);}
};

/// \brief Similiar to std::vector but does not initialize it's elements.
template<typename T,typename Allocator=memory_plain<T>>
struct vector_plain
{
    Allocator data_;
    size_t size_;

    vector_plain(size_t size) : data_(size),size_(size) {}
    vector_plain() : size_(0) {}

    T* data() const {return data_.begin();}
    T* begin() const {return data_.begin();}
    T* end() const {return data_.begin()+size_;}
    size_t capacity() const {return data_.size();}
    size_t size() const {return size_;}
    T& operator[](size_t index) {return *(data()+index);}
    const T& operator[](size_t index) const {return data_[index];}

    void resize(size_t new_size)
    {
        if(new_size>data_.size())
        {
            Allocator tmp(data_.size()>=10?data_.size()*2:10);
            memcpy(tmp.data(),data_.data(),data_.size()*sizeof(T));
            data_=std::move(tmp);
        }
        size_=new_size;
    }

    void push_back(const T& v)
    {
        auto s=size();
        resize(s+1);
        operator[](s)=v;
    }
    void push_back(T&& v)
    {
        auto s=size();
        resize(s+1);
        operator[](s)=v;
    }
    void emplace_back(const T& v){push_back(v);}
    void emplace_back(T&& v){push_back(v);}
};

}   // namespace stk

#endif // STK_MEMORY_PLAIN_H
