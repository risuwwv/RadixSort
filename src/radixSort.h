//
// Copyright (c) 2017, RISUWWV (https://github.com/risuwwv). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

//TODO cover it (all sizes: 8, 16, 24, 32, 58, 64, 70 + complex nested tuples + some testcases with list and const lists and list of tuple<const int>...)
//TODO fuzz it
//TODO static analysis
//TODO optimize:
//  user should be able to pass temporary random access containers (by iterator) instead of always allocting them
//  if the source and dst are of compatible type (no Funct), we should modify them instead of forming (key, value)  (ie key = transformed value, value is implicit and we revert key at the end)
//  transform in place when possible (random access + not const + POD)
//  don't swap tmp/tmp2, ...

//TODO relax constraints on tuple/pair in first phase: Duck typing!

#include <vector>
#include <list>
#include <iterator>
#include <algorithm>
#include <tuple>
#include <type_traits>
#include <cstring>
#include <cassert>
#include <iostream>
#include <numeric>
#include <array>

namespace risuwwv
{
    namespace details
    {
        template<typename T>
        struct recursive_decay
        {
            using type = std::decay_t<T>;
        };

        template<typename T>
        using recursive_decay_t = typename recursive_decay<T>::type;

        template<typename... Ts>
        struct recursive_decay<std::tuple<Ts...>>
        {
            using type = std::tuple<recursive_decay_t<Ts>...>;
        };

        template<typename... Ts>
        struct recursive_decay<std::pair<Ts...>>
        {
            using type = std::pair<recursive_decay_t<Ts>...>;
        };

        template<typename... Ts>
        struct recursive_decay<const std::tuple<Ts...>>
        {
            using type = std::tuple<recursive_decay_t<Ts>...>;
        };

        template<typename... Ts>
        struct recursive_decay<const std::pair<Ts...>>
        {
            using type = std::pair<recursive_decay_t<Ts>...>;
        };

	    //radix_helper is to be used to convert all values to unsigned integers so that radix sort can be done on them
        template<typename T, typename U = void>
        struct radix_helper
        {
            //unknown type => we don't know what to do of it
        };

        template<typename T>
        struct radix_helper<T, std::enable_if_t<std::is_unsigned_v<T>>>
        {
            constexpr auto operator()(T val)
            {
                return val;
            }
        };

        template<typename T>
        struct radix_helper<T, std::enable_if_t<!std::is_unsigned_v<T> && std::is_integral_v<T>>>
        {
            constexpr auto operator()(T val)
            {
                using uT = std::make_unsigned_t<T>;
                return uT{static_cast<uT>(static_cast<uT>(val) ^ (static_cast<uT>(1) << (8*sizeof(T) - 1)))};//decltype(short ^ short) == int -_-"
            }
        };    
        
        //TODO: can't be constexpr at the moment
	    template<typename DstType, typename SrcType>
	    //constexpr
	    DstType bit_cast(SrcType src)
	    {
		    static_assert(sizeof(DstType) == sizeof(SrcType), "sizeof(DstType) != sizeof(SrcType)");

		    DstType dst;
		    memcpy(&dst, &src, sizeof(SrcType));
		    return dst;
	    }
        
        template<>
        struct radix_helper<float>
        {
            //inspred by http://stereopsis.com/radix.html
            //constexpr 
            uint32_t operator()(float val)
            {
                auto f = bit_cast<uint32_t>(val);
                return f ^ uint32_t(-int32_t(f>>31)|int32_t(1u << 31));
            }        
        };
        
        template<>
        struct radix_helper<double>
        {
            //constexpr 
            uint64_t operator()(double val)
            {
                auto f = bit_cast<uint64_t>(val);
                return f ^ uint64_t(-int64_t(f>>63)|int64_t(1ul << 63));
            }        
        };  
        
    #if defined(_GLIBCXX_USE_FLOAT128) && (_GLIBCXX_USE_FLOAT128 == 1)
        template<>
        struct radix_helper<__float128>
        {
            //constexpr 
            __uint128_t operator()(__float128 val)
            {
                auto f = bit_cast<__uint128_t>(val);
                return f ^ static_cast<__uint128_t>(-__int128_t(f>>127)|(__int128_t(1u) << 127));
            }        
        }; 
    #endif//_GLIBCXX_USE_FLOAT128 == 1

        template <class T>
        struct is_tuple : std::false_type
        {};
        
        template<typename... Ts>
        struct is_tuple<std::tuple<Ts...>> : std::true_type
        {}; 
      
        template<typename... Ts>
        struct is_tuple<const std::tuple<Ts...>> : std::true_type
        {};
        
        template<typename... Ts>
        struct is_tuple<std::pair<Ts...>> : std::true_type
        {}; 
      
        template<typename... Ts>
        struct is_tuple<const std::pair<Ts...>> : std::true_type
        {};    
        
        template<typename T>
        inline constexpr bool is_tuple_v = is_tuple<T>::value;
        
        template<typename... Ts, size_t... Is>
        constexpr auto radix_variadic_helper(const std::tuple<Ts...>& t, std::index_sequence<Is...>) {
            return std::make_tuple(radix_helper<Ts>{}(std::get<Is>(t))...);   
        }
        
        template<typename... Ts, size_t... Is>
        constexpr auto radix_variadic_helper(const std::pair<Ts...>& t, std::index_sequence<Is...>) {
            return std::make_tuple(radix_helper<Ts>{}(std::get<Is>(t))...);   
        }
        
        //can be made more general so that it works on all classes supporting std::get<> not only std::tuple/std::pair
        template<typename T>
        struct radix_helper<T, std::enable_if_t<is_tuple_v<T>>>
        {
            constexpr auto operator()(T val)
            {
                constexpr size_t size = std::tuple_size_v<T>;
                return radix_variadic_helper(val, std::make_index_sequence<size>{});
            }
        };

        template<typename T, size_t N>
        struct radix_helper<std::array<T, N>>
        {
            constexpr auto operator()(const std::array<T, N>& val)
            {
                //remove inner consts from tuple_cat: we don't want tuple<const int, ...> but tuple<int>
                using TupledType = recursive_decay_t<decltype(tuple_cat(std::tuple<>{}, val))>;

                return radix_helper<TupledType>{}(TupledType{tuple_cat(std::tuple<>{}, val)});
            }
        };
        
        
        template<size_t bitPerBucket, typename T, typename U = void>
        struct buckets_count
        {      
        };
        
        template<size_t bitPerBucket, typename T>
        struct buckets_count<bitPerBucket, T, std::enable_if_t<std::is_arithmetic_v<T>>>
        {
            constexpr size_t operator()()
            {
                return (sizeof(T)*8+bitPerBucket-1)/bitPerBucket;
            }        
        };    
        
        template<size_t bitPerBucket, typename... Ts>
        struct buckets_count<bitPerBucket, std::tuple<Ts...>>
        {
            constexpr size_t operator()()
            {
                return (buckets_count<bitPerBucket, Ts>{}() + ...);
            }        
        };     
        
        template<size_t bitPerBucket, typename... Ts>
        struct buckets_count<bitPerBucket, const std::tuple<Ts...>>
        {
            constexpr size_t operator()()
            {
                return (buckets_count<bitPerBucket, Ts>{}() + ...);
            }        
        };           
        
        template<size_t bitPerBucket, typename T, typename U>
        struct buckets_count<bitPerBucket, std::pair<T, U>>
        {
            constexpr size_t operator()()
            {
                return buckets_count<bitPerBucket, T>{}() + buckets_count<bitPerBucket, U>{}();
            }        
        };
        
        /*template<size_t bitPerBucket, typename T, typename U>
        struct buckets_count<bitPerBucket, const std::pair<T, U>>//TODO is this needed: we use it only on tuple or plain types right? what about tuple of tuple inputs?
        {
            constexpr size_t operator()()
            {
                return buckets_count<bitPerBucket, T>{}() + buckets_count<bitPerBucket, U>{}();
            }        
        };*/  

        template<typename T, typename U>
        struct flatten_helper
        {
        };

        template<typename T>
        struct flatten
        {
            constexpr auto operator()(const T& val)
            {
                return flatten_helper<std::tuple<>, T>{}(std::tuple<>{}, val);
            }
        };
        
        //Ts... = unsigned char, unsigned char, tuple<unsigned char, unsigned char>
        //flatten_helper<std::tuple<>, std::tuple<Ts...> >

        template<typename... Ts, typename T>
        struct flatten_helper<std::tuple<Ts...>, T>
        {     
            constexpr auto operator()(const std::tuple<Ts...>& t, const T& val)
            {
                return std::tuple_cat(t, std::make_tuple(val));
            }        
        };
        
        template<typename... Ts>
        struct flatten_helper<std::tuple<Ts...>, std::tuple<>>
        { 
            constexpr auto operator()(const std::tuple<Ts...>& t, const std::tuple<>&)
            {
                return t;
            }        
        };    
        
        
        template <size_t... Ns, typename... Ts>
        constexpr auto tail_helper(std::index_sequence<Ns...>, const std::tuple<Ts...>& t)
        {
            return std::make_tuple(std::get<Ns+1>(t)...);
            //tuple{get<Ns+1>(t)...}  would fail for Ts... = tuple<tuple<u8, u8>, tuple<u8, u8>> :
            //tuple{tuple<u8, u8>} is of type tuple<u8, u8> not tuple<tuple<u8, u8>>
        }
        
        template<typename... Ts>
        constexpr auto tail(const std::tuple<Ts...>& t)
        {
            return tail_helper(std::make_index_sequence<sizeof...(Ts)-1>(), t);  
        }
        
        template<typename... Ts, typename T, typename... Tail>
        struct flatten_helper<std::tuple<Ts...>, std::tuple<T, Tail...>>
        {
            constexpr auto operator()(const std::tuple<Ts...>& t, const std::tuple<T, Tail...>& src)
            {
                return flatten_helper<std::tuple<Ts..., T>, std::tuple<Tail...>>{}(std::tuple_cat(t, std::make_tuple(std::get<0>(src))), tail(src));
            }        
        };    
        
        template<typename... Ts, typename... Tail>
        struct flatten_helper<std::tuple<Ts...>, std::tuple<std::tuple<Tail...>>>
        {
            constexpr auto operator()(const std::tuple<Ts...>& t, const std::tuple<std::tuple<Tail...>>& src)
            {
                return flatten_helper<std::tuple<Ts...>, std::tuple<Tail...>>{}(t, std::get<0>(src));
            }        
        }; 

        template<typename... Ts, typename... Ts2, typename... Tail>
        struct flatten_helper<std::tuple<Ts...>, std::tuple<std::tuple<Tail...>, Ts2...>>
        {   
            constexpr auto operator()(const std::tuple<Ts...>& t, const std::tuple<std::tuple<Tail...>, Ts2...>& src)
            {
                return flatten_helper<std::tuple<Ts...>, std::tuple<Tail..., Ts2...>>{}(t, std::tuple_cat(std::get<0>(src), tail(src)));
            }        
        };        

        template<typename T>
        constexpr auto get_bits(const T& val, size_t i, size_t bitPerBucket, size_t N)
        {
            return (static_cast<size_t>(val >> (i*bitPerBucket)) & (N-1));
        }

        template<size_t bitPerBucket, size_t Idx, size_t Idx2, size_t N, int M, typename TupleType>
        constexpr void fill_buckets_helper(std::array<size_t, N> (&buckets)[M], const TupleType& kp)
        {
            if constexpr (Idx < std::tuple_size_v<TupleType>)
            {
                const auto& val = std::get<std::tuple_size_v<TupleType>-1-Idx>(kp);//the least significant member of the tuple is the last one...
                
                const size_t upper = (8*sizeof(val)+bitPerBucket-1)/bitPerBucket;
                
                for(size_t i = 0; i < upper; ++i)
                {
                    size_t bits = get_bits(val, i, bitPerBucket, N);
  
                    ++buckets[Idx2+i][bits];
                }
        
                fill_buckets_helper<bitPerBucket, Idx+1, Idx2+upper>(buckets, kp);
            }
        }

        template<size_t bitPerBucket, size_t N, int M, typename TupleType>
        constexpr void fill_buckets(std::array<size_t, N> (&buckets)[M], const TupleType& kp)
        {
            fill_buckets_helper<bitPerBucket, 0, 0>(buckets, kp);
        }  

        template<size_t Idx, size_t size_v, size_t start, size_t bitPerBucket, typename KeyValuePair, typename Array>
        constexpr KeyValuePair* use_buckets(KeyValuePair* tmp, KeyValuePair* tmp2, size_t size, Array& buckets)
        {
            constexpr size_t index = size_v-1-Idx;
            constexpr size_t upper = (8*sizeof(decltype(std::get<index>(tmp[0].first)))+bitPerBucket-1)/bitPerBucket;
            
            for(size_t i = 0; i < upper; ++i)
            { 
                for(size_t j = 0; j < size; ++j)
                {
                    auto& elem = tmp[j];
                    const auto& val = std::get<index>(elem.first);//the least significant member of the tuple is the last one...

                    size_t bits = get_bits(val, i, bitPerBucket, (1<<bitPerBucket));
                    
                    tmp2[buckets[i+start][bits]] = elem;
                    ++buckets[i+start][bits];
                }
                               
                std::swap(tmp, tmp2);     
            }            
            
            if constexpr(Idx+1 < size_v)
            {       
                return use_buckets<Idx+1, size_v, start+upper, bitPerBucket>(tmp, tmp2, size, buckets);
            }
            else
            {
                return tmp;
            }
        }
        	
    }//::details

    template<typename It, typename OutIt, typename Funct>
    void radix_sort(It begin, It end, OutIt outIt, Funct f)
    {
        if(begin == end)
        {
            return;    
        }
        
        //I see 3 possibilities: 
        //- calling radix_helper(f(...)) every time it is needed
        //- filling a vector of pair<transformed key, object>
        //- filling a vector of pair<transformed key, It>
     
        using ReturnedType = decltype(f(*begin));
        using TmpType = decltype(details::radix_helper<ReturnedType>{}(f(*begin)));
        using KeyType = decltype(details::flatten<TmpType>{}(std::declval<TmpType>()));

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
            KeyValuePair keyValuePair{details::flatten<TmpType>{}(details::radix_helper<ReturnedType>{}(f(elem))), elem};          
              
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
    auto radix_sort(Container& c, OutIt it) -> decltype((void)(*(++it) = *std::begin(c)), void())
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
