# hcx-fastgenlst

Faster version of hcx-genlst in hcx-scripts.

## Requirements
* make
* g++

## Usage
###### ./hcx-fastgenlst -h
```
 generate a password wordlist from strings (words)

 options:
   -h         show this help message and exit
   -s word    append word to word set for generation (can have multiple -s)
   -i infile  append every line in file to word set
   -o outfile file to write to (default: stdout)
   -v         be verbose (print status)
   -l         add lowercase word variation to word set
   -u         add UPPERCASE word variation to word set
   -t         add Title word variation to word set
   -r         reverse string
   -1         word + int
   -2         int + word
   -3         int + word + int
   -y         just generate [0](0-100) and years 1800-2025
   -m number  min password len (default: 8)
   -c         check if output is unique, don't generate dupes, slower
   -d         double mode -- permutate every word in word set len 2 (<str><str>)
   -z         double mode -- just do (<str1><str1>)
   -j string  double mode -- join string (<str><join><str>)
```
