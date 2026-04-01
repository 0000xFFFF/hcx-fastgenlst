# hcx-fastgenlst

Generate a password wordlist from strings (words).

###### this is a faster version of hcx-genlst from [hcx-tools-extra](https://github.com/0000xFFFF/hcx-tools-extra)

## Example
```sh
./hcx-fastgenlst -s steve -lut123
```

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
* [argparse](https://github.com/p-ranav/argparse)

## Build & Install
```sh
make
sudo make install
```

## Usage
```
./hcx-fastgenlst -h
```

```
Usage: hcx-fastgenlst [--help] [--version] [--string VAR]... [--input VAR] [--output VAR] [--verbose] [--lower] [--upper] [--title] [--reverse] [--wordint] [--intword] [--intwordint] [--check] [--double] [--double-small] [--year] [--join VAR] [--min-len VAR]

Generate a password wordlist from strings (words)

Optional arguments:
  -h, --help          shows help message and exits
  -v, --version       prints version information and exits
  -s, --string        append word to word set for generation (can have multiple -s) [may be repeated]
  -i, --input         append every line in file to word set
  -o, --output        file to write to (default: stdout)
  -v, --verbose       be verbose (print status)
  -l, --lower         add lowercase word variation to word set
  -u, --upper         add UPPERCASE word variation to word set
  -t, --title         add Titlecase word variation to word set
  -r, --reverse       add reversed word variation to word set
  -1 --wordint        word + int
  -2 --intword        int + word
  -3 --intwordint     int + word + int
  -c, --check         check if output is unique, don't generate dupes, slower
  -d, --double        double mode -- permutate every word in word set len 2 (<str><str>)
  -z, --double-small  double mode -- just do (<str1><str1>)
  -y, --year          just generate [0](0-100) and years 1800-2025
  -j, --join          double mode -- join string (<str><join><str>) [nargs=0..1] [default: ""]
  -m, --min-len       min password len (default: 8) [nargs=0..1] [default: 8]
```
