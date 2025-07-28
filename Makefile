GCC = g++
CLANG = clang++
FILES = *.c *.cpp

release:
	$(GCC) -g -Wall -Wextra -O2 $(FILES) -o hcx-fastgenlst

o1:
	$(GCC) -g -Wall -Wextra -O1 $(FILES) -o hcx-fastgenlst-o1

o2:
	$(GCC) -g -Wall -Wextra -O2 $(FILES) -o hcx-fastgenlst-o2

o3:
	$(GCC) -g -Wall -Wextra -O3 $(FILES) -o hcx-fastgenlst-o3

o1c:
	$(CLANG) -g -Wall -Wextra -O1 $(FILES) -o hcx-fastgenlst-o1-c

o2c:
	$(CLANG) -g -Wall -Wextra -O2 $(FILES) -o hcx-fastgenlst-o2-c

o3c:
	$(CLANG) -g -Wall -Wextra -O3 $(FILES) -o hcx-fastgenlst-o3-c

all: release o1 o2 o3 o1c o2c o3c

install:
	install -m 755 hcx-fastgenlst /usr/local/bin

debug:
	$(GCC) $(FILES) -o hcx-fastgenlst

