#include <iostream>
#include <fstream>
#include <string>

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

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file>\n";
        return 1;
    }

    const std::string filename(argv[1]);
    std::ifstream infile(filename, std::ifstream::binary);
    if (!infile) { std::cerr << "Error: Could not open file " << argv[1] << '\n'; return 1; }

    std::string line;
    while (std::getline(infile, line)) {
        int length = utf8_strlen(line);
        if (length == -1) { std::cout << line << std::endl; }
    }

    infile.close();
    return 0;
}
