

all:main


main:
	clang++ -emit-llvm -O0 -c hello.cpp -o hello.bc    #create bitcode .bc
	llc hello.bc                                 #create assembly .s
	g++ hello.s -o bin                           #create executable "foo"



