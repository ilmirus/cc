pp: main.cpp preprocessor.cpp preprocessor.h pp_test.cpp pp_test.h
	g++ -std=c++20 -Wno-narrowing -g main.cpp preprocessor.cpp pp_test.cpp -o pp

clean:
	rm pp
