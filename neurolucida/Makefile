GTEST_DIR = googletest/googletest/
SOURCES=lex.cpp parse_ll.cpp neuron_builder.cpp

CXX=clang++
CPPFLAGS += -isystem $(GTEST_DIR)/include -I.
CXXFLAGS += -g -rdynamic -Wall -Wextra -pthread -std=c++14

GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h
#
# Usually you shouldn't tweak such internal variables, indicated by a
# trailing _.
GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)


main_run: main
	env SEGFAULT_SIGNALS="abrt segv" LD_PRELOAD=/lib/x86_64-linux-gnu/libSegFault.so ./main small_morph.asc

test: tests
	./tests

main: main.cpp $(SOURCES)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -o $@

test.o: test.cpp $(SOURCES)

tests: test.o gtest_main.a $(SOURCES)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -lpthread test.o gtest_main.a -o $@

gtest-all.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest-all.cc

gtest_main.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest_main.cc

gtest.a : gtest-all.o
	$(AR) $(ARFLAGS) $@ $^

gtest_main.a : gtest-all.o gtest_main.o
	$(AR) $(ARFLAGS) $@ $^

clean:
	-rm -rf *.o *.a tests main
