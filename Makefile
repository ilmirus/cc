all: pptoken/pp_test

pptoken/pp_test: pptoken/pptoken.cpp pptoken/pptoken.h \
		pptoken/pp_test.cpp pptoken/pp_test.h \
		utils/file_utils.cpp utils/file_utils.h \
		utils/string_utils.cpp utils/string_utils.h \
		pptoken/pp_input.h \
		pptoken/pptoken.enum.generated.cpp pptoken/pptoken.regex.generated.cpp pptoken/pptoken.match.generated.cpp \
		pptoken/pptoken.begin.generated.cpp
	g++ -o pptoken/pp_test -std=c++20 -Wno-narrowing -g -I . -I pptoken \
		pptoken/pptoken.cpp \
		pptoken/pp_test.cpp \
		utils/file_utils.cpp \
		utils/string_utils.cpp

lex/lex: lex/lex.cpp \
		utils/file_utils.cpp utils/file_utils.h \
		utils/string_utils.cpp utils/string_utils.h
	g++ -I . -o lex/lex -std=c++20 -Wno-narrowing -g -I utils \
		lex/lex.cpp \
		utils/file_utils.cpp \
		utils/string_utils.cpp

pptoken/pptoken.enum.generated.cpp: lex/lex pptoken/pptoken.lex
	lex/lex pptoken/pptoken.lex

pptoken/pptoken.regex.generated.cpp: lex/lex pptoken/pptoken.lex
	lex/lex pptoken/pptoken.lex

pptoken/pptoken.match.generated.cpp: lex/lex pptoken/pptoken.lex
	lex/lex pptoken/pptoken.lex

pptoken/pptoken.begin.generated.cpp: lex/lex pptoken/pptoken.lex
	lex/lex pptoken/pptoken.lex

clean:
	rm pptoken/pp_test *pptoken/generated.cpp lex/lex

test: pptoken/pp_test
	./pptoken/pp_test