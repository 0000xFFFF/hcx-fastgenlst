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

#include "utf8_strings.h"

// Global settings and flags
std::set<std::string> arg_words;
std::set<std::string> words;
std::unordered_set<std::string> output_uniq;
std::mutex mtx;
bool verbose = false, verbose_more = false,
     lower = false, upper = false, title = false, reverse = false,
     wordint = false, intword = false, intwordint = false,
     to_file = false,
     check = false,
     double_mode = false, double_small = false,
     year = false;

std::string double_join = "";
int min_len = 8;
std::string input_file, output_file;
std::ofstream output;

void print_args() {
    std::cerr << "vlutr123cdzy\n"
    << verbose << lower << upper << title << reverse << wordint << intword << intwordint << check << double_mode << double_small << year << std::endl;
    if (!input_file.empty()) { std::cerr << "input file: " << input_file << std::endl; }
    if (to_file) { std::cerr << "output file: " << output_file << std::endl; }
    else         { std::cerr << "printing to stdout" << std::endl; }
    std::cerr << "word set length: " << words.size() << std::endl;
}

// Add variations of a word (lowercase, uppercase, title case, reverse)
void add_word_variations(const std::string& word) {
    words.insert(word);
    if (lower) { words.insert(utf8_tolower(word)); }
    if (upper) { words.insert(utf8_toupper(word)); }
    if (title) { words.insert(utf8_titlecase(word)); }
    if (reverse) { words.insert(utf8_reverse(word)); }
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

    if (utf8_strlen(word) < min_len) return;

    if (check) {
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

        out_minlen_uniq(word); // just print out var
        if (wordint)    { generate_suffixes(word, generate_wi);  }
        if (intword)    { generate_suffixes(word, generate_iw);  }
        if (intwordint) { generate_suffixes(word, generate_iwi); }
    }

    if (verbose) { progress.finish(); }
}

// Handle double mode generation (word1 + join + word2)
void namename() {

    int words_n = words.size();
    Progress progress(words_n);
    if (verbose) { progress.start(); }

    int count = 0;
    for (const auto& word : words) {
        out_minlen_uniq(word + double_join + word);
    }

    // TODO: try to speed this up it's really slow -- python itertools is faster :/
    if (!double_small) {
        for (const auto& word : words) {
            progress.update(++count);
            for (const auto& word2 : words) {
                if (word != word2) {
                    out_minlen_uniq(word + double_join + word2);
                }
            }
        }
    }

    if (verbose) { progress.finish(); }
}

// Load words from input file
void load_input_file(const std::string& input_file) {
    std::ifstream infile(input_file);
    if (!infile) { std::cerr << "can't load file: " << input_file << std::endl; }
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
    "   -v         be verbose (print status)\n"
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


void print_loaded_words() {
    for (const auto& word : words) {
        std::cerr << word << " -- " <<  utf8_strlen(word) << std::endl;
    }
}

// Main function
int main(int argc, char** argv) {

    if (!load_args(argc, argv)) { return 1; }

    for (const auto& word : arg_words) { add_word_variations(word); }

    // Load words from file if specified
    if (!input_file.empty()) { load_input_file(input_file); }

    if (verbose) { print_args(); }

    print_loaded_words();

    // Open output file if specified
    if (to_file) { output.open(output_file); }

    // Generate wordlist
    if (double_mode) { namename(); }
    else             { wordnum();  }

    if (to_file) { output.close(); }

    return 0;
}

