CXX = g++
LD = g++
OPTI = -O2
FLAGS = -Wall -std=c++11 -g $(OPTI)
SRC = $(filter-out "Pipe.cpp",$(shell echo *.cpp))
OBJS = $(subst .cpp,.o,$(SRC))
TEST_SRC = 
DEFS = 
EXEC = walk


all: $(OBJS)
	echo "Sources: " $(SRC)
#	g++ -Wall -g -O2 -std=c++11 -o walk walk.cpp
	echo $(OBJS)
	$(LD) $(FLAGS) -o $(EXEC) $(OBJS)

tests: all
	$(MAKE) -C unit_tests

%.o: %.cpp
	$(CXX) $(FLAGS) $(DEFS) -c $<

clean:
	rm -f *.o $(EXEC)
	$(MAKE) -C unit_tests clean

clean_obj:
	rm -f *.o

.PHONY:tests, clean

