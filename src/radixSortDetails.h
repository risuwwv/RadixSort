#ifndef RADIX_SORT_DETAILS_H
#define RADIX_SORT_DETAILS_H

#include <tuple>
#include <array>
#include <cstring>

//on compilater that don't have header detection, ignore this
#if defined(__has_include)
#if __has_include("boost/pfr.hpp")
#include <boost/pfr.hpp>
#endif//defined(__has_include)
#endif//__has_include("boost/pfr.hpp")

#ifdef __GNUC__
namespace std
{
    template<>
    struct is_arithmetic<__uint128_t>: std::true_type
    {};

    template<>
    struct is_arithmetic<__int128_t>: std::true_type
    {};

    template<>
    struct is_integral<__int128_t>: std::true_type
    {};

    template<>
    struct is_integral<__uint128_t>: std::true_type
    {};

    template<>
    struct is_floating_point<__float128>: std::true_type
    {};
}
#endif//__GNUC__

namespace risuwwv
{
    namespace details
    {   
        //decay_value is used for array<const int, N> to array<int, N> and other similar things...
        template<typename T>
        struct decay_value_helper
        {
            constexpr auto operator()(const T& val)
            {
                return val;
            }
        };

        template<typename T>
        auto decay_value(const T& val)
        {
            return decay_value_helper<T>{}(val);
        }

        template<typename T, size_t N>
        struct decay_value_helper<std::array<const T, N>>
        {
            constexpr auto operator()(const std::array<const T, N>& val)
            {
                std::array<T, N> res{};
                std::transform(val.begin(), val.end(), res.begin(), [](const auto& elem){
                    return decay_value(elem);
                });

                return res;
            }            
        };

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

        template<typename T, size_t N>
        struct recursive_decay<const std::array<T, N>>
        {
            using type = recursive_decay_t<std::array<T, N>>;
        };      

        template<typename T, size_t N>
        struct recursive_decay<std::array<const T, N>>
        {
            using type = std::array<recursive_decay_t<T>, N>;
        };   

        template<typename T>
        struct is_array : std::false_type
        {};

        template<typename T, size_t N>
        struct is_array<std::array<T, N>> : std::true_type
        {}; 
      
        template<typename T, size_t N>
        struct is_array<const std::array<T, N>> : std::true_type
        {}; 

        template<typename T>
        inline constexpr bool is_array_v = is_array<T>::value;    

	    //radix_helper is to be used to convert all values to unsigned integers so that radix sort can be done on them
        template<typename T, typename U = void>
        struct radix_helper
        {
            //unknown type => we don't know what to do of it
        };

        template<typename T>
        auto radix_helper_function(const T& val)
        {
            return radix_helper<T>{}(val);
        }

        //TODO val or const& val?

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
		    std::memcpy(&dst, &src, sizeof(SrcType));
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
        
    #ifdef __GNUC__
        template<>
        struct radix_helper<__float128>
        {
            //constexpr 
            __uint128_t operator()(__float128 val)
            {
                auto f = bit_cast<__uint128_t>(val);

                //clang does not like left shift into sign bit
                return f ^ static_cast<__uint128_t>(-__int128_t(f>>127)|static_cast<__int128_t>(__uint128_t(1u) << 127));
            }        
        }; 
    #endif//__GNUC__

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

#if defined(__has_include)
#if __has_include("boost/pfr.hpp")
        //pods that are not numbers nor arrays nor tuples
        template<typename T>
        struct radix_helper<T, std::enable_if_t<!is_tuple_v<T> && !std::is_arithmetic_v<T> && !is_array_v<T> && std::is_pod_v<T>>>
        {
            constexpr auto operator()(T val)
            {
                return radix_helper_function(boost::pfr::flat_structure_to_tuple(val));
            }
        };
#endif//defined(__has_include)
#endif//__has_include("boost/pfr.hpp")

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

        template<typename T>
        auto flatten_function(const T& val)
        {
            return flatten<T>{}(val);
        }
        
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
}//::risuwwv

#endif//RADIX_SORT_DETAILS_H
