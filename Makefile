pp: main.cpp \
		preprocessor.cpp preprocessor.h \
		pp_test.cpp pp_test.h \
		file_utils.cpp file_utils.h \
		string_utils.cpp string_utils.h \
		pp_input.h \
		pptoken.enum.generated.cpp pptoken.regex.generated.cpp pptoken.match.generated.cpp
	g++ -o pp -std=c++20 -Wno-narrowing -g \
		main.cpp \
		preprocessor.cpp \
		pp_test.cpp \
		file_utils.cpp \
		string_utils.cpp

lex/lex: lex/lex.cpp \
		file_utils.cpp file_utils.h \
		string_utils.cpp string_utils.h
	g++ -I . -o lex/lex -std=c++20 -Wno-narrowing -g \
		lex/lex.cpp \
		file_utils.cpp \
		string_utils.cpp

pptoken.enum.generated.cpp: lex/lex pptoken.lex
	lex/lex pptoken.lex

pptoken.regex.generated.cpp: lex/lex pptoken.lex
	lex/lex pptoken.lex

pptoken.match.generated.cpp: lex/lex pptoken.lex
	lex/lex pptoken.lex

clean:
	rm pp *generated.cpp lex/lex

test: pp
	./pp