//
// Copyright (c) 2017, RISUWWV (https://github.com/risuwwv). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef RADIX_SORT_H
#define RADIX_SORT_H

//TODO optimize:
//  user should be able to pass temporary random access containers (by iterator) instead of always allocting them
//  if the source and dst are of compatible type (no Funct), we should modify them instead of forming (key, value)  (ie key = transformed value, value is implicit and we revert key at the end)
//  transform in place when possible (random access + not const + POD)
//  don't swap tmp/tmp2, ...

//TODO: using a type based on end-begin instead of size_t does not seam to improve performances, try with just two: size_t or uint32_t if <= 2^32 elements (std::array<size_t, (1<<bitPerBucket)>). Did not try with big elements yet

//TODO: in tmp2.push_back(keyValuePair), we can use default constructor if available (nice for expensive to copy items) 

//TODO: add http://www.boost.org/doc/libs/1_64_0/libs/sort/doc/html/index.html to the comparisons
//TODO: pairs of uint64_t to see why uint128 is so slow
//TODO: check with 8bits, 11bits, 14bits and 15bits
//TODO: same perf test with gcc
//TODO: enforce the speedups in a statically sound way (average and variance within bounds) to prevent regressions
//TODO: colors on speedup
//TODO: small container optimization: stack allocation
//TODO: push back to github

//TODO: make compiler=clang sanitize=msan is not happy with unit

/*
DONE:
-use CATCH or another lib for unit tests

TODOs:
-why no (begin, end, outIt) without functor
-f below is not clear, call it keyExtractor or something or radixKey
-static asserts may be separated from runtime tests to make it easier to read
-document that the sort is stable as tested by the stability test
-cover it (all sizes: 8, 16, 24, 32, 58, 64, 70 + complex nested tuples + some testcases with list and const lists and list of tuple<const int>...
-Unit tests should include at least one example of more than 256 elements and more than 65536 ones
-float/double/float128 random values used in the perf test should be from uniform int distrib (current version has upper bits always identical...), probably ignoring NaNs for simplicity
-When possible, should use an implem where only key is used, not key/value as it is smaller. Drawback: need reverse transform at the end, types are differents than now and works only if key extractor is identity or at least reversible.
-find which bucket size works better depending on the input type: 8 to 15 bits radix?
-can prefetching help? Seams complicated as it is generic code
-more unit tests should be generic so they work on list too
-unit test of sorting from a file (written by the test to be clean?) directly and from a input iterator
-is it faster to sort uint128_t or <uint64_t, uint64_t>
-when possible, exploit the input or/and the output as tmp instead of allocating
-when possible, use stack allocation instead of tmp vectors
-simple case should minimize copies: transform input into itself or tmp, do radix sorting passes, last radix + untransform into input or output
-allocator for tmps and/or user passed temporary buffer...
-std::string can be worked on in the following way: radix_sort them by size, shorter first. Than radix sort per last radix only on the subset of maximal length strings, searching bounds linearly,
iterate: second least significant radix on string having it, ... (string that don't have counts as \0\0\0 so have to come first, which they already do because we sorted by length!)
-the problem with std::string is that it needs specific code for every string field in the flatten tuple
-it may be possible to group small fields to exploit bigger radix: tuple<char, char, char, char> needs one pass per char while uint64_t may do better with a bigger radix
-small arrays are probably better sorted by insertion sort like std::sort do, find the bound for various types.
-Msvc compilation
-for large objects, doing all the prefix sums in one pass is probably bad
-sizeof(T) is an upper bound for radix base: 11 bits is useless for uint8_t sort
-sorting <key, it> can be an idea too
-take an allocator for the needed memory
-write proper documentation
-bit_cast can't be constexpr at the moment
-constexpr all that can be
-static analysis
*/

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
        
        std::array<size_t, (1<<bitPerBucket)> buckets[bucketsCount]{};//zero init

        //fill buckets
        for(It it = begin; it != end; ++it)
        {
            auto elem = *it;

            //if elem is std::array<const int, N>, we have to decay_value it to std::array<int, N> to assign it to keyValuePair
            KeyValuePair keyValuePair{details::flatten_function(details::radix_helper_function(f(elem))), details::decay_value(elem)};          
              
            details::fill_buckets<bitPerBucket>(buckets, keyValuePair.first);  
                    
            tmp.push_back(keyValuePair);    
            tmp2.push_back(keyValuePair);
        }
        
        //buckets[0] is the least significant histogram of the least significant member of the tuple
        
        //sum buckets (like future std::exclusive_scan...)
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
    void radix_sort(Container& c, OutIt outIt, Funct f)
    {
        radix_sort(std::begin(c), std::end(c), outIt, f);
    }

    template<typename Container, typename OutIt>
    auto radix_sort(Container& c, OutIt outIt) -> decltype((void)(*(++outIt) = details::decay_value(*std::begin(c))), void())
    {
        radix_sort(std::begin(c), std::end(c), outIt, [](auto i){return i;});
    }

    template<typename Container>
    void radix_sort(Container& c)
    {
        radix_sort(std::begin(c), std::end(c));
    }

}//namespace risuwwv

#endif//RADIX_SORT_H
