#include <algorithm>
#include <argparse/argparse.hpp>
#include <atomic>
#include <chrono>
#include <cstdlib> // for free()
#include <cstring>
#include <cwchar>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

#include "utf8_strings.h"

#define HCX_FASTGENLST_VERSION "2.0.1"

// Global settings and flags
std::unordered_set<std::string> arg_words;
std::unordered_set<std::string> words;
std::unordered_set<std::string> output_uniq;
bool verbose = false, verbose_more = false, lower = false, upper = false,
     title = false, reverse = false, wordint = false, intword = false,
     intwordint = false, to_file = false, check = false, double_mode = false,
     double_small = false, year = false;

std::string double_join = "";
size_t min_len = 8;
std::string input_file, output_file;
std::ofstream output;

void print_args()
{
    std::cerr << "vlutr123cdzy\n"
              << verbose << lower << upper << title << reverse << wordint
              << intword << intwordint << check << double_mode << double_small
              << year << std::endl;
    if (!input_file.empty()) {
        std::cerr << "input file: " << input_file << std::endl;
    }
    if (to_file) {
        std::cerr << "output file: " << output_file << std::endl;
    }
    else {
        std::cerr << "printing to stdout" << std::endl;
    }
    std::cerr << "word set length: " << words.size() << std::endl;
}

// Add variations of a word (lowercase, uppercase, title case, reverse)
void add_word_variations(const std::string& word)
{
    words.insert(word);
    if (lower) {
        words.insert(utf8_tolower(word));
    }
    if (upper) {
        words.insert(utf8_toupper(word));
    }
    if (title) {
        words.insert(utf8_titlecase(word));
    }
    if (reverse) {
        words.insert(utf8_reverse(word));
    }
}

class Progress {
  public:
    Progress(int total, int length = 25)
        : current(0), total(total), length(length), running(true),
          start_time(std::chrono::steady_clock::now()), prev_time(start_time),
          prev_processed(0) {}

    void start()
    {
        progress_thread = std::thread([this]() {
            while (running) {
                display(current.load());
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        });
    }

    void display(int current)
    {
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_time = now - start_time;
        std::chrono::duration<double> time_delta = now - prev_time;

        double words_per_sec = (current - prev_processed) / time_delta.count();

        // Prevent division by zero
        double percent_complete =
            (total > 0) ? (static_cast<double>(current) / total) * 100.0 : 100.0;

        int filled_length = static_cast<int>(length * percent_complete / 100.0);
        std::string bar = std::string(filled_length, '#') +
                          std::string(length - filled_length, '-');
        {
            std::lock_guard<std::mutex> lock(cerr_mutex);
            std::cerr << "\r[" << bar << "] " << std::fixed << std::setprecision(0)
                      << percent_complete << "% (" << current << "/" << total << ") "
                      << std::fixed << std::setprecision(1) << words_per_sec
                      << " w/s          ";
            std::cerr.flush();
        }

        prev_time = now;
        prev_processed = current;
    }

    void update(int current_value) { current.store(current_value); }

    void stop()
    {
        running = false;
        if (progress_thread.joinable()) {
            progress_thread.join();
        }
        std::cerr << std::endl;
    }

    void finish()
    {
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
    std::mutex cerr_mutex;
};

class Timer {
  public:
    // start timer
    // Timer() : start_time(std::chrono::high_resolution_clock::now()) {}

    // end timer
    //~Timer() { stop(); }

    void start() { start_time = std::chrono::high_resolution_clock::now(); }

    void stop()
    {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            end_time - start_time);

        std::cerr << "took: " << duration.count() << " microseconds" << std::endl;
    }

  private:
    std::chrono::high_resolution_clock::time_point start_time;
};

inline void out_minlen_uniq(const std::string& word)
{

    if (utf8_strlen(word) < min_len)
        return;

    if (check) {
        if (output_uniq.find(word) != output_uniq.end())
            return;
        output_uniq.insert(word);
    }

    if (to_file) {
        output << word << '\n';
    }
    else {
        std::cout << word << '\n';
    }
}

inline void generate_wi(const std::string& w1, const std::string& w2)
{
    out_minlen_uniq(w1 + w2);
}
inline void generate_iw(const std::string& w1, const std::string& w2)
{
    out_minlen_uniq(w2 + w1);
}
inline void generate_iwi(const std::string& w1, const std::string& w2)
{
    out_minlen_uniq(w2 + w1 + w2);
}

// Generate suffixes for a word (e.g., word + "123", word + "!")
void generate_suffixes(
    const std::string& word,
    std::function<void(const std::string&, const std::string&)> callback)
{

    std::vector<std::string> suffixes = {
        "!",
        "!!",
        "!!!",
        ".",
        "..",
        "...",
        "@",
        "@@",
        "@@@",
        "#",
        "12345",
        "123456",
        "1234567",
        "12345678",
        "123456789",
        "1234567890",
        "12345678910",
        "012345",
        "0123456",
        "01234567",
        "012345678",
        "0123456789",
        "01234567890",
        "012345678910",
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
        std::vector<std::string> special = {"123", "1234", "31337", "1337",
                                            "1312", "3112", "403"};
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
void wordnum()
{
    int words_n = words.size();

    Progress progress(words_n);
    if (verbose) {
        progress.start();
    }

    int count = 0;
    for (const auto& word : words) {
        if (verbose) {
            progress.update(++count);
        }

        out_minlen_uniq(word); // just print out var
        if (wordint) {
            generate_suffixes(word, generate_wi);
        }
        if (intword) {
            generate_suffixes(word, generate_iw);
        }
        if (intwordint) {
            generate_suffixes(word, generate_iwi);
        }
    }

    if (verbose) {
        progress.finish();
    }
}

// Handle double mode generation (word1 + join + word2)
void namename()
{

    int words_n = words.size();
    Progress progress(words_n * (words_n - 1));
    if (verbose) {
        progress.start();
    }

    int count = 0;
    for (const auto& word : words) {
        out_minlen_uniq(word + double_join + word);
    }

    // TODO: try to speed this up it's really slow -- python itertools is faster
    // :/
    if (!double_small) {
        for (const auto& word : words) {
            for (const auto& word2 : words) {
                progress.update(++count);
                if (word != word2) {
                    out_minlen_uniq(word + double_join + word2);
                }
            }
        }
    }

    if (verbose) {
        progress.finish();
    }
}

// Load words from input file
void load_input_file(const std::string& input_file)
{
    std::ifstream infile(input_file);
    if (!infile) {
        std::cerr << "can't load file: " << input_file << std::endl;
    }
    std::string line;
    while (std::getline(infile, line)) {
        add_word_variations(line);
    }
}

void append_word_from_args(const std::string word) { arg_words.insert(word); }

void verbosify()
{
    if (verbose) {
        verbose_more = true;
    }
    else {
        verbose = true;
    }
}

std::unique_ptr<argparse::ArgumentParser> create_parser()
{
    auto program = std::make_unique<argparse::ArgumentParser>("hcx-fastgenlst", HCX_FASTGENLST_VERSION);

    program->add_description("Generate a password wordlist from strings (words)");

    program->add_argument("-s", "--string")
        .append()
        .help("append word to word set for generation (can have multiple -s)");

    program->add_argument("-i", "--input")
        .help("append every line in file to word set");

    program->add_argument("-o", "--output")
        .help("file to write to (default: stdout)");

    program->add_argument("-v", "--verbose")
        .default_value(false)
        .implicit_value(true)
        .nargs(0)
        .help("be verbose (print status)");

    program->add_argument("-l", "--lower")
        .default_value(false)
        .implicit_value(true)
        .nargs(0)
        .help("add lowercase word variation to word set");

    program->add_argument("-u", "--upper")
        .default_value(false)
        .implicit_value(true)
        .nargs(0)
        .help("add UPPERCASE word variation to word set");

    program->add_argument("-t", "--title")
        .default_value(false)
        .implicit_value(true)
        .nargs(0)
        .help("add Titlecase word variation to word set");

    program->add_argument("-r", "--reverse")
        .default_value(false)
        .implicit_value(true)
        .nargs(0)
        .help("add reversed word variation to word set");

    program->add_argument("-1", "--wordint")
        .default_value(false)
        .implicit_value(true)
        .nargs(0)
        .help("word + int");

    program->add_argument("-2", "--intword")
        .default_value(false)
        .implicit_value(true)
        .nargs(0)
        .help("int + word");

    program->add_argument("-3", "--intwordint")
        .default_value(false)
        .implicit_value(true)
        .nargs(0)
        .help("int + word + int");

    program->add_argument("-c", "--check")
        .default_value(false)
        .implicit_value(true)
        .nargs(0)
        .help("check if output is unique, don't generate dupes, slower");

    program->add_argument("-d", "--double")
        .default_value(false)
        .implicit_value(true)
        .nargs(0)
        .help(
            "double mode -- permutate every word in word set len 2 (<str><str>)");

    program->add_argument("-z", "--double-small")
        .default_value(false)
        .implicit_value(true)
        .nargs(0)
        .help("double mode -- just do (<str1><str1>)");

    program->add_argument("-y", "--year")
        .default_value(false)
        .implicit_value(true)
        .nargs(0)
        .help("just generate [0](0-100) and years 1800-2025");

    program->add_argument("-j", "--join")
        .default_value(std::string(""))
        .help("double mode -- join string (<str><join><str>)");

    program->add_argument("-m", "--min-len")
        .default_value(size_t(8))
        .scan<'u', size_t>()
        .help("min password len (default: 8)");

    return program;
}

bool load_args(int argc, char** argv)
{
    auto parser = create_parser();

    try {
        parser->parse_args(argc, argv);
    }
    catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << *parser;
        return false;
    }

    // Process arguments
    if (parser->is_used("-s")) {
        auto words_vec = parser->get<std::vector<std::string>>("-s");
        for (const auto& word : words_vec) {
            append_word_from_args(word);
        }
    }

    if (parser->is_used("-i")) {
        input_file = parser->get<std::string>("-i");
    }

    if (parser->is_used("-o")) {
        output_file = parser->get<std::string>("-o");
        to_file = true;
    }

    if (parser->get<bool>("-v")) {
        verbosify();
    }

    lower = parser->get<bool>("-l");
    upper = parser->get<bool>("-u");
    title = parser->get<bool>("-t");
    reverse = parser->get<bool>("-r");
    wordint = parser->get<bool>("-1");
    intword = parser->get<bool>("-2");
    intwordint = parser->get<bool>("-3");
    check = parser->get<bool>("-c");
    double_mode = parser->get<bool>("-d");
    double_small = parser->get<bool>("-z");
    year = parser->get<bool>("-y");

    if (parser->is_used("-j")) {
        double_join = parser->get<std::string>("-j");
    }

    min_len = parser->get<size_t>("-m");

    return true;
}

void print_loaded_words()
{
    std::cerr << "==[ LOADED WORDS BEGIN ]==" << std::endl;
    int i = 0;
    for (const auto& word : words) {
        i++;
        std::cerr << i << "/" << words.size() << " -- " << word << " -- "
                  << utf8_strlen(word) << std::endl;
    }
    std::cerr << "==[ LOADED WORDS END ]==" << std::endl;
}

// Main function
int main(int argc, char** argv)
{
    Timer t; // start timer for whole program

    if (!load_args(argc, argv)) {
        return 1;
    }

    for (const auto& word : arg_words) {
        add_word_variations(word);
    }

    // Load words from file if specified
    if (!input_file.empty()) {
        load_input_file(input_file);
    }

    if (verbose) {
        print_args();
        t.start();
    }
    if (verbose_more) {
        print_loaded_words();
    }

    // Open output file if specified
    if (to_file) {
        output.open(output_file);
    }

    // Generate wordlist
    if (double_mode) {
        namename();
    }
    else {
        wordnum();
    }

    if (to_file) {
        output.close();
    }

    if (verbose) {
        t.stop();
    }

    return 0;
}
