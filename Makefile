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

pptoken/.generated.stamp: lex/lex pptoken/pptoken.lex
	lex/lex pptoken/pptoken.lex
	touch pptoken/.generated.stamp

pptoken/pptoken.enum.generated.cpp \
pptoken/pptoken.regex.generated.cpp \
pptoken/pptoken.match.generated.cpp \
pptoken/pptoken.begin.generated.cpp: pptoken/.generated.stamp

clean:
	rm -f pptoken/pp_test pptoken/*.generated.cpp pptoken/.generated.stamp lex/lex

test: pptoken/pp_test
	./pptoken/pp_test
