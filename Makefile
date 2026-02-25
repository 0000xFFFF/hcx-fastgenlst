GCC = g++
FILES = src/*.c src/*.cpp

release:
	$(GCC) -g -Wall -Wextra -O2 $(FILES) -o hcx-fastgenlst

clean:
	rm hcx-fastgenlst

install:
	#install -m 755 hcx-fastgenlst /usr/local/bin/hcx-fastgenlst
	ln -sfr hcx-fastgenlst /usr/local/bin/hcx-fastgenlst

uninstall:
	rm /usr/local/bin/hcx-fastgenlst

debug:
	$(GCC) $(FILES) -o hcx-fastgenlst

