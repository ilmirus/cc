pp: main.cpp preprocessor.cpp preprocessor.h pp_test.cpp pp_test.h file_utils.cpp file_utils.h string_utils.cpp string_utils.h
	g++ -std=c++20 -Wno-narrowing -g main.cpp preprocessor.cpp pp_test.cpp file_utils.cpp string_utils.cpp -o pp

clean:
	rm pp

test: pp
	./pp