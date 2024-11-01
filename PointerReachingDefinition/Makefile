LLVM_CONFIG=../llvm_build/bin/llvm-config

CXX=`$(LLVM_CONFIG) --bindir`/clang
CXXFLAGS=`$(LLVM_CONFIG) --cppflags` -fPIC -fno-rtti
LDFLAGS=`$(LLVM_CONFIG) --ldflags`
IRFLAGS=-Xclang -disable-O0-optnone -fno-discard-value-names -S -emit-llvm
OPT=`$(LLVM_CONFIG) --bindir`/opt

.PHONY: all test run clean 
all: hw2.so test

test: test1.ll

hw2.so: hw2.cpp
	$(CXX) -shared -o $@ $< $(CXXFLAGS) $(LDFLAGS)

test1.ll: icpp.c
	$(CXX) $(IRFLAGS) -o $@ $<

test2.ll: icpp2.c
	$(CXX) $(IRFLAGS) -o $@ $<

test3.ll: icpp3.c
	$(CXX) $(IRFLAGS) -o $@ $<

test4.ll: foo.c
	$(CXX) $(IRFLAGS) -o $@ $<


run1: test1.ll hw2.so
	$(OPT) -disable-output -load-pass-plugin=./hw2.so -passes=hw2 $<

run2: test2.ll hw2.so
	$(OPT) -disable-output -load-pass-plugin=./hw2.so -passes=hw2 $<

run3: test3.ll hw2.so
	$(OPT) -disable-output -load-pass-plugin=./hw2.so -passes=hw2 $<

run4: test4.ll hw2.so
	$(OPT) -disable-output -load-pass-plugin=./hw2.so -passes=hw2 $<


clean:
	rm -f *.o *.ll *.so
