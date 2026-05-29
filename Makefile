CXX := g++
CXXFLAGS := -std=c++20 -Wno-narrowing -g -I .

all: pptoken/pp_test ctrlexpr/test

# generated pptoken files
pptoken/.generated.stamp: lex/lex pptoken/pptoken.lex
	lex/lex pptoken/pptoken.lex
	touch pptoken/.generated.stamp

# objects
utils/file_utils.o: utils/file_utils.cpp utils/file_utils.h
	$(CXX) -c -o $@ $(CXXFLAGS) utils/file_utils.cpp

utils/string_utils.o: utils/string_utils.cpp utils/string_utils.h
	$(CXX) -c -o $@ $(CXXFLAGS) utils/string_utils.cpp

lex/grammar_common.o: lex/grammar_common.cpp lex/grammar_common.h
	$(CXX) -c -o $@ $(CXXFLAGS) lex/grammar_common.cpp

pptoken/pptoken.o: pptoken/pptoken.cpp pptoken/pptoken.h \
                   pptoken/pp_input.h \
                   pptoken/pptoken.enum.generated.cpp \
                   pptoken/pptoken.regex.generated.cpp \
                   pptoken/pptoken.match.generated.cpp \
                   pptoken/pptoken.begin.generated.cpp
	$(CXX) -c -o $@ $(CXXFLAGS) -I pptoken pptoken/pptoken.cpp

ctrlexpr/ctrlexpr.o: ctrlexpr/ctrlexpr.cpp ctrlexpr/ctrlexpr.h pptoken/pptoken.enum.generated.cpp
	$(CXX) -c -o $@ $(CXXFLAGS) -I ctrlexpr ctrlexpr/ctrlexpr.cpp

utils/libutils.o: utils/file_utils.o utils/string_utils.o
	$(CXX) -r -o $@ $^

# tools
lex/lex: lex/lex.cpp lex/grammar_common.o utils/libutils.o
	$(CXX) -o $@ $(CXXFLAGS) -I lex \
		lex/lex.cpp \
		lex/grammar_common.o \
		utils/libutils.o

gnm/gnm: gnm/gnm.cpp lex/grammar_common.o utils/libutils.o utils/vector_utils.h
	$(CXX) -o $@ $(CXXFLAGS) \
		gnm/gnm.cpp \
		lex/grammar_common.o \
		utils/libutils.o

# tests
pptoken/pp_test: pptoken/pp_test.cpp pptoken/pp_test.h \
                 pptoken/pptoken.o utils/libutils.o
	$(CXX) -o $@ $(CXXFLAGS) -I pptoken \
		pptoken/pp_test.cpp \
		pptoken/pptoken.o \
		utils/libutils.o

ctrlexpr/test: ctrlexpr/test.cpp \
               ctrlexpr/ctrlexpr.o pptoken/pptoken.o utils/libutils.o \
               gnm/gnm
	$(CXX) -o $@ $(CXXFLAGS) -I ctrlexpr \
		ctrlexpr/test.cpp \
		ctrlexpr/ctrlexpr.o \
		pptoken/pptoken.o \
		utils/libutils.o

clean:
	rm -f pptoken/pp_test pptoken/*.generated.cpp pptoken/.generated.stamp \
	      lex/lex ctrlexpr/test gnm/gnm \
	      utils/*.o lex/*.o pptoken/*.o ctrlexpr/*.o \
	      utils/*.a lex/*.a pptoken/*.a ctrlexpr/*.a

test: pptoken/pp_test ctrlexpr/test
	./pptoken/pp_test
	./ctrlexpr/test

.PHONY: all clean test

.PHONY: bear
bear: Makefile
	bear --output compile_commands.json -- make -B -j$$(nproc)

debug: gnm/gnm
	gnm/gnm ctrlexpr/ctrlexpr

pptoken/pptoken.enum.generated.cpp \
pptoken/pptoken.regex.generated.cpp \
pptoken/pptoken.match.generated.cpp \
pptoken/pptoken.begin.generated.cpp: pptoken/.generated.stamp