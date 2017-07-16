//
// Copyright (c) 2017, RISUWWV (https://github.com/risuwwv). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef RADIX_SORT_H
#define RADIX_SORT_H

//TODO cover it (all sizes: 8, 16, 24, 32, 58, 64, 70 + complex nested tuples + some testcases with list and const lists and list of tuple<const int>...)
//TODO fuzz it
//TODO static analysis
//TODO optimize:
//  user should be able to pass temporary random access containers (by iterator) instead of always allocting them
//  if the source and dst are of compatible type (no Funct), we should modify them instead of forming (key, value)  (ie key = transformed value, value is implicit and we revert key at the end)
//  transform in place when possible (random access + not const + POD)
//  don't swap tmp/tmp2, ...

//TODO relax constraints on tuple/pair in first phase: Duck typing!

#include <iterator>
#include <algorithm>
#include <tuple>
#include <type_traits>
#include <array>

#include "radixSortDetails.h"

namespace risuwwv
{
    template<typename It, typename OutIt, typename Funct>
    void radix_sort(It begin, It end, OutIt outIt, Funct f)
    {
        if(begin == end)
        {
            return;    
        }
        
        //I see 3 possibilities: 
        //- calling radix_helper(f(...)) every time it is needed
        //- filling a vector of pair<transformed key, object> //what is done below
        //- filling a vector of pair<transformed key, It>   //"It" needs to be at least a forward iterator
     
        using KeyType = decltype(details::flatten_function(details::radix_helper_function(f(*begin))));

        //typename It::value_type does not work because it can have nested const fields
        //typename OutIt::value_type is void for a back_inserter...
        //std::iterator_traits is needed when It is a plain pointer.
        using KeyValuePair = std::pair<KeyType, details::recursive_decay_t<typename std::iterator_traits<It>::value_type>>;
     
        std::vector<KeyValuePair> tmp, tmp2;
        
        using iterator_category = typename std::iterator_traits<It>::iterator_category;
        if constexpr(std::is_same_v<iterator_category, std::random_access_iterator_tag>)
        {
            tmp.reserve(static_cast<size_t>(end-begin)); 
            tmp2.reserve(static_cast<size_t>(end-begin));  
        }
        
        //TODO think about this  
        //alternative is to allocate tmp2 manually in a unique_ptr, unitialized and well aligned, unroll the loop in use_buckets and do placement new on the first call and first loop turn...
        //http://en.cppreference.com/w/cpp/memory/c/aligned_alloc
        //alignof

        constexpr size_t bitPerBucket = 11;
        constexpr int bucketsCount = details::buckets_count<bitPerBucket, KeyType>{}();
        
        //TODO: using a type based on end-begin instead of size_t does not seam to improve performances, try with just two: size_t or uint32_t if <= 2^32 elements
        std::array<size_t, (1<<bitPerBucket)> buckets[bucketsCount]{};//zero init

        //fill buckets
        for(It it = begin; it != end; ++it)
        {
            auto elem = *it;

            //if elem is std::array<const int, N>, we have to decay_value it to std::array<int, N> to assign it to keyValuePair
            KeyValuePair keyValuePair{details::flatten_function(details::radix_helper_function(f(elem))), details::decay_value(elem)};          
              
            details::fill_buckets<bitPerBucket>(buckets, keyValuePair.first);  
                    
            tmp.push_back(keyValuePair);    
            tmp2.push_back(keyValuePair);//TODO we can put default constructible ones if available here 
        }
        
        //buckets[0] is the least significant histogram of the least significant member of the tuple
        
        //sum buckets:
        for(size_t i = 0; i < bucketsCount; ++i)
        { 
		    size_t sum = 0;
		    size_t tsum;
		    for (auto it = buckets[i].begin(); it != buckets[i].end(); ++it) 
		    {
			    tsum = *it + sum;
			    *it = sum;
			    sum = tsum;
		    }         
        }
            
        constexpr size_t size_v = std::tuple_size_v<decltype(tmp[0].first)>;
        auto size = tmp.size();
        auto ptr = details::use_buckets<0, size_v, 0, bitPerBucket>(tmp.data(), tmp2.data(), tmp.size(), buckets);
          
        std::transform(ptr, ptr+size, outIt,
            [](const auto& val)
            {
                return val.second;
            });
    }

    template<typename It, typename Funct>
    void radix_sort(It begin, It end, Funct f)
    {
        radix_sort(begin, end, begin, f);
    }

    template<typename It>
    void radix_sort(It begin, It end)
    {
        radix_sort(begin, end, [](auto x){return x;});
    }

    //expression sfinae is used to remove ambiguity on functor/output iterator.
    //cast to void prevents user defined overload of comma to be problem.

    template<typename Container, typename Funct>
    auto radix_sort(Container& c, Funct f) -> decltype(static_cast<void>(void(f(*std::begin(c)))), void())
    {
        radix_sort(std::begin(c), std::end(c), f);
    }

    template<typename Container, typename OutIt, typename Funct>
    void radix_sort(Container& c, OutIt it, Funct f)
    {
        radix_sort(std::begin(c), std::end(c), it, f);
    }

    template<typename Container, typename OutIt>
    auto radix_sort(Container& c, OutIt it) -> decltype((void)(*(++it) = details::decay_value(*std::begin(c))), void())
    {
        radix_sort(std::begin(c), std::end(c), it, [](auto i){return i;});
    }

    template<typename Container>
    void radix_sort(Container& c)
    {
        radix_sort(std::begin(c), std::end(c));
    }

}//namespace risuwwv

/*
template<typename It, typename OutIt, typename Funct>
void radix_sort(It begin, It end, OutIt it, Funct f);

template<typename It, typename Funct>
void radix_sort(It begin, It end, Funct f);

template<typename It>
void radix_sort(It begin, It end);

template<typename Container, typename Funct>
auto radix_sort(Container& c, Funct f) -> decltype(void(f(*std::begin(c))), void());

//This version takes a accessor functor type. It is used to evaluate each element. This is a bit like a hash function and std::less: the evaluation is used to define the sorted order (example: sort by the ² of input integers)
//TODO make this clearer
template<typename Container, typename OutIt, typename Funct>
void radix_sort(Container& c, OutIt it, Funct f);

//version workind directly on a container. It does NOT need to offer random access. Sorted result will be in OutIt it. It does not need to be a random access iterator. I may overlap with c. Additional memory is used.
//TODO document that c can be modified even if it is set back to original at the end (if this optim is done)
template<typename Container, typename OutIt>
auto radix_sort(Container& c, OutIt it) -> decltype((void)(*(++it) = *std::begin(c)), void());

//version workind directly on a container. It does NOT need to offer random access. The container is modified in place but additional memory is used.
//TODO take an allocator for the needed memory
template<typename Container>
void radix_sort(Container& c);
*/

#endif//RADIX_SORT_H
