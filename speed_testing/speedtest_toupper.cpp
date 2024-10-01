#include <iostream>
#include <fstream>
#include <vector>
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

#include "../utf8_strings.h"
#include "../UtfConv.h"

class Timer {
public:
    // start timer
    Timer() : start_time(std::chrono::high_resolution_clock::now()) {}

    // end timer
    ~Timer() {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

        std::cerr << "took: " << duration.count() << " microseconds" << std::endl;
    }

private:
    std::chrono::high_resolution_clock::time_point start_time;
};


// Load words from input file
void process_file_cpp(const std::string& input_file) {
    std::ifstream infile(input_file);
    if (!infile) { std::cerr << "can't load file: " << input_file << std::endl; }
    std::string line;
    while (std::getline(infile, line)) {
        
        std::string upper = utf8_toupper(line);
    }
}


void process_file_c(const char* filename) {

    FILE* fp = fopen(filename, "r");
    if (!fp) { perror("Error opening file"); return; }

    char *line = NULL;
    size_t n = 0;
    while (getline(&line, &n, fp) != -1) {
        Utf8Char* utf8str = Utf8StrMakeUprUtf8Str((Utf8Char*)line);
        //printf("%s", line);
        free(utf8str); // Don't forget to free the allocated buffer
        n = 0; // Reset buffer size for next iteration
    }

    fclose(fp);
}


// Main function
int main(int argc, char** argv) {

    {
        std::cout << "test1" << std::endl;
        Timer t;
        process_file_cpp("english.txt");
    }

    {
        std::cout << "test2" << std::endl;
        Timer t;
        process_file_c("english.txt");
    }

    return 0;
}

