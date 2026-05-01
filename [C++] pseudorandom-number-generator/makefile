CXX = g++
CXXFLAGS = -std=c++17 -Iinclude -Wall -Werror -Wextra -pedantic

SRC_DIR = src
OBJ_DIR = obj
SRC = $(SRC_DIR)/main.cpp $(SRC_DIR)/bbs.cpp $(SRC_DIR)/primes.cpp $(SRC_DIR)/fips_tests.cpp $(SRC_DIR)/bg_crypto.cpp
OBJ = $(addprefix $(OBJ_DIR)/, $(notdir $(SRC:.cpp=.o)))

EXE = bbs_project

all: $(OBJ_DIR) $(EXE)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(EXE): $(OBJ)
	$(CXX) $(OBJ) -lgmp -o $(EXE)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(EXE)

.PHONY: all clean