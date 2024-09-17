#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <sys/resource.h>
#include <unistd.h>

size_t utf8_strlen(const std::string& str) {
    size_t length = 0;
    for (size_t i = 0; i < str.length(); ) {
        unsigned char c = str[i];
        if (c <= 0x7F) {
            // 1-byte character (ASCII)
            i += 1;
        } else if (c >= 0xC2 && c <= 0xDF) {
            // 2-byte character
            i += 2;
        } else if (c >= 0xE0 && c <= 0xEF) {
            // 3-byte character
            i += 3;
        } else if (c >= 0xF0 && c <= 0xF4) {
            // 4-byte character
            i += 4;
        } else {
            //std::cerr << "Invalid UTF-8 sequence encountered at byte " << i << std::endl;
            return -1;
        }
        ++length;
    }
    return length;
}

// System call to Python to get length of string using Python's len() function
size_t get_python_strlen(const std::string& str) {
    // Write the string to a temporary file
    std::ofstream temp_file("temp_input.txt");
    if (!temp_file) {
        std::cerr << "Error: Could not create temporary file\n";
        return -1;
    }
    temp_file << str;
    temp_file.close();

    // Create a Python command to read from the file and get the length
    std::string command = "python3 -c \"with open('temp_input.txt', 'r', encoding='utf-8', errors='ignore') as f: print(len(f.read()))\"";
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) return -1;
    
    char buffer[128];
    std::string result = "";
    
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    
    pclose(pipe);

    // Remove the temporary file
    std::filesystem::remove("temp_input.txt");

    // Convert result to an integer (length)
    return std::stoi(result);
}

size_t get_file_line_count(const std::string& filename) {
    std::ifstream infile(filename);
    if (!infile) {
        std::cerr << "Error: Could not open file " << filename << '\n';
        return 0;
    }

    size_t line_count = 0;
    std::string line;
    while (std::getline(infile, line)) {
        ++line_count;
    }

    infile.close();
    return line_count;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file>\n";
        return 1;
    }

    const std::string filename(argv[1]);

    int i = 0;
    int n = get_file_line_count(filename);

    // Open file from command-line argument
    std::ifstream infile(filename, std::ifstream::binary);
    if (!infile) {
        std::cerr << "Error: Could not open file " << argv[1] << '\n';
        return 1;
    }


    std::string line;
    while (std::getline(infile, line)) {
        i++;
        std::cout << i << "/" << n << " -- '" << line << "' -- "; 
        std::cout.flush();
        size_t cpp_length = utf8_strlen(line);  // C++ string length using utf8_strlen
        std::cout << cpp_length << " ";
        std::cout.flush();
        size_t python_length = get_python_strlen(line);  // Python len() length
        std::cout << python_length << " " << std::endl;
        if (cpp_length != python_length) {
            std::cerr << i << "/" << n << " -- '" << line << "' -- " << cpp_length << " " << python_length << std::endl;
        }
    }

    infile.close();
    return 0;
}

