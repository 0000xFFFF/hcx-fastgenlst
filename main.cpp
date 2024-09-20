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

// Global settings and flags
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

void print_args() {
    std::cerr << "vlutr123cdzy\n" <<
    verbose << lower << upper << title << reverse << wordint << intword << intwordint << check << double_mode << double_small << year << std::endl;
}

void print_words() {
    std::cerr << "==[ BEGIN WORDSET ]==" << std::endl;
    for (const auto& word : words) { std::cerr << word << std::endl; }
    std::cerr << "==[ END WORDSET ]==" << std::endl;
}

// Add variations of a word (lowercase, uppercase, title case, reverse)
void add_word_variations(const std::string& word) {
    words.insert(word);
    if (lower) {
        std::string lower_word = word;
        std::transform(lower_word.begin(), lower_word.end(), lower_word.begin(), ::tolower);
        words.insert(word);
    }
    if (upper) {
        std::string upper_word = word;
        std::transform(upper_word.begin(), upper_word.end(), upper_word.begin(), ::toupper);
        words.insert(upper_word);
    }
    if (title) {
        std::string title_word = word;
        title_word[0] = toupper(title_word[0]);
        words.insert(title_word);
    }
    if (reverse) {
        std::string reversed_word = word;
        std::reverse(reversed_word.begin(), reversed_word.end());
        words.insert(reversed_word);
    }
}

// Progress bar class to show processing status
class ProgressBar {
public:
    ProgressBar(int total, int length = 25) : total(total), length(length), running(true), current(0) {}

    void start() {
        thread = std::thread([this]() {
            while (running) {
                display();
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        });
    }

    void update(int progress) {
        std::lock_guard<std::mutex> lock(mtx);
        current = progress;
    }

    void stop() {
        running = false;
        if (thread.joinable()) { thread.join(); }
    }

    void display() {
        int filled_length = (current * length) / total;
        std::string bar(filled_length, '#');
        bar.resize(length, '-');
        std::cerr << "\r[" << bar << "] " << std::setw(3) << (current * 100) / total
                  << "% (" << current << "/" << total << ") words processed" << std::flush;
    }

private:
    int total, length, current;
    bool running;
    std::thread thread;
};

// Output function (to file or stdout)
void out_minlen_uniq(const std::string& word) {
    if (word.length() < min_len) return;

    if (check) {
        std::lock_guard<std::mutex> lock(mtx);
        if (output_uniq.find(word) != output_uniq.end()) return;
        output_uniq.insert(word);
    }

    if (to_file) { output    << word << '\n'; }
    else         { std::cout << word << '\n'; }
    // todo: might need to flush?
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

    ProgressBar progress(words_n);
    if (verbose) { progress.start(); }

    int count = 0;
    for (const auto& word : words) {
        if (verbose) { progress.update(++count); }

        if (wordint)    { generate_suffixes(word, generate_wi);  }
        if (intword)    { generate_suffixes(word, generate_iw);  }
        if (intwordint) { generate_suffixes(word, generate_iwi); }
    }

    if (verbose) { progress.stop(); }
}

// Handle double mode generation (word1 + join + word2)
void namename() {
    int words_n = words.size();

    if (verbose) {
        ProgressBar progress(words_n * (words_n - 1));
        progress.start();

        int count = 0;
        for (const auto& word : words) {
            out_minlen_uniq(word + double_join + word);
            for (const auto& word2 : words) {
                if (word != word2) {
                    out_minlen_uniq(word + double_join + word2);
                }
            }
            progress.update(++count);
        }

        progress.stop();
    } else {
        for (const auto& word : words) {
            out_minlen_uniq(word + double_join + word);
            for (const auto& word2 : words) {
                if (word != word2) {
                    out_minlen_uniq(word + double_join + word2);
                }
            }
        }
    }
}

// Load words from input file
void load_input_file(const std::string& input_file) {
    std::ifstream infile(input_file);
    std::string line;
    while (std::getline(infile, line)) { add_word_variations(line); }
}

bool load_args(int& argc, char**& argv) {
    int opt;
    while ((opt = getopt(argc, argv, "s:i:o:vlutr123cdzyjm:")) != -1) {
        switch (opt) {
            case 's': add_word_variations(optarg); break;
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

    print_args();
    print_words();

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

