//
// Copyright (c) 2017, RISUWWV (https://github.com/risuwwv). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

//recent clang does not need libFuzzer.a, just -fsanitize=fuzzer
//cls && clang++ -Werror -fvisibility=hidden -Wno-endif-labels -Wno-missing-field-initializers -Wno-error=effc++ -Wno-error=inline -Wno-error=aggregate-return -Weverything -fdiagnostics-color=always -Wno-c++98-compat-pedantic -Wno-error=deprecated -Wno-missing-prototypes -Wno-missing-variable-declarations -Wno-error=global-constructors -Wno-padded -fsanitize-coverage=trace-pc-guard,edge -fsanitize=fuzzer,address radixSortFuzz.cpp -o radixSortFuzz -std=c++1z -stdlib=libc++ -Wno-undefined-func-template -Wno-missing-braces libFuzzer.a

//fuzz for 5 minutes, crash if uses more than 8000MB 
//./radixSortFuzz CORPUS_DIR -max_len=100 -max_total_time=300 -rss_limit_mb=8000

//replay only the existing corpus
//./radixSortFuzz CORPUS_DIR -max_len=100 -runs=0 -rss_limit_mb=8000 

#include <fstream>
#include <iostream>
#include <tuple>
#include <vector>
#include <array>

#include "radixSort.h"

using std::tuple;
using namespace risuwwv;

//inheritance because we want to use std::get<>
template<typename... Ts>
struct typelist : public std::tuple<Ts...>
{};

template<typename T>
struct flatten_typelist
{};

template<typename T>
using flatten_typelist_t = typename flatten_typelist<T>::type;

template<typename... Ts1, typename... Ts2, typename... Ts3>
struct flatten_typelist<typelist<typelist<Ts1...>, typelist<Ts2...>, Ts3...>>
{
    using type = flatten_typelist_t<typelist<typelist<Ts1..., Ts2...>, Ts3...>>;
};

template<typename... Ts1>
struct flatten_typelist<typelist<typelist<Ts1...>>>
{
    using type = typelist<Ts1...>;
};

template<typename T, typename U>
struct cartesian_prod_helper
{};

template<typename T, typename... Ts>
struct cartesian_prod_helper<T, typelist<Ts...>>
{
    using type = typelist<tuple<T, Ts>...>;
};

template<typename T, typename U>
struct cartesian_prod
{};

template<typename... Ts, typename T>
struct cartesian_prod<typelist<Ts...>, T>
{
    using type = flatten_typelist_t<typelist<typename cartesian_prod_helper<Ts, T>::type...>>;
};

template<typename T, typename U>
using cartesian_prod_t = typename cartesian_prod<T, U>::type;

template<typename T>
struct flatten_tuple
{};

template<typename T>
using flatten_tuple_t = typename flatten_tuple<T>::type;

template<typename T1, typename T2, typename T3>
struct flatten_tuple<std::tuple<std::tuple<T1, T2>, T3>>
{
    using type = std::tuple<T1, T2, T3>;
};

template<typename... Ts>
struct flatten_tuple<typelist<Ts...>>
{
    using type = typelist<flatten_tuple_t<Ts>...>;
};


template<typename T, typename U, typename... V>
struct union_list
{};

template<typename... Ts>
using union_list_t = typename union_list<Ts...>::type;

template<typename... Ts1, typename... Ts2, typename... Ts3>
struct union_list<typelist<Ts1...>, typelist<Ts2...>, Ts3...>
{
    using type = union_list_t<typelist<Ts1..., Ts2...>, Ts3..., void>;
};

template<typename... Ts1, typename... Ts2>
struct union_list<typelist<Ts1...>, void, Ts2...>
{
    static_assert((std::is_same_v<Ts2, void> && ...));
    using type = typelist<Ts1...>;
};

template<typename T>
struct typelist_size
{};

template<typename... Ts>
struct typelist_size<typelist<Ts...>>
{
    using type = std::tuple_size<tuple<Ts...>>;
};

template<typename... Ts>
inline constexpr std::size_t typelist_size_v = typelist_size<Ts...>::type::value;

template<typename T>
void test(const uint8_t *data, std::size_t size)
{
    constexpr auto Tsize = sizeof(T);
    std::vector<T> v;

    const T* table = reinterpret_cast<const T*>(data);

    for(std::size_t i = 0; (i+1)*Tsize <= size; ++i)
    {
        v.push_back(table[i]);
    }

    auto vc = v;

    radix_sort(v.begin(), v.end());
    std::sort(vc.begin(), vc.end());

    assert((v.size() == vc.size() && std::equal(v.begin(), v.end(), vc.begin())));
}

template<std::size_t Idx>
void helper_function(const uint8_t *data, std::size_t size)
{   
    //quick test of the helper function
    {
        using Cart = cartesian_prod_t<typelist<uint8_t, uint16_t, uint32_t>, typelist<uint8_t, uint16_t, uint32_t>>;
        using Cart2 = typelist<
                        tuple<uint8_t, uint8_t>, 
                        tuple<uint8_t, uint16_t>, 
                        tuple<uint8_t, uint32_t>,
                        tuple<uint16_t, uint8_t>, 
                        tuple<uint16_t, uint16_t>, 
                        tuple<uint16_t, uint32_t>,
                        tuple<uint32_t, uint8_t>, 
                        tuple<uint32_t, uint16_t>, 
                        tuple<uint32_t, uint32_t>>;

        static_assert(std::is_same_v<Cart, Cart2>);
    }
    
    using Ints = typelist<uint8_t/*, uint16_t*//*,uint32_t, uint64_t, int8_t, int16_t, int32_t, int64_t*/, std::array<int8_t, 3>>;  
    using Pairs = cartesian_prod_t<Ints, Ints>;

    using PairInts = cartesian_prod_t<Pairs, Ints>;
    using IntPairs = cartesian_prod_t<Ints, Pairs>;
    using Triples = flatten_tuple_t<PairInts>;

    using Additionals = typelist<
                    tuple<tuple<int64_t, uint8_t>, uint64_t, int32_t>,
                    tuple<tuple<int64_t, uint8_t>, tuple<uint64_t, int32_t>>,
                    tuple<tuple<tuple<int64_t, uint8_t>, tuple<uint64_t, int32_t>>>
                                >;

    using Types = union_list_t<Ints, Pairs, PairInts, IntPairs, Triples, Additionals>;

    //TODO uncomment Ints...
    //currently 1610 types if Ints is complete

    //std::cout << typelist_size_v<Types> << '\n';

    if constexpr(Idx < typelist_size_v<Types>)
    {
        using Type = std::decay_t<decltype(std::get<Idx>(Types{}))>;
        test<Type>(data, size);
    }
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, std::size_t size) 
{
    if(size > 0)
    {
        uint8_t mode = data[0];

        #define CASE(n) case n: \
        helper_function<n>(data, size); \
        break

        #define CASES1(n) \
        CASE(n); \
        CASE(n+1); \
        CASE(n+2); \
        CASE(n+3); \
        CASE(n+4); \
        CASE(n+5); \
        CASE(n+6); \
        CASE(n+7);

        #define CASES2(n) \
        CASES1(n); \
        CASES1(n+8); \
        CASES1(n+16); \
        CASES1(n+24); \
        CASES1(n+32); \
        CASES1(n+40); \
        CASES1(n+48); \
        CASES1(n+56);

        #define CASES3(n) \
        CASES2(n); \
        CASES2(n+64); \
        CASES2(n+128); \
        CASES2(n+192); \
        CASES2(n+256); \
        CASES2(n+320); \
        CASES2(n+384); \
        CASES2(n+448);

        switch(mode & 0b111111)
        {
        CASES3(0);     
        CASES3(512);
        CASES3(1024);
        CASES3(1536);
        CASES3(2048);
        CASES3(2560);
        CASES3(3072);
        default:
            ;//do nothing
        }

        #undef CASE
        #undef CASES1
        #undef CASES2
        #undef CASES3
    }
    return 0;  // Non-zero return values are reserved for future use.
}
