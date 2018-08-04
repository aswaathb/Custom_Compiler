CPPFLAGS += -std=c++11 -g
CPPFLAGS += -I include

all : bin/compiler

src/parser.tab.cpp src/parser.tab.hpp : src/parser.y include/ast.hpp include/ast/operations.hpp
	bison -v -d -Wnone src/parser.y -o src/parser.tab.cpp

src/lexer.yy.cpp : src/lexer.flex src/parser.tab.hpp
	flex -o src/lexer.yy.cpp  src/lexer.flex

bin/compiler : src/compiler.o src/parser.tab.o src/lexer.yy.o src/parser.tab.o
	mkdir -p bin
	g++ $(CPPFLAGS) -o bin/compiler $^

# test/test : test/test.cpp
# 	mkdir -p test
# 	g++ $(CPPFLAGS) -o test/test $^

clean :
	rm src/*.o
	rm bin/*
	rm src/*.tab.cpp
	rm src/*.output
	rm src/*.tab.hpp
	rm src/*.yy.cpp
