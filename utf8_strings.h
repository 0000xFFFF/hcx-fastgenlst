#ifndef UTF8_STRINGS
#define UTF8_STRINGS

#include <cstddef>
#include <cstring>
#include <string>
#include "UtfConv.h"

size_t utf8_strlen(const std::string&);
std::string utf8_tolower(const std::string&);
std::string utf8_toupper(const std::string&);
std::string utf8_titlecase(const std::string&);
std::string utf8_reverse(const std::string&);

#endif
