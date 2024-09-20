GCC = g++
FILES = *.c *.cpp

release:
	$(GCC) -g -Wall -Wextra -O3 $(FILES) -o hcx-fastgenlst

debug:
	$(GCC) $(FILES) -o hcx-fastgenlst

