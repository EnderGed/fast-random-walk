CXX = g++
LD = g++
OPTI = -O2
FLAGS = -Wall -std=c++11 -g $(OPTI) -lboost_unit_test_framework -pthread
SRC = $(shell echo *.cpp)
OBJS = $(subst .cpp,.o,$(SRC))
DEFS = 
EXEC = $(subst .cpp,,$(SRC))


tests: $(EXEC)
	echo ; echo ; \
	for exec in $(EXEC); \
	do \
		if ! ./$$exec; \
		then echo "!!! ERROR !!! $$exec CRASHED!"; \
	fi; \
	done; \
	echo ; echo ;

%.o: %.cpp
	$(CXX) $(FLAGS) $(DEFS) -c $<

%:
	$(CXX) $(FLAGS) $(DEFS) -c $@.cpp
	$(LD) $(FLAGS) -o $@ $@.o $(filter-out ../walk.o, $(shell echo ../*.o))

clean:
	rm -f *.o $(EXEC)

clean_obj:
	rm -f *.o


