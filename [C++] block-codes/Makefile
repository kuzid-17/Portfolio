CXX = g++
CXXFLAGS = -std=c++17 -Iinclude -Wall -Werror -Wextra -pedantic

EXE = pucch_f2
SRC = $(wildcard src/*.cpp)
OBJ = $(patsubst src/%.cpp, obj/%.o, $(SRC))

all: $(EXE)

$(EXE): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(EXE)

obj/%.o: src/%.cpp | obj/
	$(CXX) $(CXXFLAGS) -c $< -MMD -o $@

obj/:
	mkdir -p obj/

include $(wildcard obj/*.d)

clean:
	rm -rf obj/ $(EXE)

.PHONY: all clean
