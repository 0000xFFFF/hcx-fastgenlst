#include "utf8_strings.h"

// TODO: if -1 try to get len of string anyway
// TODO: try to make smiliar func like: python's utf8 'ignore errors' str(len())
// TODO: compare my new version with python's
size_t utf8_strlen(const std::string& input) {
    size_t length = 0;
    for (size_t i = 0; i < input.length(); ) {
        unsigned char c = input[i];
        if      (c <= 0x7F)              { i += 1;    } // 1-byte character (ASCII)
        else if (c >= 0xC2 && c <= 0xDF) { i += 2;    } // 2-byte character
        else if (c >= 0xE0 && c <= 0xEF) { i += 3;    } // 3-byte character
        else if (c >= 0xF0 && c <= 0xF4) { i += 4;    } // 4-byte character
        else                             { return -1; } // Invalid UTF-8 sequence encountered at byte i
        ++length;
    }
    return length;
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

void utf8rev(char *str) {
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

std::string utf8_reverse(const std::string& input) {
    char* buff = new char[input.length()+1];
    std::strcpy(buff, input.c_str());
    utf8rev(buff);
    std::string reversed(buff);
    delete[] buff;
    return reversed;
}

