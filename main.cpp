#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <algorithm>
#include <iterator>
#include <string>
#include <thread>
#include <mutex>
#include <chrono>
#include <getopt.h>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <unordered_set>
#include <iterator>
#include <numeric>

// Global settings and flags
std::set<std::string> words;
std::unordered_set<std::string> output_uniq;
std::mutex mtx;
bool verbose = false, lower = false, upper = false, title = false, reverse = false;
bool wordint = false, intword = false, intwordint = false, check = false, to_file = false;
bool double_mode = false, double_small = false, year = false;
std::string double_join = "";
int min_len = 8;
std::string input_file, output_file;

// Add variations of a word (lowercase, uppercase, title case, reverse)
void add_word_variations(const std::string& word) {
    words.insert(word);
    if (lower) words.insert(word);
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
    ProgressBar(int total, int length = 25)
        : total(total), length(length), running(true), current(0) {}

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
        if (thread.joinable()) {
            thread.join();
        }
    }

    void display() {
        int filled_length = (current * length) / total;
        std::string bar(filled_length, '#');
        bar.resize(length, '-');
        std::cout << "\r[" << bar << "] " << std::setw(3) << (current * 100) / total
                  << "% (" << current << "/" << total << ") words processed" << std::flush;
    }

private:
    int total, length, current;
    bool running;
    std::thread thread;
};

// Output function (to file or stdout)
void write_output(const std::string& word, std::ofstream& out_file, bool to_file) {
    if (word.length() < min_len) return;

    if (check) {
        std::lock_guard<std::mutex> lock(mtx);
        if (output_uniq.find(word) != output_uniq.end()) return;
        output_uniq.insert(word);
    }

    if (to_file) {
        out_file << word << '\n';
    } else {
        std::cout << word << '\n';
    }
}

// Generate suffixes for a word (e.g., word + "123", word + "!")
void generate_suffixes(const std::string& word, std::ofstream& out_file, bool to_file) {
    std::vector<std::string> suffixes = {"!", "!!", "!!!", ".", "..", "...", "@", "@@", "@@@", "#",
                                         "12345", "123456", "1234567", "12345678", "123456789", "1234567890"};
    for (const auto& suffix : suffixes) {
        write_output(word + suffix, out_file, to_file);
        std::string reversed_suffix = suffix;
        std::reverse(reversed_suffix.begin(), reversed_suffix.end());
        write_output(word + reversed_suffix, out_file, to_file);
    }

    for (int i = 0; i < 10; ++i) {
        write_output(word + "00" + std::to_string(i), out_file, to_file);
    }

    if (year) {
        for (int i = 1800; i <= 2025; ++i) {
            write_output(word + std::to_string(i), out_file, to_file);
        }
    }
}

// Handle word number generation (word + int, int + word, int + word + int)
void wordnum(std::ofstream& out_file, bool to_file) {
    int words_n = words.size();

    if (verbose) {
        ProgressBar progress(words_n);
        progress.start();

        int count = 0;
        for (const auto& word : words) {
            generate_suffixes(word, out_file, to_file);
            if (verbose) {
                progress.update(++count);
            }
        }

        progress.stop();
    } else {
        for (const auto& word : words) {
            generate_suffixes(word, out_file, to_file);
        }
    }
}

// Handle double mode generation (word1 + join + word2)
void namename(std::ofstream& out_file, bool to_file) {
    int words_n = words.size();

    if (verbose) {
        ProgressBar progress(words_n * (words_n - 1));
        progress.start();

        int count = 0;
        for (const auto& word : words) {
            write_output(word + double_join + word, out_file, to_file);
            for (const auto& word2 : words) {
                if (word != word2) {
                    write_output(word + double_join + word2, out_file, to_file);
                }
            }
            progress.update(++count);
        }

        progress.stop();
    } else {
        for (const auto& word : words) {
            write_output(word + double_join + word, out_file, to_file);
            for (const auto& word2 : words) {
                if (word != word2) {
                    write_output(word + double_join + word2, out_file, to_file);
                }
            }
        }
    }
}

// Load words from input file
void load_input_file(const std::string& input_file) {
    std::ifstream infile(input_file);
    std::string line;
    while (std::getline(infile, line)) {
        add_word_variations(line);
    }
}

// Main function
int main(int argc, char** argv) {
    // Parse command-line arguments
    int opt;
    while ((opt = getopt(argc, argv, "s:i:o:vlu1r2cdzytjm:")) != -1) {
        switch (opt) {
            case 's': add_word_variations(optarg); break;
            case 'i': input_file = optarg; break;
            case 'o': output_file = optarg; to_file = true; break;
            case 'v': verbose = true; break;
            case 'l': lower = true; break;
            case 'u': upper = true; break;
            case 'r': reverse = true; break;
            case '1': wordint = true; break;
            case '2': intword = true; break;
            case '3': intwordint = true; break;
            case 'c': check = true; break;
            case 'd': double_mode = true; break;
            case 'z': double_small = true; break;
            case 'y': year = true; break;
            case 't': title = true; break;
            case 'j': double_join = optarg; break;
            case 'm': min_len = std::stoi(optarg); break;
            default: std::cerr << "Invalid option\n"; return 1;
        }
    }

    // Load words from file if specified
    if (!input_file.empty()) {
        load_input_file(input_file);
    }

    // Open output file if specified
    std::ofstream out_file;
    if (to_file) {
        out_file.open(output_file);
    }

    // Generate wordlist
    if (double_mode) {
        namename(out_file, to_file);
    } else {
        wordnum(out_file, to_file);
    }

    if (to_file) {
        out_file.close();
    }

    return 0;
}

