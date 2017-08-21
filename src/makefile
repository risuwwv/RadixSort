#make format

#make compiler=gcc sanitize=scan-build
#make compiler=gcc sanitize=ubsan
#make compiler=gcc sanitize=asan
#make compiler=gcc sanitize=tsan
#make compiler=gcc sanitize=sast

#make compiler=clang sanitize=scan-build
#make compiler=clang sanitize=ubsan
#make compiler=clang sanitize=asan
#make compiler=clang sanitize=tsan
#make compiler=clang sanitize=sast

#make compiler=clang sanitize=cfi        #does not exist for gcc
#make compiler=clang sanitize=msan       #does not exist for gcc
#make compiler=clang sanitize=clang-tidy

#make compiler=clang mode=release exception=0 rtti=0 m32=1 optim=O3 #32bits release mode (with O3) and exceptions/rtti deactivated

################################################################################################

############################
#Comments about used flags:
############################

#-fno-math-errno        #Do not set ERRNO after calling math functions that are executed with a single instruction, e.g., sqrt
#-Waggregate-return     #is too much for an error.
#-fvisibility=hidden    #This is like Windows: don't export anything from DLL by default

################################################################################################

############################
#Comments about removed flags:
############################

#Removed: -funsafe-loop-optimizations

#Removed:
	#Wstrict-aliasing=2 #it is bad... ("Aggressive, quick, not too precise" vs default mode (=3) "Should have very few false positives and few false negatives."
    #-fstrict-overflow is implied by O2
	#Wunused-variable #not necessary: in WALL
	#Wcomment #not necessary: in WALL
	#Wchar-subscripts #not necessary: in WALL
	#Wempty-body #not necessary: in WEXTRA
	#Wmissing-field-initializers #not necessary: in WEXTRA
	#Wsign-compare #not necessary: in WALL
	#Wtype-limits #not necessary: in Wextra
	#Wpedantic #not necessary = pedantic
	#Wformat-security #replaced by -Wformat=2
	#Wformat-y2k #replaced by -Wformat=2
	#Wunused-parameter #not necessary: in Wextra
	#-fmerge-all-constants #BAD
    #omit-frame-pointer #is enabled in O
    #-Wunused #is in Wall
    #-Wtype-limits #is included in Wextra
    #-fstack-protector-all #leak sanitizer does not output anything if is used, probably not nice in general
    #-Qunused-arguments is useless

################################################################################################

#inspired by https://github.com/onqtam/doctest/blob/master/scripts/common.cmake#L71

#can check https://gcc.gnu.org/onlinedocs/gcc/C_002b_002b-Dialect-Options.html

#-std=c++14 or -std=c++1z
CPP_VERSION=-std=c++1z

ifndef travis
    CLANG_LINKER=-stdlib=libc++ -lc++
else
    CLANG_LINKER=
endif

GCC_LINKER=

SRC=radixSortTests.cpp
SRCPERF=radixSortPerfTests.cpp
SRCFUZZ=radixSortFuzz.cpp

EXE_NAME=radixSort
EXE_NAME_PERF=radixSortPerf
EXE_NAME_FUZZ=radixSortFuzz
######################################################################################################

ifndef optim
    ifeq ($(mode),release)
        OPTIM_LEVEL=-O3
    else
        OPTIM_LEVEL=-O1
    endif
else
    ifeq ($(optim),-O3)
        OPTIM_LEVEL=-O3
    else ifeq ($(optim),-O1) 
        OPTIM_LEVEL=-O1
    else ifeq ($(optim),-O2) 
        OPTIM_LEVEL=-O2
    else ifeq ($(optim),-O2) 
        OPTIM_LEVEL=-O2
    else ifeq ($(optim),-Os) 
        OPTIM_LEVEL=-Os
    else
        $(error "optim" value is invalid)
    endif
endif

#-fdiagnostics-show-option
#-fstrict-aliasing

#GNU and Clang:
COMMONS=$(OPTIM_LEVEL) $(CPP_VERSION) -Werror -fvisibility=hidden \
	-Wno-endif-labels -Wno-missing-field-initializers -Wno-error=effc++ -Wno-error=inline -Wno-error=aggregate-return

#-Wformat=2
#-Wstrict-overflow=5

#-Wparentheses 
#-Winit-self
#-Wuninitialized
#-Wunused-function 
#-Wmisleading-indentation
#-Wshift-negative-value

#not accepted by clang on my windows nor on my ubuntu
GCC_ONLY=-Wuseless-cast -Wzero-as-null-pointer-constant -Wnoexcept -Wsuggest-final-methods -Wsuggest-final-types \
	-Wsuggest-override -Wtrampolines -Wall -Wextra -Wconversion -Wold-style-cast -Wabi -Wimport \
	-Wmissing-format-attribute -Wmissing-noreturn -Wodr -Woverlength-strings -Wpacked -Wunreachable-code \
	-Wvariadic-macros -Wunused-local-typedefs -Wvla -pedantic -pedantic-errors \
	-Wfloat-equal -Wundef -Wredundant-decls -Wshadow \
	-Wwrite-strings -Wpointer-arith -Wcast-qual -Wswitch-default -Wmissing-include-dirs \
	-Wcast-align -Wformat-nonliteral -Wswitch-enum \
	-Wnon-virtual-dtor -Wctor-dtor-privacy \
	-Wsign-promo -Wsign-conversion -Wdisabled-optimization -Weffc++ -Winline -Winvalid-pch -Wstack-protector \
	-Wmissing-declarations -Woverloaded-virtual -Wvector-operation-performance -Wlogical-op -Wno-pmf-conversions \
	-Wunsafe-loop-optimizations -Wstrict-null-sentinel -Wno-error=noexcept -Wno-missing-declarations -Wno-inline -Wno-unsafe-loop-optimizations

#-Wno-missing-prototypes does not work for C++ with gcc (-Wno-missing-declarations instead...)
CLANG_COMMONS=$(COMMONS) -Weverything -fdiagnostics-color=always -Wno-c++98-compat-pedantic -Wno-error=deprecated -Wno-missing-prototypes -Wno-missing-variable-declarations -Wno-error=global-constructors -Wno-undefined-func-template -Wno-padded -Wno-error-unused-member-function -Wno-missing-braces $(CLANG_LINKER)

#TODO review this:
#on windows, my clang works only with "Developer Command Prompt for VS2015" + -m32
ifdef SYSTEMROOT
	CLANG_COMMONS+=-m32
else
	CLANG_COMMONS+=$(NOT_WINDOWS_CLANG)
endif

GCC_COMMONS=$(COMMONS) $(GCC_ONLY) $(GCC_LINKER)

GCC_VERSION_GTEQ_6 := $(shell expr `gcc -dumpversion | cut -f1 -d.` \>= 6)
GCC_VERSION_GTEQ_7 := $(shell expr `gcc -dumpversion | cut -f1 -d.` \>= 7)

GCC6_COMMONS=-Wshift-overflow=2 -Wnull-dereference -Wduplicated-cond
GCC7_COMMONS=-Walloc-zero -Walloca 

ifeq "$(GCC_VERSION_GTEQ_6)" "1"
    GCC_COMMONS+=$(GCC6_COMMONS)
endif

ifeq "$(GCC_VERSION_GTEQ_7)" "1"
    GCC_COMMONS+=$(GCC7_COMMONS)
endif

ifeq ($(exception),0)
    COMMONS+=-fno-exceptions -fno-asynchronous-unwind-tables -fno-unwind-tables
endif

ifeq ($(rtti),0)
    COMMONS+=-fno-rtti
endif

ifeq ($(m32),1)
    COMMONS+=-m32
endif

#At least on GCC, -O0 has strange effects to remove compiler warning on array-bounds/uninitialized
#-D_LIBCPP_DEBUG=1 does not work anymore...
CLANG_DEBUG=-g $(CLANG_COMMONS)
GCC_DEBUG= -g -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -D_FORTIFY_SOURCE=2 -D_GLIBCXX_ADDRESS_SANITIZER_ANNOTATIONS $(GCC_COMMONS)

#Probably not important (may or may not reduce size...):
#-ffunction-sections -fdata-sections -Wl,--gc-sections

#-fno-ident
#-Wl,-z,norelro
#-Wl,--hash-style=gnu 
#-Wl,--build-id=none
#strip -S --strip-unneeded --remove-section=.note.gnu.gold-version --remove-section=.comment --remove-section=.note --remove-section=.note.gnu.build-id --remove-section=.note.ABI-tag
#upx --ultra-brute

#-flto needs gold installed (http://llvm.org/docs/GoldPlugin.html)
RELEASE_COMMONS=-march=native -DNDEBUG -fno-math-errno -flto

CLANG_RELEASE=$(RELEASE_COMMONS) $(CLANG_COMMONS)
GCC_RELEASE=$(RELEASE_COMMONS) $(GCC_COMMONS)

CC=

ifeq ($(sanitize),scan-build)
    CC+=scan-build
else ifeq ($(sanitize),cfi)
    ifeq ($(compiler),clang)
        SANITIZER=-flto -fsanitize=cfi
    endif
else ifeq ($(sanitize),ubsan)
    #enable things that may not be a bug but still useful 
    #TODO make a ubsanInt if integer checks are annoying...

    ifeq ($(compiler),gcc)
        #for gcc, float-divide-by-zero is not part of undefined
	    SANITIZER=-fno-omit-frame-pointer -fsanitize=undefined,bounds-strict,float-divide-by-zero,float-cast-overflow
    else ifeq ($(compiler),clang)
        #unsigned-integer-overflow is not undefined behaviour but might still be a bug, 
        #so it is separated from the rest (included in 'integer')
        SANITIZER=-fno-omit-frame-pointer -fsanitize=undefined,integer
    endif

else ifeq ($(sanitize),asan)

    #See https://github.com/google/sanitizers/wiki/AddressSanitizerFlags
    #RUNTIME OPTIONS:
    #export ASAN_OPTIONS=check_initialization_order=true:strict_init_order=true:detect_stack_use_after_return=1
    #
    #address implies lsan (-fsanitize=leak) for both clang and gcc
    #
    #-fsanitize-address-use-after-scope is not activated by default
    #See https://github.com/google/sanitizers/wiki/AddressSanitizerExampleUseAfterScope

    ifeq ($(compiler),gcc)
        #gcc does not recognize "-fsanitize-address-use-after-scope"
	    SANITIZER=-fsanitize=address -fno-omit-frame-pointer
    else ifeq ($(compiler),clang)
        SANITIZER=-fsanitize=address -fsanitize-address-use-after-scope -fno-omit-frame-pointer
    endif

else ifeq ($(sanitize),msan)

    ifeq ($(compiler),clang)
        SANITIZER=-fsanitize=memory -fsanitize-memory-track-origins=1
#does not like gtest (or other uninstrumented code...), see https://github.com/google/sanitizers/wiki/MemorySanitizerLibcxxHowTo
    endif

else ifeq ($(sanitize),tsan)

    #https://github.com/google/sanitizers/wiki/ThreadSanitizerFlags
    #https://github.com/google/sanitizers/wiki/ThreadSanitizerDeadlockDetector
    #RUNTIME OPTIONS:
    #export TSAN_OPTIONS=detect_deadlocks=1:second_deadlock_stack=1

	SANITIZER=-fsanitize=thread

else ifeq ($(sanitize),sast)
    ifeq ($(compiler),gcc)
	    SANITIZER=-fstack-check
    else ifeq ($(compiler),clang)
	    SANITIZER=-fsanitize=safe-stack
    endif
endif

#https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html
#TODO check, should already be on in ubsan gcc: -fsanitize=alignment, -fsanitize=object-size, -fsanitize=vptr, -fsanitize=bool, -fsanitize=enum
#Clang: -fsanitize=function
#gcc: -fvtable-verify=std

ifdef SystemRoot
	TARGET_EXT=.exe
	INCLUDE=-IC:\boost_1_63_0
endif

OUT=-o $(EXE_NAME)$(TARGET_EXT)
OUT_PERF=-o $(EXE_NAME_PERF)$(TARGET_EXT)
OUT_FUZZ=-o $(EXE_NAME_FUZZ)$(TARGET_EXT)

#compiler choice
ifeq ($(compiler),gcc)
	ifeq ($(mode),release)
		FLAGS=$(GCC_RELEASE)
	else
		FLAGS=$(GCC_DEBUG)
	endif
	CC+=g++

    ifeq ($(test),1)
	    FLAGS+=-lgtest -lgtest_main -lpthread
	endif

else ifeq ($(compiler),clang)
	ifeq ($(mode),release)
		FLAGS=$(CLANG_RELEASE)
	else
		FLAGS=$(CLANG_DEBUG)
	endif

    ifeq ($(sanitize),clang-tidy)
        CC=clang-tidy
        OUT=
        FLAGS=-checks=modernize*,google*,llvm*,cppcoreguidelines*,cert*,boost*,clang-analyzer*,misc*,performance*,readability*,mpi*,-clang-diagnostic-c++11-extensions,-google-readability-todo,-clang-diagnostic-missing-variable-declarations,-google-runtime-references,-readability-else-after-return,-readability-named-parameter -warnings-as-errors=*,-cert-err58-cpp,-modernize-use-auto,-static-cast-downcast,-cppcoreguidelines-pro-type-member-init,-cppcoreguidelines-pro-type-static-cast-downcast,-misc-noexcept-move-constructor,-cppcoreguidelines-pro-type-reinterpret-cast,-cppcoreguidelines-pro-bounds-array-to-pointer-decay
	else
	    CC+=clang++
	endif
endif

all: unit fuzz perf

unit:
	$(CC) $(OUT) $(SRC) $(FLAGS) $(INCLUDE) $(SANITIZER)

#TODO are flags correct, is libFuzzer.a needed?
fuzz:
	$(CC) $(OUT_FUZZ) $(SRCFUZZ) $(FLAGS) $(INCLUDE) $(SANITIZER) -fsanitize-coverage=trace-pc-guard,edge -fsanitize=fuzzer

perf:
	$(CC) $(OUT_PERF) $(SRCPERF) $(FLAGS) $(INCLUDE) $(SANITIZER)

clean:	
	rm -f radixSort
	rm -f radixSortFuzz
	rm -f radixSortPerf
	rm -f radixSort.exe
	rm -f radixSortFuzz.exe
	rm -f radixSortPerf.exe
	
#temporay
#TODO make it clean
#run 'make sanitize=clang-tidy compiler=clang test'

current_dir=$(shell pwd)

format: $(SRC)

#$@ designate a file in $(SRC)
#"clang-format -style=file $@ > $@" erases the file -_-"
$(SRC): %: FORCE
	@echo "Formated $@"
	@cp $@ $@.old
	@clang-format -style=file $@ > $@.tmp
	@cp $@.tmp $@
	@rm $@.tmp

#forces $(SRC) rule to always run
FORCE:

test:
	@echo ""
	@echo "###########################"
	@echo "clang-tidy"
	@echo ""
	-clang-tidy $(SRC) $(FLAGS)
	@echo ""
	@echo "###########################"
	@echo "cppcheck"
	@echo ""
	-cppcheck --enable=all --inconclusive --std=posix radixSortTests.cpp
	@echo ""
	@echo "###########################"
	@echo "cpplint"
	@echo ""
    #cpplint lines are only 80, it is too short...
	-python2 cpplint.py --filter=-legal,-whitespace/comments,-whitespace/braces,-runtime/references,-whitespace/newline,-whitespace/parens,-whitespace/line_length,-readability/todo radixSortTests.cpp
	@echo ""
	@echo "###########################"
	@echo "oclint"
	@echo ""
	-oclint radixSortTests.cpp -- -c -std=c++14 -g
	@echo ""
	@echo "###########################"
	@echo "flawfinder"
	@echo ""
	-python2 /usr/bin/flawfinder -m 0 radixSortTests.cpp
	@echo ""
	@echo "###########################"
	@echo "vera"
	@echo ""
	-vera++ -s -R F001 -R F002 -R T001 -R T002 -R T003 -R T004 -R T005 -R T006 -R T007 -R T009 -R T010 -R T011 -R T012 -R T015 -R T016 -R T017 -R T018 -R T019 -R L001 -R L002 -R L003 -R L004 -R L005 -R L006 radixSortTests.cpp
	@echo ""
	@echo "###########################"
	@echo "flint"
	@echo ""	
	-flint++ radixSortTests.cpp
	@echo ""
	@echo "###########################"
	@echo "pvs-studio"
	-pvs-studio-analyzer analyze -j8 -e /usr/local
	echo "cat PVS-Studio.tasks"
	-plog-converter -a 'GA:1,2,3;64:1,2,3;OP:1,2,3;CS:1,2,3' -t tasklist -o $(current_dir)/PVS-Studio.tasks $(current_dir)/PVS-Studio.log
	./makefileHelper.sh

#DATABASE TUTORIAL for clang-tidy:
#http://eli.thegreenplace.net/2014/05/21/compilation-databases-for-clang-based-tools
#The json I made is compile_commands.json
#[
#{
#  "directory": "PATH",
#  "command": "clang++ -o ~/Desktop/main radixSortTests.cpp -march=native -O3 -DNDEBUG -fno-math-errno -std=c++14 -Werror -pedantic -pedantic-errors -fvisibility=hidden -fstrict-aliasing -Wall -Wextra -fdiagnostics-show-option -Wconversion -Wno-missing-field-initializers -Wold-style-cast -Wabi -Wimport -Wmissing-format-attribute -Wmissing-noreturn -Wodr -Woverlength-strings -Wpacked -Wunreachable-code -Wvariadic-macros -Wunused-local-typedefs -Wfloat-equal -Wundef -Wredundant-decls -Wshadow -Wstrict-overflow=5 -Wwrite-strings -Wpointer-arith -Wcast-qual -Wformat=2 -Wswitch-default -Wmissing-include-dirs -Wcast-align -Wformat-nonliteral -Wparentheses -Winit-self -Wuninitialized -Wswitch-enum -Wno-endif-labels -Wunused-function -Wnon-virtual-dtor -Wctor-dtor-privacy -Wsign-promo -Wsign-conversion -Wdisabled-optimization -Weffc++ -Winline -Winvalid-pch -Wstack-protector -Wmissing-declarations -Woverloaded-virtual -Wvla -Wno-error=effc++ -Wno-error=inline -Wno-error=aggregate-return -Wunused -Weverything -Qunused-arguments -fdiagnostics-color=always -Wno-c++98-compat-pedantic -Wno-error=deprecated -Wno-missing-prototypes",
#  "file": "PATH/radixSortTests.cpp"
#}
#]

#TODO check this: clang-format is cool too

#TODO add coverty's scan, metric++, copy-paste detector and oink/CQual++ (static analyzer)

#Works, TODO add:
#TODO install it on Windows too as it works with MSVC (search sln in http://cppcheck.sourceforge.net/manual.pdf)
#it has a -j 8 option but it deactivates unusedFunction checks so the option is a bit bad unless run twice...

#cppcheck --enable=all --inconclusive --std=posix radixSortTests.cpp

#pp-trace, clang modularize and clang-rename are nice tools

#TODO sanitizers and check if new ones...
#usan:
#	clang++ -o test test.cpp -g -fno-omit-frame-pointer  $(SANITIZER)	



