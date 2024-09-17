GCC = g++

debug:
	$(GCC) main.cpp -o hcx-fastgenlst

release:
	$(GCC) -g -Wall -Wextra main.cpp -o hcx-fastgenlst
