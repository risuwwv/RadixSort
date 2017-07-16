//
// Copyright (c) 2017, RISUWWV (https://github.com/risuwwv). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

//cls && g++ -o radixSortPerf radixSortPerfTests.cpp -march=native -DNDEBUG -fno-math-errno -flto -O3 -std=c++1z -Werror -fvisibility=hidden -Wno-endif-labels -Wno-missing-field-initializers -Wno-error=effc++ -Wno-error=inline -Wno-error=aggregate-return -Wuseless-cast -Wzero-as-null-pointer-constant -Wnoexcept -Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override -Wtrampolines -Wall -Wextra -Wconversion -Wold-style-cast -Wabi -Wimport -Wmissing-format-attribute -Wmissing-noreturn -Wodr -Woverlength-strings -Wpacked -Wunreachable-code -Wvariadic-macros -Wunused-local-typedefs -Wvla -pedantic -pedantic-errors -Wfloat-equal -Wundef -Wredundant-decls -Wshadow -Wwrite-strings -Wpointer-arith -Wcast-qual -Wswitch-default -Wmissing-include-dirs -Wcast-align -Wformat-nonliteral -Wswitch-enum -Wnon-virtual-dtor -Wctor-dtor-privacy -Wsign-promo -Wsign-conversion -Wdisabled-optimization -Weffc++ -Winline -Winvalid-pch -Wstack-protector -Wmissing-declarations -Woverloaded-virtual -Wvector-operation-performance -Wlogical-op -Wno-pmf-conversions -Wunsafe-loop-optimizations -Wstrict-null-sentinel -Wno-error=noexcept -Wno-missing-declarations -Wno-inline -Wno-unsafe-loop-optimizations  -Wshift-overflow=2 -Wnull-dereference -Wduplicated-cond -Walloc-zero -Walloca  

//TODO check that make perf mode=release compiler=gcc is same as above

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
        return stream << "\033[33m";
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

template<typename T>
void __attribute__((noinline)) radix_sort_caller(std::vector<T>& v)
{
    radix_sort(v.begin(), v.end());
}

template<typename T>
void __attribute__((noinline)) std_sort_caller(std::vector<T>& v)
{
    std::sort(v.begin(), v.end());
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
auto __attribute__((noinline)) radix_sort_caller2(std::vector<T>& v)
{
    auto begin = std::chrono::high_resolution_clock::now();
    radix_sort_caller(v);
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end-begin).count();

    std::cout << "radix_sort(" << abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr) << "[" << v.size() << "]) takes ";
    time_format(std::cout, duration); 
    std::cout << '\n';

    return duration;
}

template<typename T>
auto __attribute__((noinline)) std_sort_caller2(std::vector<T>& v)
{
    auto begin = std::chrono::high_resolution_clock::now();
    std_sort_caller(v);
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end-begin).count();

    std::cout << "std::sort(" << abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr) << "[" << v.size() << "]) takes ";
    time_format(std::cout, duration);
    std::cout << '\n';

    return duration;
}

template<typename T>
auto __attribute__((noinline)) test(std::vector<T>& v, std::vector<T>& vc)
{
    auto d1 = radix_sort_caller2(v);
    auto d2 = std_sort_caller2(vc);
    std::cout << '\n';

    return std::pair{d1,d2};
}

template<typename T>
void init(std::vector<T>& v, size_t size)
{
    std::random_device rnd_device;
    std::mt19937 mersenne_engine(rnd_device());
    std::uniform_int_distribution<T> dist(std::numeric_limits<T>::min()/2, std::numeric_limits<T>::max()/2);//ubsan dislikes it with min() and max() as it overflows in the Mersenne engine
    auto gen = std::bind(dist, mersenne_engine);

    generate_n(std::back_inserter(v), size, gen);
}

template<>
void init<float>(std::vector<float>& v, size_t size)
{
    std::random_device rnd_device;
    std::mt19937 mersenne_engine(rnd_device());

    //std::numeric_limits<float>::min() is positive for float
    //std::uniform_real_distribution<float> dist(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max()); // does not work at all: returns only 'inf'...
    std::uniform_real_distribution<float> dist(-1e20f, 1e20f);

    auto gen = std::bind(dist, mersenne_engine);

    generate_n(std::back_inserter(v), size, gen);
}

template<>
void init<double>(std::vector<double>& v, size_t size)
{
    std::random_device rnd_device;
    std::mt19937 mersenne_engine(rnd_device());
    std::uniform_real_distribution<double> dist(-1e20, 1e20);
    auto gen = std::bind(dist, mersenne_engine);

    generate_n(std::back_inserter(v), size, gen);
}

#if __GNUC__
template<>
void init<__float128>(std::vector<__float128>& v, size_t size)
{
    std::random_device rnd_device;
    std::mt19937 mersenne_engine(rnd_device());
    std::uniform_real_distribution<double> dist(-1e20, 1e20);//does not work with __float128
    auto gen = std::bind(dist, mersenne_engine);

    generate_n(std::back_inserter(v), size, gen);
}
#endif//__GNUC__

template<typename T>
auto testcase(size_t size)
{
    std::vector<T> v;
    v.reserve(size);
    
    init(v, size);
   
    auto vc = v;

    auto timePair = test(v, vc);

    assert(v.size() == vc.size() && std::equal(v.begin(), v.end(), vc.begin()));

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

template<typename T>
auto testcases(const std::vector<size_t>& sizes)
{
    constexpr size_t samples = 10;

    auto length = sizes.size();    
    std::vector<std::vector<std::pair<uint64_t, uint64_t>>> p(length);

    for(size_t i = 0; i < samples; ++i)
    {
        for(size_t j = 0; j < length; ++j) 
        {
            p[j].push_back(testcase<T>(sizes[j]));
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

template<typename T>
void helper(const std::vector<size_t>& sizes, std::vector<Vect>& results, std::vector<std::string>& typeNames)
{
    results.push_back(testcases<T>(sizes));
    typeNames.push_back(abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr));
}

int main()
{
    /*std::vector<Vect> targetMeansVariances{
        Vect{Pair{0.151723, 0.001691}, Pair{6.11161, 2.69297}, Pair{3.19991, 0.05096}, Pair{2.8599, 0.00535732}, Pair{2.85859, 0.00332693}},
        Vect{Pair{0.069839, 0.000460703}, Pair{4.2475, 1.50377}, Pair{3.90123, 0.00138581}, Pair{3.49165, 0.0013107}, Pair{3.4317, 0.000712194}},
        Vect{Pair{0.0424866, 0.000414135}, Pair{3.52203, 0.624935}, Pair{2.2812, 0.000761823}, Pair{2.82312, 0.000576567}, Pair{3.23409, 0.00161815}},
        Vect{Pair{0.0287117, 0.000413468}, Pair{1.93864, 0.16745}, Pair{1.13577, 0.000132134}, Pair{1.28205, 0.00010082}, Pair{1.46748, 6.3323e-05}},
        Vect{Pair{0.0137774, 1.08662e-05}, Pair{0.56736, 0.00756527}, Pair{0.574462, 3.76371e-05}, Pair{0.628044, 5.99537e-05}, Pair{0.707411, 0.000210576}},
        Vect{Pair{0.218344, 0.105146}, Pair{5.06448, 2.57607}, Pair{3.0504, 0.00936787}, Pair{2.74721, 0.00622715}, Pair{2.77592, 0.00247669}},
        Vect{Pair{0.123902, 0.0310118}, Pair{4.75553, 0.980764}, Pair{3.90678, 0.00476715}, Pair{3.51863, 0.00176821}, Pair{3.50966, 0.00135927}},
        Vect{Pair{0.0714727, 0.013183}, Pair{3.34869, 0.734543}, Pair{2.28733, 0.000637636}, Pair{2.84079, 0.000784556}, Pair{3.28401, 0.00115514}},
        Vect{Pair{0.0378208, 0.00276289}, Pair{1.69576, 0.240637}, Pair{1.10123, 0.00124044}, Pair{1.28125, 2.93811e-05}, Pair{1.47974, 0.00023441}},
        Vect{Pair{0.0191167, 1.28252e-05}, Pair{0.623884, 0.00704246}, Pair{0.598624, 6.75677e-05}, Pair{0.659182, 0.000297644}, Pair{0.734993, 0.00030738}},
        Vect{Pair{0.0618336, 0.00293173}, Pair{1.95969, 0.0292084}, Pair{2.90473, 0.0122568}, Pair{3.56639, 0.00201899}, Pair{4.06799, 0.000866975}},
        Vect{Pair{0.0473525, 0.00279145}, Pair{1.14862, 0.0121511}, Pair{1.32315, 0.000576055}, Pair{1.52597, 8.67487e-05}, Pair{1.75692, 0.000136875}},
        Vect{Pair{0.0825891, 0.0226774}, Pair{1.0888, 0.00280791}, Pair{1.42572, 0.000730961}, Pair{1.5778, 0.000750334}, Pair{1.76378, 0.00062688}}};
*/
    std::vector<Vect> targetMeansVariances{
        Vect{Pair{0.136619, 0.00105332}, Pair{5.40873, 1.6909}, Pair{3.19948, 0.0233439}, Pair{2.8654, 0.00291782}, Pair{2.84225, 0.0048124}},
        Vect{Pair{0.0870894, 0.00417521}, Pair{3.74942, 0.930056}, Pair{3.90567, 0.00110855}, Pair{3.47863, 0.00150519}, Pair{3.41936, 0.000793076}},
        Vect{Pair{0.0587488, 0.00477154}, Pair{3.16355, 0.340866}, Pair{2.25233, 0.000815486}, Pair{2.82961, 0.000597585}, Pair{3.25518, 0.000954513}},
        Vect{Pair{0.0327907, 0.000718227}, Pair{1.94566, 0.442034}, Pair{1.11678, 0.00143563}, Pair{1.29264, 0.000791579}, Pair{1.48278, 0.000110352}},
        Vect{Pair{0.0170585, 1.99507e-05}, Pair{0.551486, 0.00532224}, Pair{0.589677, 5.55333e-05}, Pair{0.658193, 0.00228014}, Pair{0.729345, 0.000131398}},
        Vect{Pair{0.251738, 0.152698}, Pair{5.66943, 1.75336}, Pair{3.12096, 0.00556719}, Pair{2.78199, 0.0100038}, Pair{2.77303, 0.0117341}},
        Vect{Pair{0.126835, 0.0414319}, Pair{4.20949, 1.52422}, Pair{3.96366, 0.0341269}, Pair{3.58001, 0.0301698}, Pair{3.526, 0.00934894}},
        Vect{Pair{0.0509902, 0.00126634}, Pair{2.08356, 0.0866378}, Pair{2.31884, 0.000509965}, Pair{2.85175, 0.000714393}, Pair{3.26478, 0.000593733}},
        Vect{Pair{0.0319565, 0.00109159}, Pair{1.45744, 0.21827}, Pair{1.12494, 0.000903077}, Pair{1.29411, 0.00222552}, Pair{1.48029, 0.000160282}},
        Vect{Pair{0.0151387, 1.51353e-05}, Pair{0.62705, 0.00776813}, Pair{0.579774, 1.06159e-05}, Pair{0.658054, 0.00200743}, Pair{0.727684, 3.29435e-05}},
        Vect{Pair{0.0907104, 0.0204795}, Pair{1.95498, 0.0859828}, Pair{2.91714, 0.0158409}, Pair{3.6247, 0.00174018}, Pair{4.0803, 0.00773842}},
        Vect{Pair{0.137358, 0.11819}, Pair{1.21765, 0.0466477}, Pair{1.28442, 0.00228186}, Pair{1.52573, 0.000275975}, Pair{1.7623, 0.00100477}},
        Vect{Pair{0.0795251, 0.0162571}, Pair{1.03878, 0.00831637}, Pair{1.36883, 0.00224225}, Pair{1.56741, 0.000361401}, Pair{1.76124, 0.000246089}}};

    //TODO
    //Z statistic for __float128[100000] : 2.38063
    //Z statistic for unsigned char[1000] : 3.93669
    //Z statistic for int[1000] : 3.69727
    //Z statistic for signed char[1000] : 2.94022

    std::vector<std::string> typeNames;
    std::vector<Vect> results;
    std::vector<size_t> sizes{10,1000,100*1000,10*1000*1000,100*1000*1000};

    helper<int8_t>(sizes, results, typeNames);
    helper<int16_t>(sizes, results, typeNames);
    helper<int32_t>(sizes, results, typeNames);
    helper<int64_t>(sizes, results, typeNames);

#if __GNUC__
    //TODO move assert to other test
    static_assert(std::is_unsigned_v<__uint128_t>);
    static_assert(std::is_integral_v<__uint128_t>);
    static_assert(std::is_integral_v<__int128_t>);
    static_assert(!std::is_unsigned_v<__int128_t>);

    helper<__int128_t>(sizes, results, typeNames);
#else
    results.push_back({0.0, 0.0});
    typeNames.push_back("__int128_t");
#endif//__GNUC__

    helper<uint8_t>(sizes, results, typeNames);
    helper<uint16_t>(sizes, results, typeNames);
    helper<uint32_t>(sizes, results, typeNames);
    helper<uint64_t>(sizes, results, typeNames);

#if __GNUC__
    helper<__uint128_t>(sizes, results, typeNames);
#else
    results.push_back({0.0, 0.0});
    typeNames.push_back("__uint128_t");
#endif//__GNUC__

    helper<float>(sizes, results, typeNames);
    helper<double>(sizes, results, typeNames);

#if __GNUC__
    helper<__float128>(sizes, results, typeNames);
#else
    results.push_back({0.0, 0.0});
    typeNames.push_back("__float128");
#endif//__GNUC__

    auto size = results.size();
    for(size_t i = 0; i != size; ++i)
    {
        auto& result = results[i];

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
        std::cout << "},\n";
    }

    for(size_t i = 0; i != size; ++i)
    {
        auto& result = results[i];
        auto& target = targetMeansVariances[i];

        auto size2 = result.size();
        for(size_t j = 0; j != size2; ++j)
        {   
            auto [m1, v1] = result[j];
            auto [m2, v2] = target[j];

            auto relativeDev1 = sqrt(v1) / m1;
            auto relativeDev2 = sqrt(v2) / m2;
            auto z = fabs(m1-m2)/sqrt(relativeDev1*relativeDev1 + relativeDev2*relativeDev2);

            if(z > 2.0)
            {
                if(m2 < m1)
                {
                    std::cout << color::yellow;
                }
                std::cout << color::red;
            }
            else
            {
                std::cout << color::green;
            }

            std::cout << "Z statistic for " << typeNames[i] << "[" << sizes[j] << "] : " << z << '\n' << color::reset;
        }
    }


/*
float results were strange on clang cause the generator was returning only 'inf' and libc++ version
of std::sort seams smart enough to not sort an array made only of 'inf' (works on all already sorted?)

TODO: add http://www.boost.org/doc/libs/1_64_0/libs/sort/doc/html/index.html to the comparisons
TODO: pairs of uint64_t to see why uint128 is so slow
TODO: check with 8bits, 11bits, 14bits and 15bits
TODO: same with gcc
TODO: enforce the speedups in a statically sound way (average and variance within bounds) to prevent regressions
TODO: colors on speedup
TODO: small container optimization: stack allocation
TODO: push back to github
*/

}
