//
// Copyright (c) 2017, RISUWWV (https://github.com/risuwwv). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

//cls && g++ -o radixSort radixSortTests.cpp -march=native -DNDEBUG -fno-math-errno -flto -O3 -std=c++1z -Werror -fvisibility=hidden -Wno-endif-labels -Wno-missing-field-initializers -Wno-error=effc++ -Wno-error=inline -Wno-error=aggregate-return -Wuseless-cast -Wzero-as-null-pointer-constant -Wnoexcept -Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override -Wtrampolines -Wall -Wextra -Wconversion -Wold-style-cast -Wabi -Wimport -Wmissing-format-attribute -Wmissing-noreturn -Wodr -Woverlength-strings -Wpacked -Wunreachable-code -Wvariadic-macros -Wunused-local-typedefs -Wvla -pedantic -pedantic-errors -Wfloat-equal -Wundef -Wredundant-decls -Wshadow -Wwrite-strings -Wpointer-arith -Wcast-qual -Wswitch-default -Wmissing-include-dirs -Wcast-align -Wformat-nonliteral -Wswitch-enum -Wnon-virtual-dtor -Wctor-dtor-privacy -Wsign-promo -Wsign-conversion -Wdisabled-optimization -Weffc++ -Winline -Winvalid-pch -Wstack-protector -Wmissing-declarations -Woverloaded-virtual -Wvector-operation-performance -Wlogical-op -Wno-pmf-conversions -Wunsafe-loop-optimizations -Wstrict-null-sentinel -Wno-error=noexcept -Wno-missing-declarations -Wno-inline -Wno-unsafe-loop-optimizations  -Wshift-overflow=2 -Wnull-dereference -Wduplicated-cond -Walloc-zero -Walloca  

//cls && clang++ -std=c++1z -Werror -fvisibility=hidden -Wno-endif-labels -Wno-missing-field-initializers -Wno-error=effc++ -Wno-error=inline -Wno-error=aggregate-return -Weverything -fdiagnostics-color=always -Wno-c++98-compat-pedantic -Wno-error=deprecated -Wno-missing-prototypes -Wno-missing-variable-declarations -Wno-error=global-constructors -Wno-padded radixSortTests.cpp -o radixSort -stdlib=libc++ -Wno-undefined-func-template -Wno-missing-braces

#include <limits>
#include <utility>
#include <random>
#include <list>
#include <array>
#include <vector>
#include <iostream>
#include <cassert>

#include "radixSort.h"

using namespace risuwwv;

namespace testcases
{

//on compilater that don't have header detection, ignore this
#if defined(__has_include)
#if __has_include("boost/pfr.hpp")

    struct TestStruct { // no ostream operator defined!
        int i;
        char c;
        double d;
    };

    //for std::sort only, not used by radix_sort
    bool operator<(const TestStruct& lhs, const TestStruct& rhs)
    {
        return boost::pfr::flat_structure_to_tuple(lhs) < boost::pfr::flat_structure_to_tuple(rhs);
    }

    //for the test only
    bool operator==(const TestStruct& lhs, const TestStruct& rhs)
    {
        return boost::pfr::flat_structure_to_tuple(lhs) == boost::pfr::flat_structure_to_tuple(rhs);
    }

    void magic_get_testcases()
    {
        //gcc-7.1.0 does not accept the example on https://github.com/apolukhin/magic_get but we don't need as many features

        std::vector<TestStruct> v{TestStruct{1,'c', 41.0}, TestStruct{-1,'c', 41.0}, TestStruct{-1,'b', 41.0}, TestStruct{1,'b', 41.0}};
        auto vc = v;

        radix_sort(v);   
        std::sort(vc.begin(), vc.end());    
        assert(vc.size() == v.size() && std::equal(vc.begin(), vc.end(), v.begin())); 

        //pfr.hpp is so cool!!
    }
#endif//defined(__has_include)
#endif//__has_include("boost/pfr.hpp")

    void array_testcases()
    {
        std::array<int, 10> v{1,54,5,21,42,1,21,4,54,4};
        auto vc = v;

        radix_sort(v);
        std::sort(vc.begin(), vc.end()); 
        assert(vc.size() == v.size() && std::equal(vc.begin(), vc.end(), v.begin()));

        int v2[]{31,54,5,21,42,1,21,4,54,4};
        int vc2[10]; 
        std::copy(std::begin(v2), std::end(v2), std::begin(vc2));

        radix_sort(v2);
        std::sort(std::begin(vc2), std::end(vc2)); 
        assert(std::size(vc2) == std::size(v2) && std::equal(std::begin(vc2), std::end(vc2), std::begin(v2)));    

        const int v3[]{-31,54,5,21,42,1,21,4,54,4};
        int vc3[10]; 
        std::copy(std::begin(v3), std::end(v3), std::begin(vc3));

        std::vector<int> target;
        radix_sort(v3, std::back_inserter(target));
        std::sort(std::begin(vc3), std::end(vc3)); 
        assert(std::size(vc3) == std::size(v3) && std::equal(std::begin(vc3), std::end(vc3), std::begin(target)));  

        std::vector<std::array<int, 3>> v4{{1,2,7},{-1,3,4},{1,2,8},{-1,4,3}};  
        auto vc4 = v4;

        radix_sort(v4);
        std::sort(std::begin(vc4), std::end(vc4));        
        assert(std::size(vc4) == std::size(v4) && std::equal(std::begin(vc4), std::end(vc4), std::begin(v4)));    

        auto res = risuwwv::details::decay_value(std::array<const int, 3>{1,2,7});
        static_assert(std::is_same_v<decltype(res), std::array<int, 3>>);
        assert((res == std::array<int, 3>{1,2,7}));

        std::vector<std::array<const int, 3>> v5{{1,2,7},{-1,3,4},{1,2,8},{-1,4,3}}; 
        std::vector<std::array<int, 3>> vc5; //= v5 does not work
        std::transform(v5.begin(), v5.end(), std::back_inserter(vc5), [](const auto& elem){
            return risuwwv::details::decay_value(elem);
        });

        std::vector<std::array<int, 3>> target2;

        radix_sort(v5, std::back_inserter(target2));
        std::sort(std::begin(vc5), std::end(vc5));        
        assert(std::size(vc5) == std::size(target2) && std::equal(std::begin(vc5), std::end(vc5), std::begin(target2)));

        //probably illegal (gcc refuses it, clang is ok):
        //std::vector<const std::array<const int, 3>> v5{{1,2,7},{-1,3,4},{1,2,8},{-1,4,3}};  

        using Ar = std::array<int,2>;
        using Ar2 = std::array<Ar, 3>;
        
        std::vector<Ar2> v6{Ar2{Ar{1,2}, Ar{3,4}, Ar{5,6}}, Ar2{Ar{-1,2}, Ar{3,4}, Ar{5,6}},
                                                         Ar2{Ar{-1,-2}, Ar{3,4}, Ar{5,6}}, Ar2{Ar{1,-2}, Ar{3,4}, Ar{5,6}}};

        auto vc6 =v6;

        radix_sort(v6);
        std::sort(std::begin(vc6), std::end(vc6)); 
        assert(std::size(vc6) == std::size(v6) && std::equal(std::begin(vc6), std::end(vc6), std::begin(v6)));  

        using Tpl = std::tuple<int, double>;
        using Ar3 = std::array<Tpl, 2>;
        std::vector<Ar3> v7{Ar3{Tpl{}, Tpl{1,6.0}}, Ar3{Tpl{-1,7.0}, Tpl{-2,3.0}}, Ar3{Tpl{2,6.0}, Tpl{-4,6.5}}, Ar3{Tpl{3,2.0}, Tpl{1,5.5}}};
        
        auto vc7 = v7;
        radix_sort(v7);
        std::sort(std::begin(vc7), std::end(vc7)); 
        assert(std::size(vc7) == std::size(v7) && std::equal(std::begin(vc7), std::end(vc7), std::begin(v7)));       
    }

    void const_testcases()
    {
        const std::vector<int> v{-1356569119, -422563130,-1352029410,395770343,1785423163,-1351687749,-559142332,-140471621,-313712167,627271371,-1604412862,763551832,440576809,1616784356,-1399382555};
        auto vc = v;

        std::vector<int> target;

        radix_sort(v, std::back_inserter(target));
        std::sort(vc.begin(), vc.end());

        assert(target.size() == vc.size() && std::equal(vc.begin(), vc.end(), target.begin()));

        using Tup1 =std::pair<int32_t, int16_t>;

        using Tup =std::tuple<int8_t, const Tup1>;

        using Targ = risuwwv::details::recursive_decay_t<Tup>;

        static_assert(std::is_same_v<std::tuple<int8_t, std::pair<int32_t, int16_t>>, Targ>);

        std::vector<Tup> v2{Tup{82, Tup1{-1521250841, 8261}}, Tup{-9, Tup1{-782619114, -14787}}, Tup{14, Tup1{-205344574, -31945}}, Tup{-50, Tup1{665513844, -3181}}, Tup{107, Tup1{-111254781, -20765}}, Tup{20, Tup1{1918541230, -30303}}, Tup{37, Tup1{-849382585, -14716}}, Tup{-40, Tup1{-1938031009, 12790}}, Tup{89, Tup1{-1296684139, -26121}}, Tup{-1, Tup1{-2047314039, 28625}}, Tup{61, Tup1{-895186301, -22402}}, Tup{-2, Tup1{-1382437165, 2219}}, Tup{-81, Tup1{772505587, 31694}}, Tup{7, Tup1{-30777881, 10708}}, Tup{69, Tup1{-780193668, 28050}}, Tup{126, Tup1{-1156394723, -13703}}};

        std::vector<Targ> vc2{v2.begin(), v2.end()};  

        std::vector<Targ> target2;      

        radix_sort(v2, std::back_inserter(target2));
        std::sort(vc2.begin(), vc2.end());

        assert(target2.size() == vc2.size() && std::equal(vc2.begin(), vc2.end(), target2.begin()));

        using CstTup = const std::tuple<const int8_t, int8_t>;
        using Helper = std::tuple<const int8_t, CstTup, CstTup, const int8_t>;
        using HellTuple = std::tuple<const int8_t, const Helper, const Helper, const int32_t>;

        //don't hire me to write this type of things :)
        std::vector<HellTuple> v3{
                HellTuple{125, Helper{253, CstTup{254,125}, CstTup{-121,137}, 27}, Helper{253, CstTup{254,125}, CstTup{-121,137}, 27}, 245454},
                HellTuple{125, Helper{253, CstTup{254,125}, CstTup{-121,137}, 27}, Helper{253, CstTup{254,125}, CstTup{-121,137}, 27}, -245454},
                HellTuple{125, Helper{253, CstTup{254,125}, CstTup{-121,137}, 27}, Helper{253, CstTup{254,125}, CstTup{-121,137}, 27}, 38},
                HellTuple{125, Helper{253, CstTup{254,125}, CstTup{-121,137}, 27}, Helper{253, CstTup{254,125}, CstTup{-121,137}, 27}, 42}};

        using HellTuple2 = risuwwv::details::recursive_decay_t<HellTuple>;

        std::vector<HellTuple2> vc3{v3.begin(), v3.end()};
        std::vector<HellTuple2> target3;     

        radix_sort(v3, std::back_inserter(target3));
        std::sort(vc3.begin(), vc3.end());

        assert(target3.size() == vc3.size() && std::equal(vc3.begin(), vc3.end(), target3.begin())); 
    }

    //not adapted for float as comparison of NaNs is strange
    template<typename Integer>
    void int_testcase(const std::vector<Integer>& v)
    {
        auto v2 = v;
        auto v3 = v;

        radix_sort(v2.begin(), v2.end());
        std::sort(v3.begin(), v3.end());

        /*if(!std::equal(v2.begin(), v2.end(), v3.begin()))
        {
            std::cout << "v2 ";
            std::copy(v2.begin(), v2.end(), std::ostream_iterator<std::size_t>(std::cout, " "));
            std::cout << '\n';

            std::cout << "v3 ";
            std::copy(v3.begin(), v3.end(), std::ostream_iterator<std::size_t>(std::cout, " "));
            std::cout << '\n';
        }*/

        assert(v2.size() == v.size() && std::equal(v2.begin(), v2.end(), v3.begin()));
    }

    template<std::size_t>
    struct Float_equivalent
    {};

    template<>
    struct Float_equivalent<8>
    {
        using type = double;
    };

    template<>
    struct Float_equivalent<4>
    {
        using type = float;
    };   

    template<typename Integer>
    void float_testcase(const std::vector<Integer>& v)
    {
        using Float = typename Float_equivalent<sizeof(Integer)>::type;

        std::vector<Float> tmp;
        std::transform(v.begin(), v.end(), std::back_inserter(tmp), 
            [](Integer val){
                return details::bit_cast<Float, Integer>(val);
            });

        auto v2 = tmp;
        auto v3 = tmp;

        radix_sort(v2.begin(), v2.end());
        std::sort(v3.begin(), v3.end());

        assert(v2.size() == v.size() && 
            std::equal(v2.begin(), v2.end(), v3.begin(), 
                [](Float lhs, Float rhs)
                {
                    return details::bit_cast<Integer, Float>(lhs) == details::bit_cast<Integer, Float>(rhs);//NaNs are different than themselves
                }));
    }

    void int_testcases()
    {
        int_testcase(std::vector<int>{});
        int_testcase(std::vector<int>{42});
        int_testcase(std::vector<int>{-1, 1});
        int_testcase(std::vector<int>{1, 4, 2, 5, 7, 11, 33});
        int_testcase(std::vector<int>{1, 4, 2, 5, 7, 11, 4, 33});

        int_testcase(std::vector<int>{-2, -1, 0, 1});
        int_testcase(std::vector<int>{-2, -1, 1, 0});
        int_testcase(std::vector<int>{-2, 0, -1, 1});
        int_testcase(std::vector<int>{-2, 0, 1, -1});
        int_testcase(std::vector<int>{-2, 1, -1, 0});
        int_testcase(std::vector<int>{-2, 1, 0, -1}); 

        int_testcase(std::vector<int>{-1, -2, 0, 1});
        int_testcase(std::vector<int>{-1, -2, 1, 0});
        int_testcase(std::vector<int>{-1, 0, -2, 1});
        int_testcase(std::vector<int>{-1, 0, 1, -2});
        int_testcase(std::vector<int>{-1, 1, -2, 0});
        int_testcase(std::vector<int>{-1, 1, 0, -2});

        int_testcase(std::vector<int>{1, -2, 0, -1});
        int_testcase(std::vector<int>{1, -2, -1, 0});
        int_testcase(std::vector<int>{1, 0, -2, -1});
        int_testcase(std::vector<int>{1, 0, -1, -2});
        int_testcase(std::vector<int>{1, -1, -2, 0});
        int_testcase(std::vector<int>{1, -1, 0, -2});
 
        int_testcase(std::vector<int>{0, -2, 1, -1});
        int_testcase(std::vector<int>{0, -2, -1, 1});
        int_testcase(std::vector<int>{0, 1, -2, -1});
        int_testcase(std::vector<int>{0, 1, -1, -2});
        int_testcase(std::vector<int>{0, -1, -2, 1});
        int_testcase(std::vector<int>{0, -1, 1, -2});  


        int_testcase(std::vector<int>{1107311617, -1893159634, -1867447344, 1696518018, -1859364552, 1866293585, -1857175229, -201478867, 615059417, -551781893, -1811319281, 874255129, 1430170732, 413573467, 1999187880, 869415287});

        //randomly generated
        int_testcase(std::vector<int>{1107311617, -1893159634, -1867447344, 1696518018, -1859364552, 1866293585, -1857175229, -201478867, 615059417, -551781893, -1811319281, 874255129, 1430170732, 413573467, 1999187880, 869415287});
        int_testcase(std::vector<int>{197368545, 2007337319, -1941233492, 1439462150, -13195599, 2085020492, 1952337337, 162485865, 1911258710, 1196735356, 1910052146, 1532904003, 701287548, -2014383691, -1975010620});
        int_testcase(std::vector<int>{-1091105931, 980714780, -461883954, -2102428785, -142351001, 931422523, 1447692662, -1423895154, -1074893645, -1221815884, -1052830763, 868686744, -470417128, 726370077});
        int_testcase(std::vector<int>{1770505096, -352457978, -740775673, -1576265248, 1502969398, -1535243439, -1025727763, -875057370, -1778222821, -71724485, 339696508, 2074735987, 1625069913});  
        int_testcase(std::vector<int>{849409494, 132417626, 293739251, 372739124, -1556161000, -1441738872, -1515758919, -1774050118, -238818846, 1392302586, 316899726, 450671406});  
        int_testcase(std::vector<int>{-1257939017, 1356233175, -622144793, 1946033574, -1939519631, 1311490890, -514138777, 2101769146, -1746921597, 715856542, 1383346755});  
        int_testcase(std::vector<int>{917837233, 210737018, 1615008561, 425993736, 517253458, 22164689, -1023930052, 1606450998, -515600595, 625563373});   
        int_testcase(std::vector<int>{385000237, 2074681372, -180681061, 419983298, 423131308, -121081781, 1008043018, -428297368, -255154818});  
        int_testcase(std::vector<int>{-1816561814, 1452941613, -233399697, -1489964821, -156929759, -1300100224, -1202667649});  
        int_testcase(std::vector<int>{1965405046, -842296084, 412086663, -1983665198, 1954998215, -1047026477}); 
        int_testcase(std::vector<int>{1844444501, -2074258108, 969944464, 1604763831, 2089455947});   
        int_testcase(std::vector<int>{2148986, -1153206284, 1679375893, 870921407});   
        int_testcase(std::vector<int>{1914201243, -1364924158, -1540640591});  
        int_testcase(std::vector<int>{37944653, -1640951946}); 
        int_testcase(std::vector<int>{804707415}); 
    }

    void float_testcases()
    {
        float_testcase(std::vector<int>{-1356569119, -422563130,-1352029410,395770343,1785423163,-1351687749,-559142332,-140471621,-313712167,627271371,-1604412862,763551832,440576809,1616784356,-1399382555});
        float_testcase(std::vector<int>{-756409216, 576595219,147352108,-1798428427,-1712713075,975949193,1993832975,-252938704,1991177445,-1301106677,-269899389,-1834975815,-724975025,-231037640});
        float_testcase(std::vector<int>{-1762984579, 2138943090,2025148171,-1847268196,141478690,1217246495,-1368348154,368867411,-300074518,2090291135,-374625266,-1353801110,-676146797});
        float_testcase(std::vector<int>{-1424162538, 1848244056,1424218350,1447771888,1463207589,-474639137,687737955,1249185001,1924783830,4892783,714276741,29037326});
        float_testcase(std::vector<int>{656787732, -1520753696,-376890991,-566750050,-819460951,-1593146851,-1539908256,527113861,2050278366,-1930590320,-1730477959});
        float_testcase(std::vector<int>{1311533457, -1701602623,-1615043135,1926204301,1613151125,2123639583,596856897,-1309083051,1247808019,2082085548});
        float_testcase(std::vector<int>{-261132083, 1810573696,200203276,507526233,-772782763,-941885048,-1881962234,-200547304,1032601585});
        float_testcase(std::vector<int>{1806668039, 1022441670,1843962077,1498229084,2051973048,1416630281,-714591292});
        float_testcase(std::vector<int>{2051422467, 1544469173,1425274378,-978349883,736200424,1091127312});
        float_testcase(std::vector<int>{-431007896, -38239707,1544022954,1241333726,276507793});
        float_testcase(std::vector<int>{-1648888734, 641167639,1535064263,-1024681389});
        float_testcase(std::vector<int>{-693860427, 922283682,-1699433844});
        float_testcase(std::vector<int>{-364494615, -358968238});
        float_testcase(std::vector<int>{1582333377});

        //-4194304 => -NaN,     2139095040 => +inf,     -8388608 => -inf
        float_testcase(std::vector<int>{-4194304, 2139095040, 10, -8388608});
    }

    void double_testcases()
    {
        float_testcase(std::vector<int64_t>{5166950043776760000ll, 7703263844257170000,2514332659977400000,-1297298840733950000,946675713689387000,-6262958343371610000,4266421780913290000,-7867193517652540000,4140165466564780000,5559886280227320000,6793037895639390000,8935735603197710000,5442569018439790000,-3528310787602810000,-4754792994694770000ll});
        float_testcase(std::vector<int64_t>{-4840378265572450000ll, -5683448907578380000,3867382446254390000,-1368176910128970000,8811186859241370000,-4884726506345330000,1910536168222220000,-2937616646326620000,-6123389781133110000,3419696032680170000,3158014692334450000,8899869691762990000,-7376362198745440000,-2769270770200850000ll});
        float_testcase(std::vector<int64_t>{-1122384363279430000ll, 1063437150473130000,-8009679568426230000,7668716190249980000,3735606364858140000,-4437633836601420000,-8665321244732610000,5242725206840820000,-8249187708876850000,-1270127691849810000,8698352737494190000,-7965657690957460000,6869610001551930000ll});
        float_testcase(std::vector<int64_t>{6912184291992240000ll, 7847693241451640000,-5594664454849300000,4114538621967320000,5205756254657600000,5440364566480780000,-2576782908611580000,6679979710435720000,-4914716958252740000,7065825079337440000,767513927085101000,-6615580267703610000ll});
        float_testcase(std::vector<int64_t>{-3705509191288720000ll, 3715807706485210000,-1176926116571480000,4635621395402470000,896642397825155000,-1725674557274260000,8181227202493260000,2282973743543760000,1784035096577700000,-3020464190264830000,-3907949079620500000ll});
        float_testcase(std::vector<int64_t>{8415380105608920000ll, -6787539332198550000,-9179102693810900000,3890578474920240000,957207317820981000,-5224773087783170000,215760488843100000,4761175008022420000,3141560240576500000,754580177384980000ll});
        float_testcase(std::vector<int64_t>{3994802002231310000ll, 2357161368585600000,5395057946573340000,-4832132830407900000,7696140257092210000,7974258568864830000,7501435384267150000,1129590892032190000,-4329583752382970000ll});
        float_testcase(std::vector<int64_t>{-8178864475063500000ll, -8756955706154910000,-7613244497755440000,-886507567989613000,1237569993794910000,-5605454548425590000,-8441325195437820000ll});
        float_testcase(std::vector<int64_t>{-4122185803871470000ll, 1705734254004390000,-3845695972842240000,-3244396149482320000,860508822070378000,7084566097474750000ll});
        float_testcase(std::vector<int64_t>{-6521306093758170000ll, 172715127444603000,-3102196878182040000,-2114567066449560000,6869221319888530000ll});
        float_testcase(std::vector<int64_t>{-213484668751655000ll, 3109166935135240000,1189978521066320000,-5927948476301390000ll});
        float_testcase(std::vector<int64_t>{-4147257678679450000ll, -7916472571308450000,8437581683650460000ll});
        float_testcase(std::vector<int64_t>{-4506806257962530000ll, -8016885609234400000ll});
        float_testcase(std::vector<int64_t>{4233127938280680000ll});
    }

    void tuple1_testcases()
    {
        using Tup =std::tuple<int8_t, int16_t>;

        int_testcase(std::vector<Tup>{Tup{-8, -18782}, Tup{-38, 881}, Tup{-67, 2114}, Tup{20, -14878}, Tup{-71, 11158}, Tup{5, -13775}, Tup{-32, 13524}, Tup{-116, 11699}, Tup{67, -22793}, Tup{-126, 6637}, Tup{54, 13416}, Tup{-122, -28329}, Tup{-51, -30713}, Tup{-75, -12775}, Tup{23, -12478}, Tup{-56, -2578}});
        int_testcase(std::vector<Tup>{Tup{-60, -14393}, Tup{-8, 17738}, Tup{5, -3359}, Tup{43, 26285}, Tup{14, -5645}, Tup{-22, 9924}, Tup{-100, 8762}, Tup{-97, -8415}, Tup{37, 8794}, Tup{-72, -933}, Tup{55, 31775}, Tup{-20, -8204}, Tup{-56, -26343}, Tup{-83, 9254}, Tup{107, 18665}});
        int_testcase(std::vector<Tup>{Tup{-11, 9794}, Tup{-44, -704}, Tup{-90, -32394}, Tup{-58, 29147}, Tup{-104, 3988}, Tup{108, -18726}, Tup{46, 7172}, Tup{-118, 16229}, Tup{-79, 3222}, Tup{-72, 14033}, Tup{48, -31881}, Tup{-96, 4990}, Tup{81, -31903}, Tup{-91, -19632}});
        int_testcase(std::vector<Tup>{Tup{120, 10589}, Tup{119, 21644}, Tup{-101, 10248}, Tup{46, -31781}, Tup{86, 14759}, Tup{-31, 18646}, Tup{28, 8634}, Tup{111, 9792}, Tup{125, 5630}, Tup{-88, 20419}, Tup{42, 6889}, Tup{32, 8895}, Tup{-7, 30689}});
        int_testcase(std::vector<Tup>{Tup{116, -6964}, Tup{90, -19412}, Tup{46, 1027}, Tup{-29, 8327}, Tup{46, -3470}, Tup{112, 24091}, Tup{-78, 9566}, Tup{107, 9526}, Tup{72, 7091}, Tup{-123, 12333}, Tup{-112, -5619}, Tup{33, 454}});
        int_testcase(std::vector<Tup>{Tup{118, -5585}, Tup{42, 19072}, Tup{-64, -15250}, Tup{-94, 6113}, Tup{-68, -5349}, Tup{21, -27595}, Tup{58, -29149}, Tup{94, 26454}, Tup{-29, 15372}, Tup{121, 16523}, Tup{61, 23603}});
        int_testcase(std::vector<Tup>{Tup{115, 14664}, Tup{-87, 30951}, Tup{-126, 18338}, Tup{1, 3961}, Tup{-78, -9558}, Tup{-54, -22530}, Tup{83, -15510}, Tup{124, -14168}, Tup{118, 8007}, Tup{88, -31757}});
        int_testcase(std::vector<Tup>{Tup{123, -11011}, Tup{29, -4252}, Tup{-22, 29061}, Tup{20, 11004}, Tup{-83, -7697}, Tup{-15, 8619}, Tup{-53, 27996}, Tup{-98, -21508}, Tup{-53, -18796}});
        int_testcase(std::vector<Tup>{Tup{113, -19737}, Tup{-128, 14433}, Tup{10, 7907}, Tup{-68, 19841}, Tup{-90, -24608}, Tup{-2, -21831}, Tup{31, 27457}, Tup{118, 16120}});
        int_testcase(std::vector<Tup>{Tup{44, 10981}, Tup{-96, 2021}, Tup{-48, 47}, Tup{-7, 24512}, Tup{-102, 30691}, Tup{-125, -21906}, Tup{123, -13937}});
        int_testcase(std::vector<Tup>{Tup{-21, 22156}, Tup{-6, 20721}, Tup{-20, 3502}, Tup{9, -26624}, Tup{-44, 386}, Tup{-87, 17288}});
        int_testcase(std::vector<Tup>{Tup{58, -22468}, Tup{50, -12375}, Tup{-80, -29455}, Tup{88, 28143}, Tup{-108, -31955}});
        int_testcase(std::vector<Tup>{Tup{82, -24956}, Tup{46, -26466}, Tup{-124, -8964}, Tup{-2, 21431}});
        int_testcase(std::vector<Tup>{Tup{-63, 1102}, Tup{-61, -2855}, Tup{-51, -17080}});
        int_testcase(std::vector<Tup>{Tup{-4, 17282}, Tup{13, -4087}});
        int_testcase(std::vector<Tup>{Tup{73, 14369}});
    }

    void tuple2_testcases()
    {
        using Tup =std::tuple<int32_t, int8_t>;

        int_testcase(std::vector<Tup>{Tup{-508987177, -35}, Tup{1891947452, 86}, Tup{1335770070, 78}, Tup{130809850, -30}, Tup{-1592685673, -46}, Tup{-1586778294, 10}, Tup{-1464055667, 126}, Tup{1730078530, 33}, Tup{-1284554855, -120}, Tup{1086300232, 6}, Tup{-1519413548, -100}, Tup{-1127139889, 82}, Tup{1030127111, -60}, Tup{-1642700635, 89}, Tup{-339362628, 39}, Tup{-1585719437, -44}});
        int_testcase(std::vector<Tup>{Tup{-1332117895, -128}, Tup{1205459268, -34}, Tup{-983284413, 111}, Tup{136032832, -45}, Tup{-411665976, -12}, Tup{56536736, -98}, Tup{-545315949, -82}, Tup{-1179132292, 20}, Tup{142109105, -101}, Tup{1452962471, -60}, Tup{-1954880418, 62}, Tup{-2129135317, -64}, Tup{-1327494731, -80}, Tup{1782502218, 92}, Tup{976237785, 101}});
        int_testcase(std::vector<Tup>{Tup{-1900361442, 53}, Tup{1122913140, -11}, Tup{-482026772, -73}, Tup{-177850462, 97}, Tup{-1568130233, 14}, Tup{-1404055263, 81}, Tup{534964766, 57}, Tup{1950699390, -111}, Tup{-1438864619, -128}, Tup{1325826945, -63}, Tup{-1816937662, -46}, Tup{1832532622, -67}, Tup{-971870657, -83}, Tup{94364542, 75}});
        int_testcase(std::vector<Tup>{Tup{149310929, -37}, Tup{-801364872, 22}, Tup{-1803249110, -3}, Tup{-1969042688, -92}, Tup{1853763231, 116}, Tup{160822345, 126}, Tup{1936867818, -19}, Tup{354205405, -51}, Tup{-1826413415, -11}, Tup{731312691, 119}, Tup{2089003793, 92}, Tup{-697930900, -70}, Tup{1737340473, -26}});
        int_testcase(std::vector<Tup>{Tup{1114147304, 52}, Tup{143079833, -110}, Tup{307968829, 16}, Tup{-860458878, -80}, Tup{-1694824498, -47}, Tup{-1132809406, 116}, Tup{-1910027700, 84}, Tup{619704774, 111}, Tup{-1819767833, -83}, Tup{565946389, 63}, Tup{-1109464760, -120}, Tup{1554761898, -49}});
        int_testcase(std::vector<Tup>{Tup{-95091169, -43}, Tup{650953728, 6}, Tup{-1220137104, 36}, Tup{1269115893, 72}, Tup{198984674, 57}, Tup{-665295363, -72}, Tup{1612916346, 65}, Tup{-82856602, -80}, Tup{1948362890, 120}, Tup{-1447987892, -102}, Tup{-713529068, 118}});
        int_testcase(std::vector<Tup>{Tup{1941655301, -75}, Tup{347560580, 51}, Tup{-934516698, 48}, Tup{98113529, 48}, Tup{1721317945, 111}, Tup{1229648659, -126}, Tup{802619031, 27}, Tup{-630210223, -120}, Tup{1983762532, -21}, Tup{1442286224, 36}});
        int_testcase(std::vector<Tup>{Tup{-1115292695, -80}, Tup{765937679, -122}, Tup{-1652542996, -49}, Tup{-415549354, 23}, Tup{1456337309, -111}, Tup{-1850503798, -125}, Tup{759613881, 4}, Tup{-1487202398, 104}, Tup{-1315193816, 75}});
        int_testcase(std::vector<Tup>{Tup{2111661039, 44}, Tup{-1940459015, -15}, Tup{1878708800, -38}, Tup{255088247, 117}, Tup{-386577771, -53}, Tup{-40074155, 60}, Tup{747703301, 74}, Tup{-135707051, -69}});
        int_testcase(std::vector<Tup>{Tup{-1090486151, -101}, Tup{804170602, 63}, Tup{-760630059, -92}, Tup{-1373641142, -88}, Tup{1476461615, 105}, Tup{-439586875, -28}, Tup{-445367046, 67}});
        int_testcase(std::vector<Tup>{Tup{2105123795, 106}, Tup{-475482947, -93}, Tup{-1815580996, -38}, Tup{-1279346973, -59}, Tup{-1903275155, -108}, Tup{560132940, 28}});
        int_testcase(std::vector<Tup>{Tup{871727864, -107}, Tup{439443181, -71}, Tup{-965212908, -83}, Tup{1072169048, -50}, Tup{425401070, 96}});
        int_testcase(std::vector<Tup>{Tup{-672940306, -75}, Tup{1043091076, 58}, Tup{-1313080908, -126}, Tup{455476782, -54}});
        int_testcase(std::vector<Tup>{Tup{-1956610865, -86}, Tup{-1812089589, 44}, Tup{-100740144, -62}});
        int_testcase(std::vector<Tup>{Tup{7261224, 114}, Tup{731288953, 93}});
        int_testcase(std::vector<Tup>{Tup{194904560, -98}});
    }

    void tuple3_testcases()
    {
        using Tup =std::tuple<int16_t, int32_t>;
 
        int_testcase(std::vector<Tup>{Tup{30692, 2049866342}, Tup{9352, 1525660796}, Tup{17012, 810934610}, Tup{10178, -1503221461}, Tup{-5806, -159995939}, Tup{10541, -1371954202}, Tup{7578, 1516926606}, Tup{2849, -515686360}, Tup{-8955, 1071264676}, Tup{-11771, -658514340}, Tup{7526, 851116341}, Tup{32474, 1999462764}, Tup{-13129, -479417499}, Tup{773, -1919203220}, Tup{23069, 855524417}, Tup{-22741, 1259911012}});
        int_testcase(std::vector<Tup>{Tup{21689, -406851108}, Tup{25924, -728238657}, Tup{-27421, 160193627}, Tup{-19446, 698138001}, Tup{15810, 949904311}, Tup{9612, 1936508429}, Tup{20214, -1005312917}, Tup{-30639, 1312593643}, Tup{23957, -83869880}, Tup{9490, -2108587445}, Tup{14037, 598896524}, Tup{10889, -1726309438}, Tup{-14415, 4399700}, Tup{32666, 1853491986}, Tup{17129, -1868168395}});
        int_testcase(std::vector<Tup>{Tup{4330, -608326487}, Tup{-20787, -1431652228}, Tup{-1192, 275400067}, Tup{-19703, -510205253}, Tup{-19020, -86154341}, Tup{25447, -1851870151}, Tup{6814, -1972577114}, Tup{-21502, -425855971}, Tup{-9359, 2057605219}, Tup{14826, -700989203}, Tup{-28764, 1843544148}, Tup{21468, 512323136}, Tup{6015, -1538553957}, Tup{1118, -2113219620}});
        int_testcase(std::vector<Tup>{Tup{21480, -325589138}, Tup{9271, 1363515413}, Tup{29915, -1624803909}, Tup{-16630, -1121214117}, Tup{5004, 915932258}, Tup{-23770, -554747802}, Tup{19427, -1735857267}, Tup{25082, 1335023025}, Tup{14711, 878979091}, Tup{-3235, 914766375}, Tup{-8176, 1436852704}, Tup{-18713, -1146520800}, Tup{21111, -1324937110}});
        int_testcase(std::vector<Tup>{Tup{-32163, -92589226}, Tup{-16288, 487858812}, Tup{-27524, 712804471}, Tup{29412, -1131855156}, Tup{-20983, 1401454652}, Tup{-30596, 213893899}, Tup{-7864, -1578949216}, Tup{-7072, -1104380153}, Tup{10255, -535707704}, Tup{3916, 779429653}, Tup{-1534, 1088415752}, Tup{-21739, 803485049}});
        int_testcase(std::vector<Tup>{Tup{-8662, 1041921005}, Tup{-31319, 948551957}, Tup{15740, -21566895}, Tup{-11894, 210872472}, Tup{457, -1255215895}, Tup{10577, -1818725557}, Tup{1965, -992933125}, Tup{-25733, 864077040}, Tup{14420, 970393799}, Tup{-1833, -348862739}, Tup{-1, -2023701290}});
        int_testcase(std::vector<Tup>{Tup{-467, 302893809}, Tup{21009, 760744357}, Tup{-12358, 1978894173}, Tup{-23302, 1511705400}, Tup{-28120, 607353318}, Tup{21004, 780746081}, Tup{4620, 1442607763}, Tup{24539, 102801842}, Tup{1824, -2044955012}, Tup{-8462, -2051638966}});
        int_testcase(std::vector<Tup>{Tup{9001, -1971041445}, Tup{12248, 2121063351}, Tup{-7474, 43062430}, Tup{24308, 1660374882}, Tup{173, -1455721625}, Tup{23663, -1890155666}, Tup{17015, 470084113}, Tup{-10197, 1630995657}, Tup{-14736, 630446969}});
        int_testcase(std::vector<Tup>{Tup{18705, -1879834296}, Tup{21877, 189384202}, Tup{11137, 951492177}, Tup{16148, -933090050}, Tup{23354, 1951801122}, Tup{-7749, 399364327}, Tup{-30310, -1310234795}, Tup{-6818, 784096374}});
        int_testcase(std::vector<Tup>{Tup{8828, -1193169399}, Tup{-26678, 1274533761}, Tup{26535, -717114868}, Tup{-18681, -1369475768}, Tup{21014, 591910942}, Tup{19024, 982812841}, Tup{-6406, 1268152936}});
        int_testcase(std::vector<Tup>{Tup{-4121, 135321303}, Tup{27899, -941199648}, Tup{-6754, -1842461365}, Tup{11735, -1960824734}, Tup{17507, -327364846}, Tup{18786, -1606973139}});
        int_testcase(std::vector<Tup>{Tup{-31865, -1726899710}, Tup{-1527, 1844916458}, Tup{29060, -130660078}, Tup{-30909, 2008063048}, Tup{25521, -1222098799}});
        int_testcase(std::vector<Tup>{Tup{-29037, -395699701}, Tup{29002, 1626181703}, Tup{-8685, 2142673653}, Tup{-26776, 671976972}});
        int_testcase(std::vector<Tup>{Tup{-16372, -1580180067}, Tup{5342, -573580268}, Tup{-21241, 591098354}});
        int_testcase(std::vector<Tup>{Tup{-26151, -1115344011}, Tup{8614, -566869122}});
        int_testcase(std::vector<Tup>{Tup{-19916, -956432647}});  
    }

    void tuple4_testcases()
    {
        using Tup1 =std::pair<int16_t, int8_t>;
        using Tup =std::tuple<int32_t, Tup1>;

        int_testcase(std::vector<Tup>{Tup{-1672359173, Tup1{-1056, 113}}, Tup{44816375, Tup1{27142, -67}}, Tup{1040416301, Tup1{26191, 68}}, Tup{-1018424173, Tup1{4402, -47}}, Tup{1004670241, Tup1{23665, 72}}, Tup{-1712370769, Tup1{31466, 30}}, Tup{119972083, Tup1{26462, -107}}, Tup{-165192709, Tup1{11497, -81}}, Tup{238901480, Tup1{-12096, 66}}, Tup{-277281187, Tup1{-12693, -23}}, Tup{2106621430, Tup1{-14260, -43}}, Tup{-1286300048, Tup1{24236, 54}}, Tup{1537721526, Tup1{9787, 46}}, Tup{-1178979042, Tup1{13254, 124}}, Tup{1082059798, Tup1{23490, 65}}, Tup{1063174019, Tup1{11325, 103}}});
        int_testcase(std::vector<Tup>{Tup{1043207428, Tup1{-23472, -71}}, Tup{-1783534535, Tup1{16295, -3}}, Tup{8383082, Tup1{15398, 62}}, Tup{530852396, Tup1{-2000, -91}}, Tup{-622414331, Tup1{-7324, -4}}, Tup{1822583756, Tup1{10077, -58}}, Tup{-2093585180, Tup1{6979, 18}}, Tup{619928950, Tup1{-21120, 52}}, Tup{1537935199, Tup1{31068, -107}}, Tup{-626343760, Tup1{9563, -67}}, Tup{1799675200, Tup1{-15087, -99}}, Tup{66876997, Tup1{28993, -27}}, Tup{-1959047341, Tup1{12474, -35}}, Tup{899655048, Tup1{9667, -95}}, Tup{-2067234944, Tup1{-11003, 0}}});
        int_testcase(std::vector<Tup>{Tup{-1335295395, Tup1{-12026, 6}}, Tup{1616068745, Tup1{299, 3}}, Tup{-1397256235, Tup1{29004, -10}}, Tup{-1880323957, Tup1{-14583, -54}}, Tup{1545439786, Tup1{32744, 100}}, Tup{668919414, Tup1{-28280, 62}}, Tup{1132678731, Tup1{6653, -52}}, Tup{-192086960, Tup1{-7486, -98}}, Tup{-1628540474, Tup1{-10572, 123}}, Tup{1438945435, Tup1{8389, -62}}, Tup{1934257348, Tup1{-23208, -123}}, Tup{1640705145, Tup1{-3861, -27}}, Tup{474045788, Tup1{31881, -117}}, Tup{1253293487, Tup1{-6984, -48}}});
        int_testcase(std::vector<Tup>{Tup{-847452284, Tup1{17846, -31}}, Tup{-1616758204, Tup1{22706, -73}}, Tup{324227143, Tup1{5618, -102}}, Tup{450052914, Tup1{15207, 75}}, Tup{-1858195956, Tup1{5407, -7}}, Tup{1466845866, Tup1{5543, -24}}, Tup{-169027543, Tup1{15423, 111}}, Tup{55870960, Tup1{8929, -65}}, Tup{433983077, Tup1{-9465, -63}}, Tup{-341778639, Tup1{-27846, 1}}, Tup{-319274514, Tup1{12173, -96}}, Tup{1566861936, Tup1{-12057, -103}}, Tup{1472158285, Tup1{-24137, 65}}});
        int_testcase(std::vector<Tup>{Tup{-742255661, Tup1{-24295, 78}}, Tup{821419032, Tup1{-3095, 88}}, Tup{574258394, Tup1{286, -127}}, Tup{-669206202, Tup1{-30733, -69}}, Tup{-250829864, Tup1{8810, 99}}, Tup{-1014900290, Tup1{-12295, -50}}, Tup{-1483697630, Tup1{-16286, -100}}, Tup{-785070979, Tup1{23646, 84}}, Tup{-96116083, Tup1{17617, 95}}, Tup{751125855, Tup1{-1059, 47}}, Tup{1297251937, Tup1{-1267, 114}}, Tup{-644599572, Tup1{5212, -115}}});
        int_testcase(std::vector<Tup>{Tup{-1309925676, Tup1{20644, 17}}, Tup{-402565370, Tup1{9356, 42}}, Tup{1135754643, Tup1{16896, 107}}, Tup{538468115, Tup1{12702, 98}}, Tup{-505895643, Tup1{-20450, -66}}, Tup{551714957, Tup1{19183, 67}}, Tup{-173049882, Tup1{6481, 25}}, Tup{-929601430, Tup1{-12996, -61}}, Tup{-245957396, Tup1{-16717, 5}}, Tup{-1734105241, Tup1{14656, -102}}, Tup{1072206611, Tup1{-8765, -84}}});
        int_testcase(std::vector<Tup>{Tup{-472638537, Tup1{-26456, -85}}, Tup{1849319600, Tup1{-21377, -116}}, Tup{1539654733, Tup1{-12566, 29}}, Tup{-269085998, Tup1{-27709, 27}}, Tup{-169193571, Tup1{-31324, -78}}, Tup{-1110910070, Tup1{30940, 106}}, Tup{1107443304, Tup1{-21400, 118}}, Tup{97511720, Tup1{-20375, 21}}, Tup{-373804428, Tup1{-7626, 6}}, Tup{1093498991, Tup1{16529, 90}}});
        int_testcase(std::vector<Tup>{Tup{1297424619, Tup1{25303, -65}}, Tup{603010871, Tup1{-5563, 92}}, Tup{-1391721650, Tup1{-18881, -99}}, Tup{-196682410, Tup1{-6027, 82}}, Tup{61908777, Tup1{20930, -59}}, Tup{1526019177, Tup1{7783, 15}}, Tup{1544975325, Tup1{-18974, 122}}, Tup{-342975944, Tup1{12872, 25}}, Tup{-763548709, Tup1{6812, -8}}});
        int_testcase(std::vector<Tup>{Tup{157203325, Tup1{-20997, 78}}, Tup{-947803926, Tup1{-12022, -2}}, Tup{-1455109828, Tup1{23297, -82}}, Tup{1523847571, Tup1{24957, -107}}, Tup{1395327342, Tup1{-21540, -2}}, Tup{1324813, Tup1{7722, -40}}, Tup{696502013, Tup1{19364, 48}}, Tup{-105097128, Tup1{-11742, 54}}});
        int_testcase(std::vector<Tup>{Tup{-489778173, Tup1{29583, 74}}, Tup{1512741169, Tup1{8494, -16}}, Tup{-1306536169, Tup1{4552, -103}}, Tup{1975280029, Tup1{-28794, -12}}, Tup{1225259108, Tup1{-27696, -74}}, Tup{235914809, Tup1{3705, 26}}, Tup{-1083960304, Tup1{-14653, -34}}});
        int_testcase(std::vector<Tup>{Tup{495505003, Tup1{23361, -93}}, Tup{-101470134, Tup1{-32566, 34}}, Tup{-1763352261, Tup1{9696, 45}}, Tup{1551419692, Tup1{7901, 76}}, Tup{910484501, Tup1{92, 103}}, Tup{1257488363, Tup1{12350, -47}}});
        int_testcase(std::vector<Tup>{Tup{-1656887504, Tup1{-28870, -18}}, Tup{1533011710, Tup1{-21476, 46}}, Tup{1359133966, Tup1{11080, 46}}, Tup{789981882, Tup1{15518, 21}}, Tup{1416444335, Tup1{9421, 69}}});
        int_testcase(std::vector<Tup>{Tup{-570404961, Tup1{32389, -128}}, Tup{-349338699, Tup1{-8234, -14}}, Tup{-1462472605, Tup1{-29071, 59}}, Tup{-342843499, Tup1{-9529, -117}}});
        int_testcase(std::vector<Tup>{Tup{-2056971140, Tup1{8519, -47}}, Tup{-1458848984, Tup1{-5259, -89}}, Tup{-1763191601, Tup1{-26259, 83}}});
        int_testcase(std::vector<Tup>{Tup{550155976, Tup1{16277, 50}}, Tup{1481363541, Tup1{13289, -8}}});
        int_testcase(std::vector<Tup>{Tup{346237723, Tup1{-12470, -120}}});
    }

    void tuple5_testcases()
    {
        using Tup1 =std::pair<int8_t, int16_t>;
        using Tup =std::tuple<int32_t,Tup1>;

        int_testcase(std::vector<Tup>{Tup{-68516209, Tup1{34, -2473}}, Tup{-1880570288, Tup1{-100, 4095}}, Tup{-332800702, Tup1{72, 9112}}, Tup{1754517861, Tup1{-92, -15123}}, Tup{-222687077, Tup1{72, -25381}}, Tup{-1274008015, Tup1{-95, 29324}}, Tup{1724836035, Tup1{1, -26023}}, Tup{828728727, Tup1{53, 19981}}, Tup{47002844, Tup1{-40, -3466}}, Tup{-1950983075, Tup1{-27, -28544}}, Tup{71381664, Tup1{26, -23665}}, Tup{-1302641485, Tup1{57, 204}}, Tup{-314082461, Tup1{-123, -24345}}, Tup{1351572529, Tup1{-68, -4343}}, Tup{335267009, Tup1{115, -17961}}, Tup{-182507810, Tup1{70, 17710}}});
        int_testcase(std::vector<Tup>{Tup{-281887550, Tup1{-104, 3715}}, Tup{1215244834, Tup1{-68, 449}}, Tup{-2029906364, Tup1{-101, -13376}}, Tup{-1182335105, Tup1{-26, -9700}}, Tup{-1327346941, Tup1{-127, -12437}}, Tup{-1909255116, Tup1{69, 11485}}, Tup{487774014, Tup1{-80, 23020}}, Tup{-704545826, Tup1{90, 15943}}, Tup{1186742193, Tup1{-89, 7872}}, Tup{177427629, Tup1{65, 20910}}, Tup{381983175, Tup1{17, 26778}}, Tup{-224400988, Tup1{112, -3272}}, Tup{-1466332815, Tup1{-29, -9139}}, Tup{-359273407, Tup1{4, 16909}}, Tup{-2081325713, Tup1{4, 7414}}});
        int_testcase(std::vector<Tup>{Tup{-772905954, Tup1{-114, 834}}, Tup{-1407112092, Tup1{-3, -11384}}, Tup{1276734138, Tup1{23, 15981}}, Tup{1814489970, Tup1{-121, 24144}}, Tup{-464967281, Tup1{0, -2399}}, Tup{1499229369, Tup1{-38, -29529}}, Tup{-2074653398, Tup1{-13, 16512}}, Tup{1968529729, Tup1{80, 25843}}, Tup{295743591, Tup1{15, -1390}}, Tup{1888796287, Tup1{117, -29668}}, Tup{898563521, Tup1{-125, -7660}}, Tup{385525678, Tup1{-2, 9750}}, Tup{79433816, Tup1{76, -7024}}, Tup{-1472297053, Tup1{-12, -26247}}});
        int_testcase(std::vector<Tup>{Tup{-1992984396, Tup1{100, -29512}}, Tup{836404158, Tup1{28, 31411}}, Tup{-1459109384, Tup1{-116, -17528}}, Tup{1121132211, Tup1{-2, 29912}}, Tup{732139116, Tup1{-27, -20588}}, Tup{744092741, Tup1{-84, -28912}}, Tup{-2054487766, Tup1{-97, 30279}}, Tup{-692294692, Tup1{-88, 32662}}, Tup{1537214949, Tup1{-90, -31839}}, Tup{-1483895103, Tup1{-85, 24987}}, Tup{1605176162, Tup1{65, -23684}}, Tup{306890484, Tup1{-97, -26933}}, Tup{-637200747, Tup1{-79, 23471}}});
        int_testcase(std::vector<Tup>{Tup{2056361018, Tup1{34, 29044}}, Tup{502091544, Tup1{7, -15879}}, Tup{1414184098, Tup1{-116, -20824}}, Tup{-1829028734, Tup1{60, -16452}}, Tup{-66685753, Tup1{127, -11064}}, Tup{335887792, Tup1{-75, -15235}}, Tup{1754886694, Tup1{-12, -19414}}, Tup{-535680655, Tup1{-104, 23777}}, Tup{1294795279, Tup1{-114, -15926}}, Tup{311997299, Tup1{-85, -5884}}, Tup{-1528485497, Tup1{-19, -25815}}, Tup{1664998309, Tup1{22, 6277}}});
        int_testcase(std::vector<Tup>{Tup{1679292563, Tup1{15, -28232}}, Tup{1105542158, Tup1{112, -13550}}, Tup{456256137, Tup1{63, -23064}}, Tup{-987132891, Tup1{-28, 32528}}, Tup{-1084278985, Tup1{-70, 14865}}, Tup{1265576656, Tup1{84, -31323}}, Tup{-1261300282, Tup1{5, 765}}, Tup{-691097254, Tup1{67, -2163}}, Tup{-844538232, Tup1{57, 8558}}, Tup{1054886563, Tup1{106, 32426}}, Tup{-582748506, Tup1{-69, -31575}}});
        int_testcase(std::vector<Tup>{Tup{-1442890690, Tup1{52, -7604}}, Tup{322265205, Tup1{70, 24892}}, Tup{-1696949397, Tup1{-67, 22157}}, Tup{-2076682635, Tup1{-57, 15621}}, Tup{-1615851502, Tup1{-98, 24673}}, Tup{-97341671, Tup1{-43, 853}}, Tup{1216202583, Tup1{-122, -29813}}, Tup{-948702961, Tup1{-38, -3514}}, Tup{-564640036, Tup1{-50, 22664}}, Tup{1850499856, Tup1{-111, -6023}}});
        int_testcase(std::vector<Tup>{Tup{666887483, Tup1{-126, 17974}}, Tup{-810790353, Tup1{-118, 30965}}, Tup{-687591114, Tup1{29, 24620}}, Tup{660518512, Tup1{45, 20293}}, Tup{-1678596728, Tup1{-65, 17836}}, Tup{-208921673, Tup1{112, -27643}}, Tup{-1236914665, Tup1{-36, 22362}}, Tup{-648270197, Tup1{-5, -23704}}, Tup{-397290357, Tup1{53, -333}}});
        int_testcase(std::vector<Tup>{Tup{556900066, Tup1{-55, -6545}}, Tup{669992733, Tup1{-107, 27334}}, Tup{1584830837, Tup1{-72, -14793}}, Tup{-1032355201, Tup1{-94, -22732}}, Tup{-1855042005, Tup1{-72, 1262}}, Tup{1373347739, Tup1{-70, 30332}}, Tup{-1743165144, Tup1{20, -21537}}, Tup{-58434443, Tup1{-51, -22733}}});
        int_testcase(std::vector<Tup>{Tup{541198388, Tup1{73, -30653}}, Tup{2019130600, Tup1{-48, -27250}}, Tup{1448035153, Tup1{-8, 7725}}, Tup{-433767764, Tup1{-70, -16522}}, Tup{-171989900, Tup1{-74, 13879}}, Tup{-1552845062, Tup1{-95, 2454}}, Tup{-1971651757, Tup1{101, -27918}}});
        int_testcase(std::vector<Tup>{Tup{1283401294, Tup1{42, 22947}}, Tup{1854148325, Tup1{62, -28112}}, Tup{-75195014, Tup1{14, -6557}}, Tup{-821987937, Tup1{63, 19546}}, Tup{201831823, Tup1{83, 27237}}, Tup{2128258805, Tup1{-86, -17567}}});
        int_testcase(std::vector<Tup>{Tup{-515830288, Tup1{43, -1951}}, Tup{1265705509, Tup1{-33, -4822}}, Tup{533883644, Tup1{26, 14972}}, Tup{-665278225, Tup1{-53, -8878}}, Tup{586728587, Tup1{-49, 17672}}});
        int_testcase(std::vector<Tup>{Tup{-1866471330, Tup1{124, 16110}}, Tup{137440761, Tup1{-121, 5174}}, Tup{-1457562761, Tup1{-33, 5477}}, Tup{1035360302, Tup1{-19, -31067}}});
        int_testcase(std::vector<Tup>{Tup{-197080740, Tup1{65, 28310}}, Tup{-1881908053, Tup1{112, 20412}}, Tup{1008229154, Tup1{79, 2046}}});
        int_testcase(std::vector<Tup>{Tup{76956955, Tup1{-79, -10904}}, Tup{37521693, Tup1{120, 19333}}});
        int_testcase(std::vector<Tup>{Tup{389561179, Tup1{22, -27613}}});
    }

    void tuple6_testcases()
    {
        using Tup1 =std::pair<int16_t, int32_t>;
        using Tup =std::tuple<int8_t, Tup1>;

        int_testcase(std::vector<Tup>{Tup{-61, Tup1{17413, -1624525207}}, Tup{35, Tup1{28316, 102707899}}, Tup{78, Tup1{-18180, -1246822426}}, Tup{109, Tup1{-22684, -1603667865}}, Tup{66, Tup1{-32408, -460131094}}, Tup{-104, Tup1{-5178, 352775266}}, Tup{-28, Tup1{4456, -407993124}}, Tup{56, Tup1{14631, 237672154}}, Tup{115, Tup1{-20255, 2102029672}}, Tup{-100, Tup1{-32440, -1651300049}}, Tup{94, Tup1{10080, -2022816173}}, Tup{-61, Tup1{-11079, 1081772865}}, Tup{111, Tup1{-32537, 1921472998}}, Tup{-14, Tup1{9772, -578255970}}, Tup{-111, Tup1{-23043, 315416662}}, Tup{-31, Tup1{-1838, -1949874099}}});
        int_testcase(std::vector<Tup>{Tup{124, Tup1{18800, 355822061}}, Tup{-13, Tup1{-12228, 701600406}}, Tup{82, Tup1{20674, -7342612}}, Tup{45, Tup1{-3683, -727831626}}, Tup{97, Tup1{22030, -654215862}}, Tup{-49, Tup1{-1418, -287003349}}, Tup{47, Tup1{26847, 540043834}}, Tup{67, Tup1{-28306, 1695043267}}, Tup{83, Tup1{-7105, -1032847951}}, Tup{63, Tup1{-19292, -1900715146}}, Tup{-8, Tup1{-31253, 1909865946}}, Tup{63, Tup1{20710, -1927132604}}, Tup{111, Tup1{9747, -752483685}}, Tup{55, Tup1{-29701, 72463215}}, Tup{102, Tup1{-8199, -305677068}}});
        int_testcase(std::vector<Tup>{Tup{-70, Tup1{18430, 71623713}}, Tup{-54, Tup1{17059, -39152990}}, Tup{66, Tup1{-22716, 541071592}}, Tup{120, Tup1{-28096, 1995707145}}, Tup{-21, Tup1{-2799, -1455932988}}, Tup{-59, Tup1{11039, -1832140192}}, Tup{-49, Tup1{30109, 1866632581}}, Tup{-79, Tup1{3268, -1256507950}}, Tup{-64, Tup1{19077, -805461978}}, Tup{-79, Tup1{24532, 1519238720}}, Tup{56, Tup1{18517, -299057471}}, Tup{68, Tup1{24605, -1173465452}}, Tup{99, Tup1{-30495, 1214802258}}, Tup{-9, Tup1{-229, -529211252}}});
        int_testcase(std::vector<Tup>{Tup{121, Tup1{26584, 1316354472}}, Tup{96, Tup1{-26379, -1366492129}}, Tup{72, Tup1{3730, 251573872}}, Tup{-80, Tup1{24190, -149473087}}, Tup{-52, Tup1{-29268, -175908365}}, Tup{-14, Tup1{-3148, -585943451}}, Tup{111, Tup1{10953, 131356568}}, Tup{-24, Tup1{-17805, -1152525964}}, Tup{-94, Tup1{16575, 1252915457}}, Tup{-123, Tup1{20729, 151803216}}, Tup{99, Tup1{30232, -1966778619}}, Tup{3, Tup1{15270, -300024235}}, Tup{0, Tup1{1760, -1620597807}}});
        int_testcase(std::vector<Tup>{Tup{-21, Tup1{-10887, 225163966}}, Tup{58, Tup1{-7990, -881146517}}, Tup{-58, Tup1{-19652, 1820703900}}, Tup{49, Tup1{20308, -395491247}}, Tup{36, Tup1{-1375, 1168344937}}, Tup{116, Tup1{-1025, 624625962}}, Tup{123, Tup1{5878, -2098033951}}, Tup{42, Tup1{6616, -913918625}}, Tup{-125, Tup1{13321, 2038347981}}, Tup{87, Tup1{24536, -1614423621}}, Tup{-64, Tup1{2258, -1468365846}}, Tup{82, Tup1{3550, 251126658}}});
        int_testcase(std::vector<Tup>{Tup{4, Tup1{1714, 2050937625}}, Tup{120, Tup1{-1960, -176516412}}, Tup{73, Tup1{11088, 18195946}}, Tup{-39, Tup1{-12386, -5024341}}, Tup{-122, Tup1{-11824, 1921330550}}, Tup{-74, Tup1{7821, 1557350270}}, Tup{42, Tup1{-22342, 1872552811}}, Tup{39, Tup1{-2412, 1917968331}}, Tup{78, Tup1{-23401, -46552568}}, Tup{106, Tup1{-27161, 1519464863}}, Tup{43, Tup1{26949, 1966011112}}});
        int_testcase(std::vector<Tup>{Tup{121, Tup1{22687, -1526477249}}, Tup{69, Tup1{-22475, 634029878}}, Tup{-36, Tup1{-21909, -451414649}}, Tup{46, Tup1{-24383, -803905506}}, Tup{16, Tup1{7930, -954861854}}, Tup{-40, Tup1{27493, 637830073}}, Tup{-109, Tup1{-13397, 978869727}}, Tup{60, Tup1{28901, -1699045261}}, Tup{-4, Tup1{3163, 1143803275}}, Tup{-9, Tup1{673, 1645901206}}});
        int_testcase(std::vector<Tup>{Tup{-44, Tup1{-21746, 818487596}}, Tup{64, Tup1{-633, -1795226578}}, Tup{8, Tup1{-28425, -1319995799}}, Tup{39, Tup1{-6047, 1540143428}}, Tup{-75, Tup1{5114, -594741965}}, Tup{27, Tup1{-6008, -53108767}}, Tup{74, Tup1{-19504, -1745223160}}, Tup{-48, Tup1{-16008, -165337503}}, Tup{-11, Tup1{10840, -1716380578}}});
        int_testcase(std::vector<Tup>{Tup{-46, Tup1{643, 1410470299}}, Tup{-29, Tup1{22717, 2055817459}}, Tup{79, Tup1{-16531, 1316822838}}, Tup{-68, Tup1{-20122, 77147480}}, Tup{76, Tup1{-30392, 487749921}}, Tup{35, Tup1{7333, -370190333}}, Tup{-104, Tup1{11823, -1630163832}}, Tup{-17, Tup1{-17483, -518551365}}});
        int_testcase(std::vector<Tup>{Tup{-23, Tup1{-22948, -612436878}}, Tup{105, Tup1{27315, 898386151}}, Tup{-7, Tup1{15016, -664608104}}, Tup{57, Tup1{5040, -2131092779}}, Tup{7, Tup1{-12069, -1098192453}}, Tup{114, Tup1{-14652, -1863271053}}, Tup{-105, Tup1{-28147, 379068219}}});
        int_testcase(std::vector<Tup>{Tup{-6, Tup1{3465, 1518468852}}, Tup{95, Tup1{15533, 198563763}}, Tup{-35, Tup1{13352, 2007169519}}, Tup{-86, Tup1{-12564, -419208970}}, Tup{-64, Tup1{32510, -1350955765}}, Tup{-47, Tup1{-5121, 989202441}}});
        int_testcase(std::vector<Tup>{Tup{-66, Tup1{10446, -470707634}}, Tup{2, Tup1{29531, 277954471}}, Tup{-121, Tup1{-11020, 770325276}}, Tup{-84, Tup1{31778, 2024891016}}, Tup{96, Tup1{18605, -769764731}}});
        int_testcase(std::vector<Tup>{Tup{113, Tup1{13773, -1469110246}}, Tup{103, Tup1{909, 1443687692}}, Tup{-15, Tup1{-29540, -397006309}}, Tup{-56, Tup1{16178, -1840196327}}});
        int_testcase(std::vector<Tup>{Tup{-75, Tup1{-17928, 401983368}}, Tup{98, Tup1{-31058, 926888318}}, Tup{-34, Tup1{-10865, 123855929}}});
        int_testcase(std::vector<Tup>{Tup{41, Tup1{11009, -113851788}}, Tup{-30, Tup1{27894, -1003432814}}});
        int_testcase(std::vector<Tup>{Tup{90, Tup1{-27924, -1404532711}}});
    }

    void tuple7_testcases()
    {
        using Tup1 =std::pair<int32_t, int16_t>;
        using Tup =std::tuple<int8_t, Tup1>;

        int_testcase(std::vector<Tup>{Tup{82, Tup1{-1521250841, 8261}}, Tup{-9, Tup1{-782619114, -14787}}, Tup{14, Tup1{-205344574, -31945}}, Tup{-50, Tup1{665513844, -3181}}, Tup{107, Tup1{-111254781, -20765}}, Tup{20, Tup1{1918541230, -30303}}, Tup{37, Tup1{-849382585, -14716}}, Tup{-40, Tup1{-1938031009, 12790}}, Tup{89, Tup1{-1296684139, -26121}}, Tup{-1, Tup1{-2047314039, 28625}}, Tup{61, Tup1{-895186301, -22402}}, Tup{-2, Tup1{-1382437165, 2219}}, Tup{-81, Tup1{772505587, 31694}}, Tup{7, Tup1{-30777881, 10708}}, Tup{69, Tup1{-780193668, 28050}}, Tup{126, Tup1{-1156394723, -13703}}});
        int_testcase(std::vector<Tup>{Tup{23, Tup1{7493490, -1447}}, Tup{117, Tup1{413157765, 12191}}, Tup{106, Tup1{-1410677319, 28633}}, Tup{-124, Tup1{-2042903965, 1374}}, Tup{65, Tup1{2040911566, -27220}}, Tup{-104, Tup1{-2051828434, -11431}}, Tup{-103, Tup1{-1631484985, -13957}}, Tup{-37, Tup1{1664740114, 17829}}, Tup{106, Tup1{358872230, -16330}}, Tup{-21, Tup1{-1717996260, -29311}}, Tup{-7, Tup1{-1680172038, -11674}}, Tup{-53, Tup1{-1954459623, 10849}}, Tup{-64, Tup1{-3252477, -1827}}, Tup{-83, Tup1{1017345516, 24493}}, Tup{105, Tup1{138202026, 11828}}});
        int_testcase(std::vector<Tup>{Tup{102, Tup1{748957494, -2355}}, Tup{-68, Tup1{2141704865, 14474}}, Tup{-125, Tup1{-549732637, -12150}}, Tup{28, Tup1{2082585390, -7972}}, Tup{30, Tup1{-807544526, -31120}}, Tup{37, Tup1{1778401209, 16095}}, Tup{-85, Tup1{1167869233, 30725}}, Tup{121, Tup1{-2080427453, 10677}}, Tup{65, Tup1{-1923140494, 21495}}, Tup{-18, Tup1{-447430948, 6540}}, Tup{-12, Tup1{-406688882, 11127}}, Tup{-21, Tup1{837447637, -9779}}, Tup{32, Tup1{1472123475, -2141}}, Tup{17, Tup1{-1300217086, -8123}}});
        int_testcase(std::vector<Tup>{Tup{-49, Tup1{1269331290, -17781}}, Tup{95, Tup1{-395090936, 22532}}, Tup{24, Tup1{1905624999, 9685}}, Tup{-2, Tup1{1244907688, 28811}}, Tup{113, Tup1{52148215, -10644}}, Tup{75, Tup1{1086746516, 3422}}, Tup{-4, Tup1{1263933478, -23340}}, Tup{21, Tup1{1638943083, -28713}}, Tup{104, Tup1{-1108284436, 28588}}, Tup{18, Tup1{2022394401, -281}}, Tup{-78, Tup1{422383077, 20838}}, Tup{19, Tup1{-159922892, -19342}}, Tup{-70, Tup1{-991715005, -20647}}});
        int_testcase(std::vector<Tup>{Tup{120, Tup1{-502790513, -13299}}, Tup{63, Tup1{-2003753016, 79}}, Tup{-97, Tup1{-1691021886, 24453}}, Tup{-116, Tup1{-1369505011, -13384}}, Tup{-35, Tup1{-1844473841, 31467}}, Tup{-7, Tup1{-1386700133, 13858}}, Tup{-119, Tup1{-1164962037, 18797}}, Tup{-35, Tup1{-1808338990, 27327}}, Tup{48, Tup1{197953642, 24070}}, Tup{79, Tup1{588714886, 1612}}, Tup{33, Tup1{-658242901, -11427}}, Tup{-48, Tup1{-451543653, 20964}}});
        int_testcase(std::vector<Tup>{Tup{-108, Tup1{-2095264458, 21202}}, Tup{-120, Tup1{1412427897, -17654}}, Tup{-110, Tup1{831489564, 4623}}, Tup{90, Tup1{628217989, 8815}}, Tup{-22, Tup1{1590994667, 15713}}, Tup{-97, Tup1{286434749, 8279}}, Tup{-77, Tup1{-110109871, -27023}}, Tup{49, Tup1{-1897783827, -26621}}, Tup{74, Tup1{-736525934, -31366}}, Tup{19, Tup1{2044176862, -213}}, Tup{-41, Tup1{308052618, 5214}}});
        int_testcase(std::vector<Tup>{Tup{-118, Tup1{1500491321, -8673}}, Tup{-35, Tup1{-436737254, -19597}}, Tup{108, Tup1{-193023770, -27263}}, Tup{114, Tup1{1395500916, -30862}}, Tup{-3, Tup1{452945912, -1970}}, Tup{-66, Tup1{1620472916, -12596}}, Tup{62, Tup1{1361472799, -2731}}, Tup{52, Tup1{847490196, 32493}}, Tup{-114, Tup1{-1828679114, 14582}}, Tup{-20, Tup1{703635159, 9267}}});
        int_testcase(std::vector<Tup>{Tup{-28, Tup1{1468353664, -21890}}, Tup{19, Tup1{31077468, 28772}}, Tup{-125, Tup1{865701139, -6739}}, Tup{6, Tup1{636531148, 566}}, Tup{83, Tup1{902032355, 2652}}, Tup{-92, Tup1{23442124, -7923}}, Tup{119, Tup1{1950418955, -6069}}, Tup{7, Tup1{801523177, 31290}}, Tup{102, Tup1{328235196, 18879}}});
        int_testcase(std::vector<Tup>{Tup{-21, Tup1{-1567799465, -13810}}, Tup{-52, Tup1{2072302058, -21366}}, Tup{43, Tup1{318240851, 14716}}, Tup{-10, Tup1{293994327, 25224}}, Tup{-19, Tup1{-2037292880, 3679}}, Tup{-123, Tup1{394553700, -17951}}, Tup{77, Tup1{-91450737, 3074}}, Tup{-96, Tup1{-2043844025, -17343}}});
        int_testcase(std::vector<Tup>{Tup{-10, Tup1{2077496995, 26141}}, Tup{42, Tup1{22907918, -3094}}, Tup{11, Tup1{-286976627, -2382}}, Tup{112, Tup1{-1040581615, -23439}}, Tup{-65, Tup1{1237659142, -24091}}, Tup{-24, Tup1{-656574282, 24438}}, Tup{-43, Tup1{251211441, -20863}}});
        int_testcase(std::vector<Tup>{Tup{-99, Tup1{-1336292158, 23006}}, Tup{-44, Tup1{-1283250422, -431}}, Tup{80, Tup1{2039758930, -23206}}, Tup{106, Tup1{-1201786921, 27902}}, Tup{-54, Tup1{-103619151, 9248}}, Tup{56, Tup1{-1660022381, -8132}}});
        int_testcase(std::vector<Tup>{Tup{-118, Tup1{1837182583, -28679}}, Tup{-14, Tup1{178587256, 3066}}, Tup{-20, Tup1{944753935, 15933}}, Tup{76, Tup1{-347567658, -20544}}, Tup{70, Tup1{1626505708, -4510}}});
        int_testcase(std::vector<Tup>{Tup{33, Tup1{-18980737, 7595}}, Tup{14, Tup1{1256426935, 5386}}, Tup{47, Tup1{958310946, -29773}}, Tup{14, Tup1{1369468539, 1353}}});
        int_testcase(std::vector<Tup>{Tup{81, Tup1{-1463199089, 24522}}, Tup{-63, Tup1{1627479411, 28803}}, Tup{-69, Tup1{2079175703, -28909}}});
        int_testcase(std::vector<Tup>{Tup{-67, Tup1{-1203859551, -20702}}, Tup{24, Tup1{-1755642409, -23825}}});
        int_testcase(std::vector<Tup>{Tup{88, Tup1{1240691582, -31479}}});
    }

    void tuple8_testcases()
    {}

    //TODO
    //16, 8, 32
    //16, 32, 8

    //A, B, C, D in {int8_t, int16_t, int32_t, int64_t, uint8_t, uint16_t, uint32_t, uint64_t>
    //the fuzzer determinates the A, B, C, D (16^4 = 2^16 => 16 bits code to choose the types), we test all the structures below

    //tuple<A, B, C, D>
    //tuple<A, B, C, tuple<D>>
    //tuple<A, B, tuple<C, D>>
    //tuple<A, tuple<B, C, D>>
    //tuple<A, tuple<B, tuple<C, D>>>
    //tuple<A, tuple<B, C>, D>
    //tuple<tuple<A, B>, C, D>
    //tuple<tuple<A, B, C>, D>
    //tuple<tuple<A, B>, tuple<C, D>>

/*
    void tuple4_testcases()
    {
        using type1 = int32_t;
        using type2 = int16_t;
        using type3 = int8_t;

        using Tup1 =std::pair<type2, type3>;
        using Tup =std::tuple<type1, Tup1>;

        std::random_device rd;  //Will be used to obtain a seed for the random number engine
        std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()

        std::uniform_int_distribution<type1> dis1(std::numeric_limits<type1>::min(), std::numeric_limits<type1>::max());
        std::uniform_int_distribution<type2> dis2(std::numeric_limits<type2>::min(), std::numeric_limits<type2>::max());  
        std::uniform_int_distribution<type3> dis3(std::numeric_limits<type3>::min(), std::numeric_limits<type3>::max());  

        for(int j = 16; j >= 1; --j)
        {
            std::cout << "int_testcase(std::vector<Tup>{";
            for(int i = 0; i < j-1; ++i)
            {
                std::cout << "Tup{" << (int64_t)dis1(gen) << ", Tup1{" << (int64_t)dis2(gen) << ", " << (int64_t)dis3(gen) << "}}, ";
            }
            std::cout << "Tup{" << (int64_t)dis1(gen) << ", Tup1{" << (int64_t)dis2(gen) << ", " << (int64_t)dis3(gen) << "}}});\n";
        }
    }

    void tuple3_testcases()
    {
        using type1 = int32_t;
        using type2 = int16_t;

        std::random_device rd;  //Will be used to obtain a seed for the random number engine
        std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()

        std::uniform_int_distribution<type1> dis1(std::numeric_limits<type1>::min(), std::numeric_limits<type1>::max());
        std::uniform_int_distribution<type2> dis2(std::numeric_limits<type2>::min(), std::numeric_limits<type2>::max());  
 
        for(int j = 16; j >= 1; --j)
        {
            std::cout << "int_testcase(std::vector<Tup>{";
            for(int i = 0; i < j-1; ++i)
            {
                std::cout << "Tup{" << (int64_t)dis1(gen) << ", " << (int64_t)dis2(gen) << "}, ";
            }
            std::cout << "Tup{" << (int64_t)dis1(gen) << ", " << (int64_t)dis2(gen) << "}});\n";
        }

        using Tup =std::tuple<type1, type2>;
    }


*/

    void types_testcases()
    {
        constexpr auto value = details::radix_helper<unsigned int>{}(5);
        static_assert(value == 5);

        constexpr auto value2 = details::radix_helper<int>{}(5);

        static_assert(value2 == ((1u<<31)|5u));

        //constexpr 
        auto value3 = details::radix_helper<std::tuple<int, double>>{}(std::tuple{1, 7.0});

        static_assert(std::is_same_v<decltype(value3), std::tuple<unsigned int, uint64_t>>);
        assert(std::get<0>(value3) == 2147483649 && std::get<1>(value3) == 13842939354630062080ull);

        //can't be constexpr at the moment
        auto value4 = details::radix_helper<float>{}(1.0f);  

        static_assert(std::is_same_v<decltype(value4), uint32_t>);
        assert(value4 == 3212836864);

        auto value5 = details::radix_helper<std::tuple<int, std::tuple<int, double>>>{}(std::tuple{1, std::tuple{4, 7.0}});
        auto value6 = details::flatten<decltype(value5)>{}(value5);

        static_assert(std::is_same_v<decltype(value6), std::tuple<unsigned int, unsigned int, uint64_t>>);  
        assert(std::get<0>(value6) == 2147483649 && std::get<1>(value6) == 2147483652 && std::get<2>(value6) == 13842939354630062080ull);

        auto value7 = details::radix_helper<std::tuple<int, std::pair<int, double>, int>>{}(std::tuple{1, std::pair{4, 7.0}, 0});
        auto value8 = details::flatten<decltype(value7)>{}(value7);

        static_assert(std::is_same_v<decltype(value8), std::tuple<unsigned int, unsigned int, uint64_t, unsigned int>>);
        assert(std::get<0>(value8) == 2147483649 && std::get<1>(value8) == 2147483652 && std::get<2>(value8) == 13842939354630062080ull && std::get<3>(value8) == 2147483648);

        auto value9 = details::radix_helper<std::tuple<std::pair<int, double>, int, int>>{}(std::tuple{std::pair{4, 7.0}, 0, 1});
        auto value10 = details::flatten<decltype(value9)>{}(value9);

        static_assert(std::is_same_v<decltype(value10), std::tuple<unsigned int, uint64_t, unsigned int, unsigned int>>);
        assert(std::get<0>(value10) == 2147483652 && std::get<1>(value10) == 13842939354630062080ull && std::get<2>(value10) == 2147483648 && std::get<3>(value10) == 2147483649);

        auto value11 = details::flatten<int>{}(0);
        static_assert(std::is_same_v<decltype(value11), std::tuple<int>>);

        static_assert(details::buckets_count<8, int>{}() == 4);
        static_assert(details::buckets_count<11, int>{}() == 3);
        static_assert(details::buckets_count<8, std::tuple<int, uint64_t>>{}() == 12);
        static_assert(details::buckets_count<8, std::pair<int, int>>{}() == 8);
    }

    void general_testcases()
    {
        std::vector<int> v{-1356569119, -422563130,-1352029410,395770343,1785423163,-1351687749,-559142332,-140471621,-313712167,627271371,-1604412862,763551832,440576809,1616784356,-1399382555};
        auto vc = v;
 
        auto funct = [](int i)
                        {
                            //signed overflow is undefined behaviour...
                            int64_t tmp = i-5;
                            tmp *= tmp;

                            constexpr int64_t mini{std::numeric_limits<int32_t>::min()};
                            constexpr int64_t maxi{std::numeric_limits<int32_t>::max()}; 

                            return static_cast<int>(std::min(maxi, std::max(mini, tmp)));
                        };

        radix_sort(v, 
            funct);
   
        std::sort(vc.begin(), vc.end(), 
            [&funct](int lhs, int rhs)
            {
                return funct(lhs) < funct(rhs);
            });

        assert(std::equal(v.begin(), v.end(), vc.begin()));

        std::vector<float> v2{1.f, 4.f, 2.f, 5.f, 7.f, 11.f, 4.f, 33.f};
        auto vc2 = v2;

        radix_sort(v2);
        std::sort(vc2.begin(), vc2.end());

        assert(std::equal(v2.begin(), v2.end(), vc2.begin()));

        //TODO: make compiler=clang sanitize=msan is not happy
        struct Test{
            int key_;
            std::string val_;
        };

        std::list<Test> l{{7, "a"}, {1, "b"}, {3, "c"}, {2, "ff"}};
        auto lc = l;

        radix_sort(l, 
            [](const Test& t)
            {
                return t.key_;
            });

        lc.sort(
            [](const Test& lhs, const Test& rhs)
            {
                return lhs.key_ < rhs.key_;
            });

        assert(std::equal(lc.begin(), lc.end(), l.begin(), 
            [](const Test& lhs, const Test& rhs)
            {
                return lhs.key_ == rhs.key_;   
            }));
 
        std::list<int> l2;
        std::vector<int> v4{1, -4, 2, 5, 7, 11, 4, -33};
        auto vc4 = v4;

        radix_sort(v4, std::back_inserter(l2));
        std::sort(vc4.begin(), vc4.end());

        assert(std::equal(l2.begin(), l2.end(), vc4.begin()));

        std::vector v5{std::tuple(10, 2.0), std::tuple(3, 5.0), std::tuple(3, 2.0), std::tuple(11, 3.0)};
        auto vc5 = v5;

        radix_sort(v5);
        std::sort(vc5.begin(), vc5.end());

        assert(std::equal(v5.begin(), v5.end(), vc5.begin()));

        std::vector v6{std::tuple(10, std::tuple(4, 2.0)), std::tuple(3, std::tuple(5,5.0)), std::tuple(3, std::tuple(9, 2.0)), std::tuple(11, std::tuple(33,3.0))};
        auto vc6 = v6;

        radix_sort(v6);
        std::sort(vc6.begin(), vc6.end());

        assert(std::equal(v6.begin(), v6.end(), vc6.begin()));

        struct notDefaultConstr
        {
            explicit notDefaultConstr(int val): val_(val){}

            bool operator<(const notDefaultConstr& other) const
            {
                return val_ < other.val_;
            }

            bool operator==(const notDefaultConstr& other) const
            {
                return val_ == other.val_;
            }

            int val_;
        };

        std::vector<notDefaultConstr> v7{notDefaultConstr{1},notDefaultConstr{2},notDefaultConstr{3},notDefaultConstr{4}};
        auto vc7 = v7;

        radix_sort(v7, [](const notDefaultConstr& elem)
            {
                return elem.val_;
            });

        std::sort(vc7.begin(), vc7.end());

        assert(std::equal(v7.begin(), v7.end(), vc7.begin()));

        std::tuple<std::tuple<uint8_t, uint8_t>, uint8_t, uint8_t> val1;
        static_assert(std::is_same_v<decltype(risuwwv::details::tail(val1)),  std::tuple<uint8_t, uint8_t>>);

        std::tuple<std::tuple<uint8_t, uint8_t>, std::tuple<uint8_t, uint8_t>> val2;
        static_assert(std::is_same_v<decltype(risuwwv::details::tail(val2)),  std::tuple<std::tuple<uint8_t, uint8_t>>>);

    }

}//end of namespace testcases

int main()
{
    testcases::magic_get_testcases();
    testcases::array_testcases();
    testcases::const_testcases();

    testcases::tuple1_testcases();
    testcases::tuple2_testcases();
    testcases::tuple3_testcases();  
    testcases::tuple4_testcases(); 
    testcases::tuple5_testcases();
    testcases::tuple6_testcases();
    testcases::tuple7_testcases();  
    testcases::tuple8_testcases();

    testcases::int_testcases();
    testcases::float_testcases();
    testcases::double_testcases();
    testcases::general_testcases();
    testcases::types_testcases();

    std::cout << "All tests are ok\n";
}
