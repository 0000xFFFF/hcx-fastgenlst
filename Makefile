GCC = g++
FILES = *.c main.cpp

debug:
	$(GCC) $(FILES) -o hcx-fastgenlst

release:
	$(GCC) -g -Wall -Wextra $(FILES) -o hcx-fastgenlst
