#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <algorithm>
#include <string>
#include <thread>
#include <mutex>
#include <chrono>
#include <getopt.h>
#include <cstring>
#include <iomanip>
#include <unordered_set>
#include <functional>
#include <atomic>
#include <cwchar>

#include <string>
#include <cstdlib>  // for free()
#include <cctype>   // for std::tolower, std::toupper

#include <locale>
#include <codecvt>

#include "UtfConv.h"

// Global settings and flags
std::set<std::string> arg_words;
std::set<std::string> words;
std::unordered_set<std::string> output_uniq;
std::mutex mtx;
bool verbose = false, lower = false, upper = false, title = false, reverse = false,
     wordint = false, intword = false, intwordint = false,
     to_file = false,
     check = false,
     double_mode = false, double_small = false,
     year = false;

std::string double_join = "";
int min_len = 8;
std::string input_file, output_file;
std::ofstream output;

size_t utf8_strlen(const std::string& str) {
    size_t length = 0;
    for (size_t i = 0; i < str.length(); ) {
        unsigned char c = str[i];
        if      (c <= 0x7F)              { i += 1;    } // 1-byte character (ASCII)
        else if (c >= 0xC2 && c <= 0xDF) { i += 2;    } // 2-byte character
        else if (c >= 0xE0 && c <= 0xEF) { i += 3;    } // 3-byte character
        else if (c >= 0xF0 && c <= 0xF4) { i += 4;    } // 4-byte character
        else                             { return -1; } // Invalid UTF-8 sequence encountered at byte i
        ++length;
    }
    return length;
}

void utf8_reverse(char *str) {
    /* this assumes that str is valid UTF-8 */
    char *scanl, *scanr, *scanr2, c;

    /* first reverse the string */
    for (scanl= str, scanr= str + strlen(str); scanl < scanr;)
        c= *scanl, *scanl++= *--scanr, *scanr= c;

    /* then scan all bytes and reverse each multibyte character */
    for (scanl= scanr= str; (c= *scanr++);) {
        if ( (c & 0x80) == 0) // ASCII char
            scanl= scanr;
        else if ( (c & 0xc0) == 0xc0 ) { // start of multibyte
            scanr2= scanr;
            switch (scanr - scanl) {
                case 4: c= *scanl, *scanl++= *--scanr, *scanr= c; // fallthrough
                case 3: // fallthrough
                case 2: c= *scanl, *scanl++= *--scanr, *scanr= c;
            }
            scanr= scanl= scanr2;
        }
    }
}

std::string utf8_tolower(const std::string& input) {
    Utf16Char* utf16_input = Utf8ToUtf16(reinterpret_cast<const Utf8Char*>(input.c_str()));
    if (!utf16_input) { return ""; }

    Utf16Char* lower_utf16 = Utf16StrMakeLwrUtf16Str(utf16_input);
    free(utf16_input);
    if (!lower_utf16) { return ""; }

    Utf8Char* lower_utf8 = Utf16ToUtf8(lower_utf16);
    free(lower_utf16);
    if (!lower_utf8) { return ""; }

    std::string result(reinterpret_cast<char*>(lower_utf8));
    free(lower_utf8);  // Free the UTF-8 string
    return result;
}

std::string utf8_toupper(const std::string& input) {
    Utf16Char* utf16_input = Utf8ToUtf16(reinterpret_cast<const Utf8Char*>(input.c_str()));
    if (!utf16_input) { return ""; }

    Utf16Char* upper_utf16 = Utf16StrMakeUprUtf16Str(utf16_input);
    free(utf16_input);
    if (!upper_utf16) { return ""; }

    Utf8Char* upper_utf8 = Utf16ToUtf8(upper_utf16);
    free(upper_utf16);
    if (!upper_utf8) { return ""; }

    std::string result(reinterpret_cast<char*>(upper_utf8));
    free(upper_utf8);
    return result;
}

std::string utf8_titlecase(const std::string& str) {

    int i = 0;
    unsigned char c = str[i];
    size_t char_length = 0;
    if      (c <= 0x7F)              { char_length = 1; } // 1-byte character (ASCII)
    else if (c >= 0xC2 && c <= 0xDF) { char_length = 2; } // 2-byte character
    else if (c >= 0xE0 && c <= 0xEF) { char_length = 3; } // 3-byte character
    else if (c >= 0xF0 && c <= 0xF4) { char_length = 4; } // 4-byte character
    else                             { return "";       } // Invalid UTF-8 sequence

    return utf8_toupper(str.substr(i, char_length)) + str.substr(char_length);
}

void print_args() {
    std::cerr << "vlutr123cdzy\n" <<
    verbose << lower << upper << title << reverse << wordint << intword << intwordint << check << double_mode << double_small << year << std::endl;
}

void print_words() {
    std::cerr << "==[ BEGIN WORDSET ]==" << std::endl;
    for (const auto& word : words) { std::cerr << word << " -- len: " << utf8_strlen(word) << std::endl; }
    std::cerr << "==[ END WORDSET ]==" << std::endl;
}

// Add variations of a word (lowercase, uppercase, title case, reverse)
void add_word_variations(const std::string& word) {
    words.insert(word);
    if (lower) { words.insert(utf8_tolower(word)); }
    if (upper) { words.insert(utf8_toupper(word)); }
    if (title) { words.insert(utf8_titlecase(word)); }

    if (reverse) {
        char* buff = new char[word.length()+1];
        std::strcpy(buff, word.c_str());
        utf8_reverse(buff);
        std::string reversed(buff);
        words.insert(reversed);
        delete[] buff;
    }
}

class Progress {
public:
    Progress(int total, int length = 25)
        : current(0), total(total), length(length), running(true), 
          start_time(std::chrono::steady_clock::now()), prev_time(start_time), prev_processed(0) {
    }

    void start() {
        progress_thread = std::thread([this]() {
            while (running) {
                display(current.load());
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        });
    }

    void display(int current) {
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_time = now - start_time;
        std::chrono::duration<double> time_delta = now - prev_time;

        double words_per_sec = (current - prev_processed) / time_delta.count();

        // Prevent division by zero
        double percent_complete = (total > 0) ? (static_cast<double>(current) / total) * 100.0 : 100.0;

        int filled_length = static_cast<int>(length * percent_complete / 100.0);
        std::string bar = std::string(filled_length, '#') + std::string(length - filled_length, '-');

        std::cerr << "\r[" << bar << "] " << std::fixed << std::setprecision(0)
                  << percent_complete << "% (" << current << "/" << total << ") "
                  << std::fixed << std::setprecision(1) << words_per_sec << " w/s          ";
        std::cerr.flush();

        prev_time = now;
        prev_processed = current;
    }

    void update(int current_value) {
        current.store(current_value);
    }

    void stop() {
        running = false;
        if (progress_thread.joinable()) {
            progress_thread.join();
        }
        std::cerr << std::endl;
    }

    void finish() {
        current = total;
        display(current.load());
        stop();
    }

private:
    std::atomic<int> current;
    int total;
    int length;
    std::atomic<bool> running;
    std::thread progress_thread;
    std::chrono::steady_clock::time_point start_time, prev_time;
    int prev_processed;
};

void out_minlen_uniq(const std::string& word) {

    // TODO: FIX -- PROBLEMATIC FOR UTF8
    if (word.length() < min_len) return;

    if (check) {
        std::lock_guard<std::mutex> lock(mtx);
        if (output_uniq.find(word) != output_uniq.end()) return;
        output_uniq.insert(word);
    }

    if (to_file) { output    << word << '\n'; }
    else         { std::cout << word << '\n'; }
}


void generate_wi(const std::string& w1, const std::string& w2)  { out_minlen_uniq(w1 + w2); }
void generate_iw(const std::string& w1, const std::string& w2)  { out_minlen_uniq(w2 + w1); }
void generate_iwi(const std::string& w1, const std::string& w2) { out_minlen_uniq(w2 + w1 + w2); }

// Generate suffixes for a word (e.g., word + "123", word + "!")
void generate_suffixes(const std::string& word, std::function<void(const std::string&, const std::string&)> callback) {

    std::vector<std::string> suffixes = {
        "!", "!!", "!!!",
        ".", "..", "...",
        "@", "@@", "@@@",
        "#",
        "12345", "123456", "1234567", "12345678", "123456789", "1234567890", "12345678910",
        "012345", "0123456", "01234567", "012345678", "0123456789", "01234567890", "012345678910",
    };

    for (const auto& suffix : suffixes) {
        callback(word, suffix);
        std::string reversed_suffix = suffix;
        std::reverse(reversed_suffix.begin(), reversed_suffix.end());
        if (suffix != reversed_suffix) {
            callback(word, reversed_suffix);
        }
    }

    for (int i = 0; i < 10; ++i) {
        callback(word, "00" + std::to_string(i));
    }

    if (year) {
        std::vector<std::string> special = { "123", "1234", "31337", "1337", "1312", "3112", "403" };
        for (const auto& suffix : special) {
            callback(word, suffix);
            std::string reversed_suffix = suffix;
            std::reverse(reversed_suffix.begin(), reversed_suffix.end());
            if (suffix != reversed_suffix) {
                callback(word, reversed_suffix);
            }
        }
        for (int i = 0; i < 101; ++i) {
            callback(word, std::to_string(i));
            callback(word, "0" + std::to_string(i));
        }
        for (int i = 1800; i < 2025; ++i) {
            callback(word, std::to_string(i));
        }
    }
    else {
        for (int i = 0; i < 10001; ++i) {
            callback(word, std::to_string(i));
            callback(word, "0" + std::to_string(i));
        }
    }
}

// Handle word number generation (word + int, int + word, int + word + int)
void wordnum() {
    int words_n = words.size();

    Progress progress(words_n);
    if (verbose) { progress.start(); }

    int count = 0;
    for (const auto& word : words) {
        if (verbose) { progress.update(++count); }

        if (wordint)    { generate_suffixes(word, generate_wi);  }
        if (intword)    { generate_suffixes(word, generate_iw);  }
        if (intwordint) { generate_suffixes(word, generate_iwi); }
    }

    if (verbose) { progress.finish(); }
}

// Handle double mode generation (word1 + join + word2)
void namename() {
    int words_n = words.size();

    Progress progress(words_n * (words_n - 1));
    if (verbose) { progress.start(); }

    int count = 0;
    for (const auto& word : words) {
        out_minlen_uniq(word + double_join + word);
        for (const auto& word2 : words) {
            if (word != word2) {
                out_minlen_uniq(word + double_join + word2);
            }
        }
    }

    for (const auto& word : words) {
        progress.update(++count);
        out_minlen_uniq(word + double_join + word);
        for (const auto& word2 : words) {
            if (word != word2) {
                out_minlen_uniq(word + double_join + word2);
            }
        }
    }

    if (verbose) { progress.finish(); }
}

// Load words from input file
void load_input_file(const std::string& input_file) {
    std::ifstream infile(input_file);
    std::string line;
    while (std::getline(infile, line)) { add_word_variations(line); }
}

void append_word_from_args(const std::string word) {
    arg_words.insert(word);
}

void print_help() {
    std::cout << 
    "\n"
    " generate a password wordlist from strings (words)\n"
    "\n"
    " options:\n"
    "   -h         show this help message and exit\n"
    "   -s word    append word to word set for generation (can have multiple -s)\n"
    "   -i infile  append every line in file to word set\n"
    "   -o outfile file to write to (default: stdout)\n"
    "   -v         print status\n"
    "   -l         add lowercase word variation to word set\n"
    "   -u         add UPPERCASE word variation to word set\n"
    "   -t         add Title word variation to word set\n"
    "   -r         reverse string\n"
    "   -1         word + int\n"
    "   -2         int + word\n"
    "   -3         int + word + int\n"
    "   -y         just generate [0](0-100) and years 1800-2025\n"
    "   -m number  min password len (default: 8)\n"
    "   -c         check if output is unique, don't generate dupes, slower\n"
    "   -d         double mode -- permutate every word in word set len 2 (<str><str>)\n"
    "   -z         double mode -- just do (<str1><str1>)\n"
    "   -j string  double mode -- join string (<str><join><str>)\n"
    "\n"
    ;
}
bool load_args(int& argc, char**& argv) {
    int opt;
    while ((opt = getopt(argc, argv, "hs:i:o:vlutr123cdzyjm:")) != -1) {
        switch (opt) {
            case 'h': print_help(); break;
            case 's': append_word_from_args(optarg); break;
            case 'i': input_file = optarg; break;
            case 'o': output_file = optarg; to_file = true; break;
            case 'v': verbose = true; break;
            case 'l': lower = true; break;
            case 'u': upper = true; break;
            case 't': title = true; break;
            case 'r': reverse = true; break;
            case '1': wordint = true; break;
            case '2': intword = true; break;
            case '3': intwordint = true; break;
            case 'c': check = true; break;
            case 'd': double_mode = true; break;
            case 'z': double_small = true; break;
            case 'y': year = true; break;
            case 'j': double_join = optarg; break;
            case 'm': min_len = std::stoi(optarg); break;
            default: std::cerr << "Invalid option\n"; return false;
        }
    }
    return true;
}

// Main function
int main(int argc, char** argv) {

    if (!load_args(argc, argv)) { return 1; }

    for (const auto& word : arg_words) { add_word_variations(word); }

    if (verbose) {
        print_args();
        print_words();
    }

    // Load words from file if specified
    if (!input_file.empty()) { load_input_file(input_file); }

    // Open output file if specified
    if (to_file) { output.open(output_file); }

    // Generate wordlist
    if (double_mode) { namename(); }
    else             { wordnum();  }

    if (to_file) { output.close(); }

    return 0;
}

