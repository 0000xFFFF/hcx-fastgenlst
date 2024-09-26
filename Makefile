GCC = g++
FILES = *.c *.cpp

release:
	$(GCC) -g -Wall -Wextra -O2 $(FILES) -o hcx-fastgenlst

o1:
	$(GCC) -g -Wall -Wextra -O1 $(FILES) -o hcx-fastgenlst-o1

o2:
	$(GCC) -g -Wall -Wextra -O2 $(FILES) -o hcx-fastgenlst-o2

o3:
	$(GCC) -g -Wall -Wextra -O3 $(FILES) -o hcx-fastgenlst-o3

all: release o1 o2 o3

debug:
	$(GCC) $(FILES) -o hcx-fastgenlst

