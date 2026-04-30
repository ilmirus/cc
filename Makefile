all: pptoken/pp_test ctrlexpr/test

# pptoken
.PHONY: pptoken_core
pptoken_core: pptoken/pptoken.cpp pptoken/pptoken.h \
              utils/file_utils.cpp utils/file_utils.h \
              utils/string_utils.cpp utils/string_utils.h \
              pptoken/pp_input.h \
              pptoken/pptoken.enum.generated.cpp pptoken/pptoken.regex.generated.cpp \
              pptoken/pptoken.match.generated.cpp pptoken/pptoken.begin.generated.cpp

pptoken/pp_test: pptoken_core \
                 pptoken/pp_test.cpp pptoken/pp_test.h
	g++ -o pptoken/pp_test -std=c++20 -Wno-narrowing -g -I . -I pptoken \
		pptoken/pptoken.cpp \
		pptoken/pp_test.cpp \
		utils/file_utils.cpp \
		utils/string_utils.cpp

lex/lex: lex/lex.cpp \
		 utils/file_utils.cpp utils/file_utils.h \
		 utils/string_utils.cpp utils/string_utils.h \
		 lex/grammar_common.cpp lex/grammar_common.h
	g++ -o lex/lex -std=c++20 -Wno-narrowing -g -I . -I lex \
		lex/lex.cpp \
		utils/file_utils.cpp \
		utils/string_utils.cpp \
		lex/grammar_common.cpp

pptoken/.generated.stamp: lex/lex pptoken/pptoken.lex
	lex/lex pptoken/pptoken.lex
	touch pptoken/.generated.stamp

# ctrlexpr
ctrlexpr/test: ctrlexpr/test.cpp \
			   ctrlexpr/ctrlexpr.cpp ctrlexpr/ctrlexpr.h \
			   pptoken_core \
			   gnm/gnm
	g++ -o ctrlexpr/test -std=c++20 -Wno-narrowing -g -I . -I ctrlexpr \
		ctrlexpr/test.cpp \
		ctrlexpr/ctrlexpr.cpp \
		pptoken/pptoken.cpp \
		utils/file_utils.cpp

gnm/gnm: gnm/gnm.cpp \
		 lex/grammar_common.cpp lex/grammar_common.h
	g++ -o gnm/gnm -std=c++20 -Wno-narrowing -g -I . gnm/gnm.cpp lex/grammar_common.cpp

clean:
	rm -f pptoken/pp_test pptoken/*.generated.cpp pptoken/.generated.stamp lex/lex\
	      ctrlexpr/test gnm/gnm

test: pptoken/pp_test ctrlexpr/test
	./pptoken/pp_test
	./ctrlexpr/test

pptoken/pptoken.enum.generated.cpp \
pptoken/pptoken.regex.generated.cpp \
pptoken/pptoken.match.generated.cpp \
pptoken/pptoken.begin.generated.cpp: pptoken/.generated.stamp

.PHONY: bear
bear: Makefile
	bear --output compile_commands.json -- make -B -j$$(nproc)
