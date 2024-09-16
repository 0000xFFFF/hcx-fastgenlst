GCC = g++
FLAGS = -g -Wall -Wextra

all:
	$(GCC) $(FLAGS) main.cpp -o hcx-fastgenlst
