#include "utf8_strings.h"
#include "UtfConv.h"

size_t utf8_strlen(const std::string& input) {
    size_t length = 0;
    for (size_t i = 0; i < input.length(); ) {
        unsigned char c = input[i];
        if      (c <= 0x7F)              { i += 1;   } // 1-byte character (ASCII)
        else if (c >= 0xC2 && c <= 0xDF) { i += 2;   } // 2-byte character
        else if (c >= 0xE0 && c <= 0xEF) { i += 3;   } // 3-byte character
        else if (c >= 0xF0 && c <= 0xF4) { i += 4;   } // 4-byte character
        else                             { continue; } // Invalid UTF-8 sequence encountered at byte i -- just ignore it
        ++length;
    }
    return length;
}

std::string utf8_tolower(const std::string& input) {
    Utf8Char* buff = Utf8StrMakeLwrUtf8Str(reinterpret_cast<const Utf8Char*>(input.c_str()));
    std::string result(reinterpret_cast<char*>(buff));
    free(buff);
    return result;
}

std::string utf8_toupper(const std::string& input) {
    Utf8Char* buff = Utf8StrMakeUprUtf8Str(reinterpret_cast<const Utf8Char*>(input.c_str()));
    std::string result(reinterpret_cast<char*>(buff));
    free(buff);
    return result;
}

std::string utf8_titlecase(const std::string& input) {
    if (input.length() <= 0) { return ""; }

    int i = 0;
    unsigned char c = input[i];
    size_t char_length = 0;
    if      (c <= 0x7F)              { char_length = 1; } // 1-byte character (ASCII)
    else if (c >= 0xC2 && c <= 0xDF) { char_length = 2; } // 2-byte character
    else if (c >= 0xE0 && c <= 0xEF) { char_length = 3; } // 3-byte character
    else if (c >= 0xF0 && c <= 0xF4) { char_length = 4; } // 4-byte character
    else                             { return "";       } // Invalid UTF-8 sequence
    return utf8_toupper(input.substr(i, char_length)) + input.substr(char_length);
}

std::string utf8_reverse(const std::string& input) {
    std::string reversed = input;  // Copy the input to avoid modifying it directly
    char* str = &reversed[0];      // Get the char pointer to the string

    // Reverse the entire string
    char *scanl, *scanr, *scanr2, c;
    for (scanl = str, scanr = str + reversed.length(); scanl < scanr;)
        c = *scanl, *scanl++ = *--scanr, *scanr = c;

    // Reverse each multibyte character
    for (scanl = scanr = str; (c = *scanr++);) {
        if ((c & 0x80) == 0) // ASCII character
            scanl = scanr;
        else if ((c & 0xc0) == 0xc0) { // Start of multibyte character
            scanr2 = scanr;
            switch (scanr - scanl) {
                case 4: c = *scanl, *scanl++ = *--scanr, *scanr = c; // fallthrough
                case 3: // fallthrough
                case 2: c = *scanl, *scanl++ = *--scanr, *scanr = c;
            }
            scanr = scanl = scanr2;
        }
    }

    return reversed;
}
