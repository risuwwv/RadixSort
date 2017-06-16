//
// Copyright (c) 2017, RISUWWV (https://github.com/risuwwv). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

//this is only a draft, there is no implementation, just interfaces

#include <vector>
#include <list>
#include <iterator>
#include <algorithm>
#include <tuple>
#include <type_traits>

namespace details
{
    //radix_helper is to be used to convert all values to unsigned integers so that radix sort can be done on them
	
    template<typename T, typename U = void>
    struct radix_helper
    {
        //unknown type
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
            return uT{static_cast<uT>(val ^ (1 << (8*sizeof(T) - 1)))};
        }
    };    
    
    template<>
    struct radix_helper<float>
    {
        constexpr uint32_t operator()(float val)
        {
            return val;//TODO
        }        
    };
    
    template<>
    struct radix_helper<double>
    {
        constexpr uint64_t operator()(double val)
        {
            return val;//TODO
        }        
    };  
    
#if _GLIBCXX_USE_FLOAT128 == 1
    template<>
    struct radix_helper<__float128>
    {
        constexpr __uint128_t operator()(__float128 val)
        {
            return val;//TODO
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
    
    template<typename T>
    inline constexpr bool is_tuple_v = is_tuple<T>::value;
    
    template<typename... Ts, std::size_t... Is>
    constexpr auto radix_variadic_helper(const std::tuple<Ts...>& t, std::index_sequence<Is...>) {
        return std::tuple{radix_helper<Ts>{}(std::get<Is>(t))...};   
    }
    
    template<typename... Ts>
    constexpr auto radix_helper_function(const std::tuple<Ts...>& t)
    {
        constexpr std::size_t size = std::tuple_size_v<std::tuple<Ts...>>;
        return radix_variadic_helper(t, std::make_index_sequence<size>{});
    }
    
    //TODO make this more general so that it works on all classes supporting std::get<> not only std::tuple
    template<typename T>
    struct radix_helper<T, std::enable_if_t<is_tuple_v<T>>>
    {
        constexpr auto operator()(T val)
        {
            return radix_helper_function(val);
        }
    };   
}

template<typename It, typename Funct>
void radix_sort(It begin, It end, Funct f)
{
    //TODO temporay
    std::vector<typename It::value_type> tmp;
    std::copy(begin, end, std::back_inserter(tmp));

    std::sort(std::begin(tmp), std::end(tmp), 
        [&f](const auto& lhs, const auto& rhs)
        {
            return f(lhs) < f(rhs);
        });

    std::copy(std::begin(tmp), std::end(tmp), begin);
}

template<typename It, typename OutIt, typename Funct>
void radix_sort(It begin, It end, OutIt it, Funct f)
{
    //TODO temporay
    std::vector<typename It::value_type> tmp;
    std::copy(begin, end, std::back_inserter(tmp));

    std::sort(std::begin(tmp), std::end(tmp), 
        [&f](const auto& lhs, const auto& rhs)
        {
            return f(lhs) < f(rhs);
        });

    std::copy(std::begin(tmp), std::end(tmp), it);
}

template<typename It>
void radix_sort(It begin, It end)
{
    radix_sort(begin, end, [](auto x){return x;});
}

//expression sfinae is used to remove ambiguity on functor/output iterator.
//cast to void prevents user defined overload of comma to be problem.

template<typename Container, typename Funct>
auto radix_sort(Container& c, Funct f) -> decltype(void(f(*std::begin(c))), void())
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

int main()
{
    std::vector<int> v{1, 4, 2, 5, 7, 11, 4, 33};
    radix_sort(v.begin(), v.end());
    
    //TODO check results

    radix_sort(v, 
        [](int i)
        {
            return (i-5)*(i-5);
        });

    //TDO check resuts

    std::vector<float> v2{1.f, 4.f, 2.f, 5.f, 7.f, 11.f, 4.f, 33.f};
    radix_sort(v2);    

    //TODO check results

    struct Test{
        int key_;
        std::string val_;
    };

    std::list<Test> l{{7, "a"}, {1, "b"}, {3, "c"}, {2, "ff"}};

    radix_sort(l, [](const Test& t){return t.val_;});

    //TODO check results

    std::list<int> l2;
    std::vector<int> v4{1, -4, 2, 5, 7, 11, 4, -33};

    radix_sort(v4, std::back_inserter(l2));

    //TODO check results

	//class template argument deduction!
    std::vector v5{std::tuple(10, 2.0), std::tuple(3, 5.0), std::tuple(3, 2.0), std::tuple(11, 3.0)};

    radix_sort(v5);

    //TODO check results

    constexpr auto value = details::radix_helper<unsigned int>{}(5);
    static_assert(value == 5);

    constexpr auto value2 = details::radix_helper<int>{}(5);

    static_assert(value2 == ((1u<<31)|5u));

    constexpr auto value3 = details::radix_helper<std::tuple<int, double>>{}(std::tuple{1, 7.0});
    
    static_assert(std::is_same_v<decltype(value3), const std::tuple<unsigned int, uint64_t>>);

    return 0;
}