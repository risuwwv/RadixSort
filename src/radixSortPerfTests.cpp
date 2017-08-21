//
// Copyright (c) 2017, RISUWWV (https://github.com/risuwwv). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

//cls && make perf mode=release compiler=gcc
//cls && make perf mode=release compiler=clang

#include <vector>
#include <random>
#include <functional>
#include <cassert>
#include <algorithm>
#include <iterator>
#include <limits>
#include <chrono>
#include <iostream>
#include <cstdint>
#include <numeric>
#include <map>
#include <list>

//not standard
#include <cxxabi.h>

#include "radixSort.h"

using namespace risuwwv;

//see https://stackoverflow.com/questions/2616906/how-do-i-output-coloured-text-to-a-linux-terminal
//for color commands

namespace color
{
    struct RedColor
    {};

    struct GreenColor
    {};

    struct YellowColor
    {};

    struct ResetColor
    {};

    std::ostream& operator<<(std::ostream& stream, const RedColor&)
    {
        return stream << "\033[1;31m";
    }

    std::ostream& operator<<(std::ostream& stream, const GreenColor&)
    {
        return stream << "\033[1;32m";
    }

    std::ostream& operator<<(std::ostream& stream, const YellowColor&)
    {
        return stream << "\033[1;33m";
    }

    std::ostream& operator<<(std::ostream& stream, const ResetColor&)
    {
        return stream << "\033[0m";
    }

    inline constexpr RedColor red;
    inline constexpr GreenColor green;
    inline constexpr YellowColor yellow;
    inline constexpr ResetColor reset;
}//namespace color

//probably not necessary to wrap it twice but I don't want inlining to mess with the measure

template<typename Container>
void __attribute__((noinline)) radix_sort_caller(Container& v)
{
    radix_sort(v.begin(), v.end());
}

template<typename T>
void __attribute__((noinline)) std_sort_caller(std::vector<T>& v)
{
    std::sort(v.begin(), v.end());
}

template<typename T>
void __attribute__((noinline)) std_sort_caller(std::list<T>& v)
{
    v.sort();
}

template<typename T>
void time_format(std::ostream& stream, T duration)
{
    double d = static_cast<double>(duration);
    if(d > 1000)
    {
        d /= 1000;
    }
    else
    {
        stream << d << " ns";
        return;
    }

    if(d > 1000)
    {
        d /= 1000;
    }
    else
    {
        stream << d << " \xC2\xB5s";
        return;
    }    

    if(d > 1000)
    {
        d /= 1000;
    }
    else
    {
        stream << d << " ms";
        return;
    } 

    stream << d << " s"; 
}

template<typename T>
struct getTypeNameHelper
{
    std::string operator()() const
    {
        return abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
    }
};

//default display for containers is verbose and ugly
template<typename T, typename U>
struct getTypeNameHelper<std::vector<T, U>>
{
    std::string operator()() const
    {
        return std::string("vector<") + getTypeNameHelper<T>{}() + ">";
    }
};

template<typename T, typename U>
struct getTypeNameHelper<std::list<T, U>>
{
    std::string operator()() const
    {
        return std::string("list<") + getTypeNameHelper<T>{}() + ">";
    }
};

template<typename T>
std::string getTypeName()
{
    return getTypeNameHelper<T>{}();
}

template<typename Container>
auto __attribute__((noinline)) radix_sort_caller2(Container& v)
{
    auto begin = std::chrono::high_resolution_clock::now();
    radix_sort_caller(v);
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end-begin).count();

    std::cout << "radix_sort(" << getTypeName<Container>() << "[" << v.size() << "]) takes ";
    time_format(std::cout, duration); 
    std::cout << '\n';

    return duration;
}

template<typename Container>
auto __attribute__((noinline)) std_sort_caller2(Container& v)
{
    auto begin = std::chrono::high_resolution_clock::now();
    std_sort_caller(v);
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end-begin).count();

    std::cout << "std::sort(" << getTypeName<Container>() << "[" << v.size() << "]) takes ";
    time_format(std::cout, duration);
    std::cout << '\n';

    return duration;
}

template<typename Container>
auto __attribute__((noinline)) test(Container& v, Container& vc)
{
    auto d1 = radix_sort_caller2(v);
    auto d2 = std_sort_caller2(vc);
    std::cout << '\n';

    return std::pair{d1,d2};
}

template<typename Container>
struct initHelper
{
    void operator()(Container& v, size_t size)
    {
        using T = typename Container::value_type;
        std::random_device rnd_device;
        std::mt19937 mersenne_engine(rnd_device());
        std::uniform_int_distribution<T> dist(std::numeric_limits<T>::min()/2, std::numeric_limits<T>::max()/2);//ubsan dislikes it with min() and max() as it overflows in the Mersenne engine
        auto gen = std::bind(dist, mersenne_engine);

        generate_n(std::back_inserter(v), size, gen);
    }
};

template<>
struct initHelper<std::vector<float>>
{
    void operator()(std::vector<float>& v, size_t size)
    {
        std::random_device rnd_device;
        std::mt19937 mersenne_engine(rnd_device());

        //std::numeric_limits<float>::min() is positive for float
        //std::uniform_real_distribution<float> dist(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max()); // does not work at all: returns only 'inf'...
        std::uniform_real_distribution<float> dist(-1e20f, 1e20f);

        auto gen = std::bind(dist, mersenne_engine);

        generate_n(std::back_inserter(v), size, gen);
    }
};

template<>
struct initHelper<std::vector<double>>
{
    void operator()(std::vector<double>& v, size_t size)
    {
        std::random_device rnd_device;
        std::mt19937 mersenne_engine(rnd_device());
        std::uniform_real_distribution<double> dist(-1e20, 1e20);
        auto gen = std::bind(dist, mersenne_engine);

        generate_n(std::back_inserter(v), size, gen);
    }
};

#if __GNUC__
template<>
struct initHelper<std::vector<__float128>>
{
    void operator()(std::vector<__float128>& v, size_t size)
    {
        std::random_device rnd_device;
        std::mt19937 mersenne_engine(rnd_device());
        std::uniform_real_distribution<double> dist(-1e20, 1e20);//does not work with __float128
        auto gen = std::bind(dist, mersenne_engine);

        generate_n(std::back_inserter(v), size, gen);
    }
};
#endif//__GNUC__

template<typename Container>
void init(Container& v, size_t size)
{
    initHelper<Container>{}(v, size);
}

template<typename T, template <typename, typename> class Container>
auto testcase(size_t size)
{
    Container<T, std::allocator<T>> v;

    init(v, size);
   
    auto vc = v;

    auto timePair = test(v, vc);

    assert((v.size() == vc.size() && std::equal(v.begin(), v.end(), vc.begin())));

    //std::copy(vc.begin(), vc.end(), std::ostream_iterator<T>(std::cout, " "));
    //std::cout << '\n';

    return timePair;
}

//TODO put this in a separate file
template<typename It>
auto mean(It begIt, It endIt)
{
    using T = typename It::value_type; 

    auto sum = std::accumulate(begIt, endIt, T{0});
    return sum/static_cast<typename It::value_type>(std::distance(begIt, endIt));
}

template<typename It>
auto meanVariance(It begIt, It endIt)
{
    auto m = mean(begIt, endIt);

    using T = typename It::value_type; 

    T var = 0;
    for(auto it = begIt; it != endIt; ++it)
    {
        var += (*it-m)*(*it-m);
    }
    
    auto dist = std::distance(begIt, endIt);
    return std::pair<T, T>{m, var/std::fmax(T{1}, dist-1)};
}

template<typename It>
auto variance(It begIt, It endIt)
{
    return meanVariance(begIt, endIt).second;
}

auto speedup(const std::vector<std::pair<size_t, size_t>>& vect)
{
    std::vector<double> speedups;
    speedups.reserve(vect.size());

    std::transform(vect.begin(), vect.end(), std::back_inserter(speedups), [](const auto& p){
        return static_cast<double>(p.second)/static_cast<double>(p.first);
    });

    auto [m, v] = meanVariance(speedups.begin(), speedups.end());
    std::cout << "speedup: " << m << " (variance: " << v << ")" << '\n';

    return std::pair<double, double>{m,v};
}

template<typename T, template <typename, typename> class Container>
auto testcases(const std::vector<size_t>& sizes)
{
    constexpr size_t samples = 10;//10;

    auto length = sizes.size();    
    std::vector<std::vector<std::pair<uint64_t, uint64_t>>> p(length);

    for(size_t i = 0; i < samples; ++i)
    {
        for(size_t j = 0; j < length; ++j) 
        {
            p[j].push_back(testcase<T, Container>(sizes[j]));
        }
    }

    std::vector<std::pair<double, double>> res;

    for(size_t j = 0; j < length; ++j) 
    {
        res.push_back(speedup(p[j]));
    }

    std::cout << '\n';

    return res;
}

using Pair = std::pair<double, double>;
using Vect = std::vector<std::pair<double, double>>;

template<typename T, template <typename, typename> class Container = std::vector>
void helper(const std::vector<size_t>& sizes, std::map<std::string, Vect>& results)
{
    results[getTypeName<Container<T,std::allocator<T>>>()] = testcases<T, Container>(sizes);
}

//only _upperBytes really participates in the sort but my radix sort is still sorting using LSB first so it super slow.
//using int64_t makes it so slow that it is boring to wait for the test to finish...
struct SpecialType
{
    SpecialType(int32_t upperBytes) : _upperBytes{upperBytes}, _lowerBytes1{2100010101}, _lowerBytes2{-1114543822}, _lowerBytes3{1176943823}
    {}

    bool operator<(const SpecialType& other) const
    {
        if(_upperBytes == other._upperBytes)
        {
            if(_lowerBytes1 == other._lowerBytes1)
            {
                if(_lowerBytes2 == other._lowerBytes2)
                {
                    if(_lowerBytes3 == other._lowerBytes3)
                    {
                        return false;
                    }
                    return _lowerBytes3 < other._lowerBytes3;
                }
                return _lowerBytes2 < other._lowerBytes2;
            }
            return _lowerBytes1 < other._lowerBytes1;
        }
        return _upperBytes < other._upperBytes;
    }

    int32_t _upperBytes;
    int32_t _lowerBytes1;
    int32_t _lowerBytes2;
    int32_t _lowerBytes3;
};

struct KeyValue
{
    uint64_t _key;
    uint64_t _val;

    bool operator<(const KeyValue& other) const
    {
        return _key < other._key;
    }

    bool operator==(const KeyValue& other) const
    {
        return _key == other._key;
    }
};

namespace risuwwv
{
    namespace details
    {  
        template<>
        struct radix_helper<SpecialType>
        {
            auto operator()(const SpecialType& val)
            {
                return std::make_tuple(val._upperBytes, val._lowerBytes1, val._lowerBytes2, val._lowerBytes3);
            }
        };

        template<>
        struct radix_helper<KeyValue>
        {
            auto operator()(const KeyValue& val)
            {
                return val._key;
            }
        };
    }//namespace details
}//namespace risuwwv

template<>
void init(std::vector<SpecialType>& v, size_t size)
{
    std::random_device rnd_device;
    std::mt19937 mersenne_engine(rnd_device());
    std::uniform_int_distribution<int32_t> dist(std::numeric_limits<int32_t>::min()/2, std::numeric_limits<int32_t>::max()/2);//ubsan dislikes it with min() and max() as it overflows in the Mersenne engine
    auto gen = [&](){
        return SpecialType{dist(mersenne_engine)};
    };

    generate_n(std::back_inserter(v), size, gen);
}

template<>
void init(std::vector<KeyValue>& v, size_t size)
{
    std::random_device rnd_device;
    std::mt19937 mersenne_engine(rnd_device());
    std::uniform_int_distribution<uint64_t> dist(std::numeric_limits<uint64_t>::min()/2, std::numeric_limits<uint64_t>::max()/2);//ubsan dislikes it with min() and max() as it overflows in the Mersenne engine
    auto gen = [&](){
        auto v1 = dist(mersenne_engine);
        auto v2 = dist(mersenne_engine);
        return KeyValue{v1, v2};
    };

    generate_n(std::back_inserter(v), size, gen);
}

int main()
{
    std::map<std::string, Vect> targetMeansVariances;
        
    targetMeansVariances[getTypeName<std::vector<int8_t>>()] = 
        Vect{Pair{0.136619, 0.00105332}, Pair{5.40873, 1.6909}, Pair{3.19948, 0.0233439}, Pair{2.8654, 0.00291782}, Pair{2.84225, 0.0048124}};

    targetMeansVariances[getTypeName<std::vector<int16_t>>()] = 
        Vect{Pair{0.0870894, 0.00417521}, Pair{3.74942, 0.930056}, Pair{3.47109, 0.00988753}, Pair{3.47863, 0.00150519}, Pair{3.41936, 0.000793076}};

    targetMeansVariances[getTypeName<std::vector<int32_t>>()] = 
        Vect{Pair{0.0587488, 0.00477154}, Pair{3.16355, 0.340866}, Pair{2.25233, 0.000815486}, Pair{2.82961, 0.000597585}, Pair{3.25518, 0.000954513}};

    targetMeansVariances[getTypeName<std::vector<int64_t>>()] = 
        Vect{Pair{0.0327907, 0.000718227}, Pair{1.94566, 0.442034}, Pair{1.11678, 0.00143563}, Pair{1.29264, 0.000791579}, Pair{1.48278, 0.000110352}};

    targetMeansVariances[getTypeName<std::vector<uint8_t>>()] = 
        Vect{Pair{0.251738, 0.152698}, Pair{5.66943, 1.75336}, Pair{3.12096, 0.00556719}, Pair{2.78199, 0.0100038}, Pair{2.77303, 0.0117341}};

    targetMeansVariances[getTypeName<std::vector<uint16_t>>()] = 
        Vect{Pair{0.126835, 0.0414319}, Pair{4.20949, 1.52422}, Pair{3.96366, 0.0341269}, Pair{3.58001, 0.0301698}, Pair{3.526, 0.00934894}};

    targetMeansVariances[getTypeName<std::vector<uint32_t>>()] = 
        Vect{Pair{0.0509902, 0.00126634}, Pair{2.08356, 0.0866378}, Pair{2.31884, 0.000509965}, Pair{2.85175, 0.000714393}, Pair{3.26478, 0.000593733}};

    targetMeansVariances[getTypeName<std::vector<uint64_t>>()] = 
        Vect{Pair{0.0319565, 0.00109159}, Pair{1.45744, 0.21827}, Pair{1.12494, 0.000903077}, Pair{1.29411, 0.00222552}, Pair{1.48029, 0.000160282}};

    targetMeansVariances[getTypeName<std::vector<float>>()] = 
        Vect{Pair{0.0907104, 0.0204795}, Pair{1.95498, 0.0859828}, Pair{2.91714, 0.0158409}, Pair{3.6247, 0.00174018}, Pair{4.0803, 0.00773842}};

    targetMeansVariances[getTypeName<std::vector<double>>()] = 
        Vect{Pair{0.137358, 0.11819}, Pair{1.21765, 0.0466477}, Pair{1.28442, 0.00228186}, Pair{1.52573, 0.000275975}, Pair{1.7623, 0.00100477}};

#if __GNUC__
    targetMeansVariances[getTypeName<std::vector<__int128_t>>()] = 
        Vect{Pair{0.0170585, 1.99507e-05}, Pair{0.551486, 0.00532224}, Pair{0.589677, 5.55333e-05}, Pair{0.658193, 0.00228014}, Pair{0.729345, 0.000131398}};

    targetMeansVariances[getTypeName<std::vector<__uint128_t>>()] = 
        Vect{Pair{0.0151387, 1.51353e-05}, Pair{0.62705, 0.00776813}, Pair{0.579774, 1.06159e-05}, Pair{0.658054, 0.00200743}, Pair{0.727684, 3.29435e-05}};

    targetMeansVariances[getTypeName<std::vector<__float128>>()] = 
        Vect{Pair{0.0795251, 0.0162571}, Pair{1.03878, 0.00831637}, Pair{1.36883, 0.00224225}, Pair{1.56741, 0.000361401}, Pair{1.76124, 0.000246089}};
#endif//__GNUC__

    targetMeansVariances[getTypeName<std::vector<SpecialType>>()] = 
        Vect{Pair{0.0181939, 2.74335e-05}, Pair{0.619061, 0.000791595}, Pair{0.659534, 0.000236414}, Pair{0.792424, 3.80302e-05}, Pair{0.896214, 2.68924e-05}};
    
    targetMeansVariances[getTypeName<std::list<uint32_t>>()] = 
        Vect{Pair{0.00263821, 6.96004e-05}, Pair{3.13024, 0.364601}, Pair{3.8888, 0.269667}, Pair{11.8635, 2.12904}, Pair{15.0476, 0.0815395}};

    targetMeansVariances[getTypeName<std::vector<KeyValue>>()] = 
        Vect{Pair{0.0351223, 9.27595e-05}, Pair{1.45012, 0.119304}, Pair{0.790486, 0.000213119}, Pair{1.05464, 3.63558e-05}, Pair{1.20204, 4.03129e-05}};

    //TODO
    //Z statistic for __float128[100000] : 2.38063
    //Z statistic for unsigned char[1000] : 3.93669
    //Z statistic for int[1000] : 3.69727
    //Z statistic for signed char[1000] : 2.94022

    std::map<std::string, Vect> results;
    std::vector<size_t> sizes{10,1000,100*1000,10*1000*1000,100*1000*1000};

    //helper<SpecialType>(sizes, results);

    //helper<uint32_t, std::list>(sizes, results);

    helper<KeyValue>(sizes, results);

    //helper<int8_t>(sizes, results);
    //helper<int16_t>(sizes, results);
    /*helper<int32_t>(sizes, results);
    helper<int64_t>(sizes, results);

    helper<uint8_t>(sizes, results);
    helper<uint16_t>(sizes, results);
    helper<uint32_t>(sizes, results);
    helper<uint64_t>(sizes, results);

    helper<float>(sizes, results);
    helper<double>(sizes, results);

#if __GNUC__
    helper<__int128_t>(sizes, results);
    helper<__uint128_t>(sizes, results);
    helper<__float128>(sizes, results);
#endif//__GNUC__*/

    for(const auto& p : results)
    {
		const auto& result = p.second;
		
        std::cout << "Vect{";
    
        auto size2 = result.size();
        for(size_t j = 0; j != size2; ++j)
        {   
            auto [m1, v1] = result[j];

            std::cout << "Pair{" << m1 << ", " << v1 << "}";

            if(j != size2-1)
            {
                std::cout << ", ";
            }
        }
        std::cout << "}\n";
    }

    for(const auto& [key, result] : results)
    {
        const auto& target = targetMeansVariances[key];

        auto size2 = result.size();
        if(target.size() != size2)
        {
            std::cout << color::red << "Error: missing '" << key << "' in targetMeansVariances, exiting...\n" << color::reset;
            return -1;
        }

        for(size_t j = 0; j != size2; ++j)
        {   
            auto [newAvg, v1] = result[j];
            auto [prevAvg, v2] = target[j];

            auto relDev1 = sqrt(v1) / newAvg;
            auto relDev2 = sqrt(v2) / prevAvg;
            auto z = fabs(newAvg-prevAvg)/sqrt(relDev1*relDev1 + relDev2*relDev2);

            if(z > 2.0)
            {
                if(prevAvg > newAvg)
                {
                    std::cout << color::yellow;
                }
                else
                {
                    std::cout << color::red;
                }
            }
            else
            {
                std::cout << color::green;
            }

            std::cout << "Z statistic for " << key << "[" << sizes[j] << "] : " << z << " (new = [" 
                      << newAvg << ", " << v1 <<"] vs prev = [" << prevAvg << ", " << v2 << "]" << '\n' << color::reset;
        }
    }

//float results were strange on clang cause the generator was returning only 'inf' and libc++ version
//of std::sort seams smart enough to not sort an array made only of 'inf' (works on all already sorted?)
}
