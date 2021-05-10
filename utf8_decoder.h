// The MIT License (MIT)

// Copyright (c) 2021 Pierre Pharel Angoro Abiaga

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifndef UTF8_DECODER
#define UTF8_DECODER

#if defined(_MSC_VER) && (_MSC_VER < 1920)
typedef __int32 int32_t;
#else
#include <stdint.h>
#endif

#include <stdio.h>

#ifdef __cplusplus
#define Cast(type, var) static_cast<type>(var)
#else
#define Cast(type, var) (type)var
#endif

#include <stdlib.h>
#include <string.h>

// str bytes beginning
#define LATIN_EXTRA_BEGIN ("110")
#define BASIC_MULTILINGUAL_BEGIN ("1110")
#define OTHERS_PLANES_UNICODE_BEGIN ("11110")
#define SECONDARY_CHAR_BEGIN ("10")

#define END ('\0')
#define UTF8_BAD_CHAR 0
#define UTF8_GOOD_CHAR 1

typedef enum
{
    utf8_USASCII_t,
    utf8_LatinExtra_t,
    utf8_BasicMultiLingual_t,
    utf8_OthersPlanesUnicode_t,
    utf8_OutRange_t
} UTF8Type_t;

static UTF8Type_t utf8type(const char *hex_str, char *dest);

static void hex_to_bytes_str(const char *hex_str, char *bytes_str);

static void bytes_to_utf8chr_str(UTF8Type_t type, char *utf8_chr_str);

static void str_to_bit_decoded(const char *utf8_chr_str, char *utf8_str);

static void utf8decode(const char *hex_str, char* dest);

static short utf8valid(const char *str);

static int32_t utf8codepoint(const char *str);

static void utf8chr(const int32_t codepoint, char *dest);

static UTF8Type_t utf8type(const char *hex_str, char *dest)
{
    int32_t codepoint = 0;
    short shift = 0;

    for(const char *s = hex_str; *s != END; ++ s)
    {
        switch(*s)
        {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': codepoint = ((codepoint << shift) | (*s - '0'));
                    break;

            case 'A':
            case 'B':
            case 'C':
            case 'D':
            case 'E':
            case 'F': codepoint = ((codepoint << shift) | (*s - 'A' + 10));
                    break;

            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f': codepoint = ((codepoint << shift) | (*s - 'a' + 10));
                    break;
        }

        shift = 4;
    }

    if(dest != NULL)
        dest[0] = codepoint;

    if(codepoint >= 0x0000 && codepoint <= 0x007f)
        return utf8_USASCII_t;
    else if(codepoint > 0x007f && codepoint <= 0x07ff)
        return utf8_LatinExtra_t;
    else if(codepoint > 0x07ff && codepoint <= 0xffff)
        return utf8_BasicMultiLingual_t;
    else if(codepoint > 0xffff && codepoint <= 0x10ffff)
        return utf8_OthersPlanesUnicode_t;

    return utf8_OutRange_t;
}

static void hex_to_bytes_str(const char *hex_str, char *bytes_str)
{
    const char *s = hex_str;

    while(*s != END)
    {
        switch(*s)
        {
            case '0': utf8d_append(bytes_str, "0000");
                break;
            case '1': utf8d_append(bytes_str, "0001");
                break;
            case '2': utf8d_append(bytes_str, "0010");
                break;
            case '3': utf8d_append(bytes_str, "0011");
                break;
            case '4': utf8d_append(bytes_str, "0100");
                break;
            case '5': utf8d_append(bytes_str, "0101");
                break;
            case '6': utf8d_append(bytes_str, "0110");
                break;
            case '7': utf8d_append(bytes_str, "0111");
                break;
            case '8': utf8d_append(bytes_str, "1000");
                break;
            case '9': utf8d_append(bytes_str, "1001");
                break;
            case 'a':
            case 'A': utf8d_append(bytes_str, "1010");
                break;
            case 'b':
            case 'B': utf8d_append(bytes_str, "1011");
                break;
            case 'c':
            case 'C': utf8d_append(bytes_str, "1100");
                break;
            case 'd':
            case 'D': utf8d_append(bytes_str, "1101");
                break;
            case 'e':
            case 'E': utf8d_append(bytes_str, "1110");
                break;
            case 'f':
            case 'F': utf8d_append(bytes_str, "1111");
                break;
        }

        ++ s;
    }
}

static void bytes_to_utf8chr_str(UTF8Type_t type, char *utf8_chr_str)
{
    char bytes_str[33];

    switch(type)
    {
        case utf8_LatinExtra_t:
        {
            utf8_move(5, utf8_chr_str, bytes_str);
            utf8d_append(utf8_chr_str, LATIN_EXTRA_BEGIN);
            for(short i = 0; bytes_str[i] != END; ++ i)
                if(i < 5) // first char
                {
                    utf8_chr_str[i + 3] = bytes_str[i];
                    utf8_chr_str[i + 4] = END;
                }
                else if(i == 5) // transition
                {
                    utf8d_append(utf8_chr_str, SECONDARY_CHAR_BEGIN);
                    utf8_chr_str[i + 5] = bytes_str[i];
                    utf8_chr_str[i + 6] = END;
                }
                else // last char
                {
                    utf8_chr_str[i + 5] = bytes_str[i];
                    utf8_chr_str[i + 6] = END;
                }

            break;
        }

/*
        case utf8_BasicMultiLingual_t:
        {
            utf8_move(0, utf8_chr_str, bytes_str);
            utf8d_append(utf8_chr_str, BASIC_MULTILINGUAL_BEGIN);
            for(short i = 0; bytes_str[i] != END; ++ i)
                if(i < 4) // first char
                {
                    utf8_chr_str[i + 4] = bytes_str[i];
                    utf8_chr_str[i + 5] = END;
                }
                else if(i == 4) // transition
                {
                    utf8d_append(utf8_chr_str, SECONDARY_CHAR_BEGIN);
                    utf8_chr_str[i + 6] = bytes_str[i];
                    utf8_chr_str[i + 7] = END;
                }
                else if(i > 4 && i < 10) // second char
                {
                    utf8_chr_str[i + 6] = bytes_str[i];
                    utf8_chr_str[i + 7] = END;
                }
                else if(i == 10) // transition
                {
                    utf8d_append(utf8_chr_str, SECONDARY_CHAR_BEGIN);
                    utf8_chr_str[i + 8] = bytes_str[i];
                    utf8_chr_str[i + 9] = END;
                }
                else // last char
                {
                    utf8_chr_str[i + 8] = bytes_str[i];
                    utf8_chr_str[i + 9] = END;
                }

            break;
        }

        case utf8_OthersPlanesUnicode_t:
        {
            utf8_move(3, utf8_chr_str, bytes_str);
            utf8d_append(utf8_chr_str, OTHERS_PLANES_UNICODE_BEGIN);
            for(short i = 0; bytes_str[i] != END; ++ i)
                if(i < 3) // first char
                {
                    utf8_chr_str[i + 5] = bytes_str[i];
                    utf8_chr_str[i + 6] = END;
                }
                else if(i == 3) // transition
                {
                    utf8d_append(utf8_chr_str, SECONDARY_CHAR_BEGIN);
                    utf8_chr_str[i + 7] = bytes_str[i];
                    utf8_chr_str[i + 8] = END;
                }
                else if(i > 3 && i < 9)
                {
                    utf8_chr_str[i + 7] = bytes_str[i];
                    utf8_chr_str[i + 8] = END;
                }
                else if(i == 9) // transition
                {
                    utf8d_append(utf8_chr_str, SECONDARY_CHAR_BEGIN);
                    utf8_chr_str[i + 9] = bytes_str[i];
                    utf8_chr_str[i + 10] = END;
                }
                else if(i > 9 && i < 15)
                {
                    utf8_chr_str[i + 9] = bytes_str[i];
                    utf8_chr_str[i + 10] = END;
                }
                else if(i == 15) // transition
                {
                    utf8d_append(utf8_chr_str, SECONDARY_CHAR_BEGIN);
                    utf8_chr_str[i + 11] = bytes_str[i];
                    utf8_chr_str[i + 12] = END;
                }
                else // last char
                {
                    utf8_chr_str[i + 11] = bytes_str[i];
                    utf8_chr_str[i + 12] = END;
                    printf("s : %s\n", utf8_chr_str);
                }

            break;
        }
    */
    }
}

static void utf8decode(const char *hex_str, char* dest)
{
    UTF8Type_t type = utf8type(hex_str, NULL);
    unsigned char buffer[5] = {0};

    switch(type)
    {
        case utf8_USASCII_t:
        {
            utf8type(hex_str, dest);
            dest[1] = END;
            break;
        }

        case utf8_LatinExtra_t:
        case utf8_BasicMultiLingual_t:
        case utf8_OthersPlanesUnicode_t:
        {
            hex_to_bytes_str(hex_str, utf8_chr_str);
            bytes_to_utf8chr_str(type, utf8_chr_str);
            str_to_bit_decoded(utf8_chr_str, dest);
            break;
        }

        case utf8_OutRange_t:
            dest[0] = END;
            break;
    }
}

static short utf8valid(const char *str)
{
    const char *s = str;

    while(*s != END)
    {
        if(0xf0 == (0xf8 & *s))
        {
            // ensure each of the 3 following bytes in this 4-byte
            // utf8 codepoint began with 0b10xxxxxx
            if((0x80 != (0xc0 & s[1])) || (0x80 != (0xc0 & s[2])) || (0x80 != (0xc0 & s[3])))
                return UTF8_BAD_CHAR;

            // ensure that our utf8 codepoint ended after 4 bytes
            if(0x80 == (0xc0 & s[4]))
                return UTF8_BAD_CHAR;

            // ensure that the top 5 bits of this 4-byte utf8
            // codepoint were not 0, as then we could have used
            // one of the smaller encodings
            if((0 == (0x07 & s[0])) && (0 == (0x30 & s[1])))
                return UTF8_BAD_CHAR;

            // 4-byte utf8 code point (began with 0b11110xxx)
            s += 4;
        }
        else if(0xe0 == (0xf0 & *s))
        {
            // ensure each of the 2 following bytes in this 3-byte
            // utf8 codepoint began with 0b10xxxxxx
            if((0x80 != (0xc0 & s[1])) || (0x80 != (0xc0 & s[2])))
                return UTF8_BAD_CHAR;

            // ensure that our utf8 codepoint ended after 3 bytes
            if(0x80 == (0xc0 & s[3]))
                return UTF8_BAD_CHAR;

            // ensure that the top 5 bits of this 3-byte utf8
            // codepoint were not 0, as then we could have used
            // one of the smaller encodings
            if((0 == (0x0f & s[0])) && (0 == (0x20 & s[1])))
                return UTF8_BAD_CHAR;

            // 3-byte utf8 code point (began with 0b1110xxxx)
            s += 3;
        }
        else if(0xc0 == (0xe0 & *s))
        {
            // ensure the 1 following byte in this 2-byte
            // utf8 codepoint began with 0b10xxxxxx
            if(0x80 != (0xc0 & s[1]))
                return UTF8_BAD_CHAR;

            // ensure that our utf8 codepoint ended after 2 bytes
            if(0x80 == (0xc0 & s[2]))
                return UTF8_BAD_CHAR;

            // ensure that the top 4 bits of this 2-byte utf8
            // codepoint were not 0, as then we could have used
            // one of the smaller encodings
            if(0 == (0x1e & s[0]))
                return UTF8_BAD_CHAR;

            // 2-byte utf8 code point (began with 0b110xxxxx)
            s += 2;
        }
        else if(0x00 == (0x80 & *s))
        {
            // 1-byte ascii (began with 0b0xxxxxxx)
            s += 1;
        }
        else
        {
            // we have an invalid 0b1xxxxxxx utf8 code point entry
            return UTF8_BAD_CHAR;
        }
    }

    return UTF8_GOOD_CHAR;
}

static int32_t utf8codepoint(const char *str)
{
    int32_t codepoint = -1;
    const char *s = str;

    if(utf8valid(str))
        while(*s != END)
        {
            if(0xf0 == (0xf8 & *s))
            {
                // four bytes
                codepoint = ((0x07 & s[0]) << 18) | ((0x3f & s[1]) << 12) | ((0x3f & s[2]) << 6) | (0x3f & s[3]);
                s += 4;
            }
            else if(0xe0 == (0xf0 & *s))
            {
                // three bytes
                codepoint = ((0x0f & s[0]) << 12) | ((0x3f & s[1]) << 6) | (0x3f & s[2]);
                s += 3;
            }
            else if(0xc0 == (0xe0 & *s))
            {
                printf("two bit : %d\n", s[0]);
                // two bytes
                codepoint = ((0x1f & s[0]) << 6) | (0x3f & s[1]);
                s += 2;
            }
            else if(0x00 == (0x80 & *s))
            {
                // one byte
                printf("one bit : %d\n", s[0]);
                codepoint = s[0];
                ++ s;
            }
        }

    return codepoint;
}

static void utf8chr(const int32_t codepoint, char *dest)
{
    if(codepoint <= 0x10ffff)
    {

    }
}

#undef END
#undef LATIN_EXTRA_BEGIN
#undef BASIC_MULTILINGUAL_BEGIN
#undef OTHERS_PLANES_UNICODE_BEGIN
#undef SECONDARY_CHAR_BEGIN
#undef UTF8_BAD_CHAR
#undef UTF8_GOOD_CHAR

#endif