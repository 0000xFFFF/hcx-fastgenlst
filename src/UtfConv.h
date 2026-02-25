/*******************************************************************************************

Function set for UTF StrLwr, StrUpr and case insensitive StrCmp and  StrStr

by Jan Bergström, Alphabet AB, 2021 Open source code

2021-10-21

*******************************************************************************************/
#ifndef UTF_CONV
#define UTF_CONV

#include <stdlib.h> 
#include <string.h>

#include <stdint.h>
#define Utf32Char uint32_t		// min 4 bytes (int32_t), uint32_t suggested
#define Utf16Char wchar_t		// min 2 bytes (int16_t), wchar_t suggested 
#define Utf8Char unsigned char	// must be 1 byte, 8 bits, can be char, the UTF consortium specify unsigned 

// Utf 32
size_t StrLenUtf32(const Utf32Char* str);
Utf32Char* StrCpyUtf32(Utf32Char* dest, const Utf32Char* src);
Utf32Char* StrCatUtf32(Utf32Char* dest, const Utf32Char* src);
int StrnCmpUtf32(const Utf32Char* Utf32s1, const Utf32Char* Utf32s2, size_t ztCount);
int StrCmpUtf32(const Utf32Char* Utf32s1, const Utf32Char* Utf32s2);
Utf32Char* StrToUprUtf32(Utf32Char* pUtf32);
Utf32Char* StrToLwrUtf32(Utf32Char* pUtf32);
int StrnCiCmpUtf32(const Utf32Char* Utf32s1, const Utf32Char* Utf32s2, size_t ztCount);
int StrCiCmpUtf32(const Utf32Char* Utf32s1, const Utf32Char* Utf32s2);
Utf32Char* StrCiStrUtf32(const Utf32Char* Utf32s1, const Utf32Char* Utf32s2);

// Utf 16
size_t StrLenUtf16(const Utf16Char* str);
Utf16Char* StrCpyUtf16(Utf16Char* dest, const Utf16Char* src);
Utf16Char* StrCatUtf16(Utf16Char* dest, const Utf16Char* src);
int StrnCmpUtf16(const Utf16Char* Utf16s1, const Utf16Char* Utf16s2, size_t ztCount);
int StrCmpUtf16(const Utf16Char* Utf16s1, const Utf16Char* Utf16s2);
size_t CharLenUtf16(const Utf16Char* pUtf16);
Utf16Char* ForwardUtf16Chars(const Utf16Char* pUtf16, size_t ztForwardUtf16Chars);
size_t StrLenUtf32AsUtf16(const Utf32Char* pUtf32);
Utf16Char* Utf32ToUtf16(const Utf32Char* pUtf32);
Utf32Char* Utf16ToUtf32(const Utf16Char* pUtf16);
Utf8Char* Utf16ToUtf8(const Utf16Char* pUtf16);
Utf16Char* Utf16StrMakeUprUtf16Str(const Utf16Char* pUtf16);
Utf16Char* Utf16StrMakeLwrUtf16Str(const Utf16Char* pUtf16);
int StrnCiCmpUtf16(const Utf16Char* pUtf16s1, const Utf16Char* pUtf16s2, size_t ztCount);
int StrCiCmpUtf16(const Utf16Char* pUtf16s1, const Utf16Char* pUtf16s2);
Utf16Char* StrCiStrUtf16(const Utf16Char* pUtf16s1, const Utf16Char* pUtf16s2);

// Utf 8
size_t StrLenUtf8(const Utf8Char* str);
int StrnCmpUtf8(const Utf8Char* Utf8s1, const Utf8Char* Utf8s2, size_t ztCount);
int StrCmpUtf8(const Utf8Char* Utf8s1, const Utf8Char* Utf8s2);
size_t CharLenUtf8(const Utf8Char* pUtf8);
Utf8Char* ForwardUtf8Chars(const Utf8Char* pUtf8, size_t ztForwardUtf8Chars);
size_t StrLenUtf32AsUtf8(const Utf32Char* pUtf32);
Utf8Char* Utf32ToUtf8(const Utf32Char* pUtf32);
Utf32Char* Utf8ToUtf32(const Utf8Char* pUtf8);
Utf16Char* Utf8ToUtf16(const Utf8Char* pUtf8);
Utf8Char* Utf8StrMakeUprUtf8Str(const Utf8Char* pUtf8);
Utf8Char* Utf8StrMakeLwrUtf8Str(const Utf8Char* pUtf8);
int StrnCiCmpUtf8(const Utf8Char* pUtf8s1, const Utf8Char* pUtf8s2, size_t ztCount);
int StrCiCmpUtf8(const Utf8Char* pUtf8s1, const Utf8Char* pUtf8s2);
Utf8Char* StrCiStrUtf8(const Utf8Char* pUtf8s1, const Utf8Char* pUtf8s2);

#endif
