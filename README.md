# hcx-fastgenlst - faster version of hcx-genlst from hcx-scripts

Generate a password wordlist from strings (words).

###### ./hcx-fastgenlst -s steve -lut123
Will generate passwords like:
```
steve123
steve69
Steve42
123STEVE
steve012345
...
```
```
* -l add lowercase variation  # e.g. steve => steve
* -u add uppercase variation  # e.g. steve => STEVE
* -t add titlecase variation  # e.g. steve => Steve  |  stEve => StEve
* -1 word + int               # e.g. steve123
* -2 int + word               # e.g. 123STEVE
* -3 int + word + int         # e.g. 12steve12
```

## Requirements
* make
* g++

## Build & Install
```
make
./install.sh
```

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
