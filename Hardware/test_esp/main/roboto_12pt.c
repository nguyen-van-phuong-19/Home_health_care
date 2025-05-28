/*******************************************************************************
 * Size: 12 px
 * Bpp: 1
 * Opts: --font main/fonts/Roboto-Regular.ttf --size 12 --bpp 1 --format lvgl --range 0x20-0x7E,0xC0-0x17F --lv-include SSD1306_oled.h --lv-font-name roboto_12pt --output main/roboto_12pt.c
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "SSD1306_oled.h"
#endif

#ifndef ROBOTO_12PT
#define ROBOTO_12PT 1
#endif

#if ROBOTO_12PT

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xfc, 0x80,

    /* U+0022 "\"" */
    0xb6, 0x80,

    /* U+0023 "#" */
    0x28, 0xaf, 0xca, 0x4b, 0xf5, 0x14, 0x50,

    /* U+0024 "$" */
    0x10, 0x43, 0x9b, 0x45, 0x3, 0x3, 0x45, 0xf3,
    0x84,

    /* U+0025 "%" */
    0x60, 0x94, 0x94, 0x68, 0x10, 0x16, 0x29, 0x29,
    0x6,

    /* U+0026 "&" */
    0x30, 0x91, 0x23, 0x86, 0x12, 0xa7, 0x44, 0x74,

    /* U+0027 "'" */
    0xe0,

    /* U+0028 "(" */
    0x29, 0x49, 0x24, 0x91, 0x22,

    /* U+0029 ")" */
    0x89, 0x12, 0x49, 0x25, 0x28,

    /* U+002A "*" */
    0x21, 0x3e, 0xc5, 0x0,

    /* U+002B "+" */
    0x21, 0x9, 0xf2, 0x10, 0x80,

    /* U+002C "," */
    0x54,

    /* U+002D "-" */
    0xe0,

    /* U+002E "." */
    0x80,

    /* U+002F "/" */
    0x10, 0x84, 0x42, 0x11, 0x8, 0x44, 0x0,

    /* U+0030 "0" */
    0x74, 0x63, 0x18, 0xc6, 0x3b, 0x70,

    /* U+0031 "1" */
    0x3c, 0x92, 0x49, 0x20,

    /* U+0032 "2" */
    0x74, 0x62, 0x11, 0x11, 0x88, 0xf8,

    /* U+0033 "3" */
    0x74, 0x42, 0x33, 0x4, 0x31, 0x70,

    /* U+0034 "4" */
    0x8, 0x62, 0x8a, 0x4b, 0x2f, 0xc2, 0x8,

    /* U+0035 "5" */
    0xfc, 0x21, 0xe1, 0x86, 0x31, 0x70,

    /* U+0036 "6" */
    0x32, 0x21, 0xe8, 0xc6, 0x39, 0x70,

    /* U+0037 "7" */
    0xfc, 0x10, 0x82, 0x10, 0x43, 0x8, 0x20,

    /* U+0038 "8" */
    0x74, 0x63, 0xb7, 0x46, 0x31, 0x70,

    /* U+0039 "9" */
    0x74, 0xe3, 0x18, 0xbc, 0x22, 0x60,

    /* U+003A ":" */
    0x82,

    /* U+003B ";" */
    0x87,

    /* U+003C "<" */
    0x9, 0xb1, 0xc3, 0x84,

    /* U+003D "=" */
    0xf8, 0x1, 0xf0,

    /* U+003E ">" */
    0x83, 0x6, 0x7e, 0x40,

    /* U+003F "?" */
    0x72, 0x42, 0x11, 0x10, 0x80, 0x20,

    /* U+0040 "@" */
    0x1f, 0x8, 0x64, 0xb, 0x39, 0x92, 0x64, 0x99,
    0x26, 0x5b, 0x9b, 0x90, 0x6, 0x0, 0xf8,

    /* U+0041 "A" */
    0x10, 0x70, 0xa1, 0x46, 0xc8, 0xbf, 0x41, 0x82,

    /* U+0042 "B" */
    0xfa, 0x18, 0x61, 0xfa, 0x18, 0x61, 0xf8,

    /* U+0043 "C" */
    0x39, 0x18, 0x60, 0x82, 0x8, 0x51, 0x38,

    /* U+0044 "D" */
    0xf2, 0x28, 0x61, 0x86, 0x18, 0x62, 0xf0,

    /* U+0045 "E" */
    0xfc, 0x21, 0xf, 0xc2, 0x10, 0xf8,

    /* U+0046 "F" */
    0xfc, 0x21, 0xf, 0xc2, 0x10, 0x80,

    /* U+0047 "G" */
    0x3c, 0x8e, 0x4, 0x8, 0xf0, 0x60, 0xa1, 0x3c,

    /* U+0048 "H" */
    0x83, 0x6, 0xc, 0x1f, 0xf0, 0x60, 0xc1, 0x82,

    /* U+0049 "I" */
    0xff, 0x80,

    /* U+004A "J" */
    0x4, 0x10, 0x41, 0x4, 0x14, 0x51, 0x78,

    /* U+004B "K" */
    0x8e, 0x69, 0x28, 0xe2, 0x49, 0xa2, 0x84,

    /* U+004C "L" */
    0x84, 0x21, 0x8, 0x42, 0x10, 0xf8,

    /* U+004D "M" */
    0xc3, 0xc3, 0xc3, 0xa5, 0xa5, 0xa5, 0x99, 0x99,
    0x99,

    /* U+004E "N" */
    0x83, 0x87, 0x8d, 0x99, 0x33, 0x63, 0xc3, 0x82,

    /* U+004F "O" */
    0x38, 0x8a, 0xc, 0x18, 0x30, 0x60, 0xa2, 0x38,

    /* U+0050 "P" */
    0xfa, 0x18, 0x61, 0xfa, 0x8, 0x20, 0x80,

    /* U+0051 "Q" */
    0x38, 0x8a, 0xc, 0x18, 0x30, 0x60, 0xa2, 0x3c,
    0xc,

    /* U+0052 "R" */
    0xfa, 0x18, 0x61, 0xfa, 0x68, 0xa3, 0x84,

    /* U+0053 "S" */
    0x3c, 0x8d, 0xb, 0x1, 0x80, 0xd0, 0xa1, 0x3c,

    /* U+0054 "T" */
    0xfe, 0x20, 0x40, 0x81, 0x2, 0x4, 0x8, 0x10,

    /* U+0055 "U" */
    0x86, 0x18, 0x61, 0x86, 0x18, 0x73, 0x78,

    /* U+0056 "V" */
    0xc2, 0x85, 0x1b, 0x22, 0x45, 0x8e, 0xc, 0x18,

    /* U+0057 "W" */
    0xc4, 0x53, 0x14, 0xa5, 0x2b, 0x4a, 0x8c, 0xa3,
    0x18, 0xc6, 0x31, 0x0,

    /* U+0058 "X" */
    0x42, 0xc8, 0xb0, 0xc1, 0x7, 0xb, 0x32, 0x42,

    /* U+0059 "Y" */
    0xc6, 0x89, 0xb1, 0x43, 0x82, 0x4, 0x8, 0x10,

    /* U+005A "Z" */
    0xfc, 0x10, 0x84, 0x20, 0x84, 0x20, 0xfc,

    /* U+005B "[" */
    0xea, 0xaa, 0xaa, 0xc0,

    /* U+005C "\\" */
    0x82, 0x10, 0xc2, 0x10, 0x42, 0x10, 0x40,

    /* U+005D "]" */
    0xd5, 0x55, 0x55, 0xc0,

    /* U+005E "^" */
    0x21, 0x14, 0xa5, 0x0,

    /* U+005F "_" */
    0xf8,

    /* U+0060 "`" */
    0x4c,

    /* U+0061 "a" */
    0x74, 0x42, 0xf8, 0xc5, 0xe0,

    /* U+0062 "b" */
    0x84, 0x21, 0xe9, 0xc6, 0x31, 0x9f, 0x80,

    /* U+0063 "c" */
    0x7b, 0x28, 0x20, 0x83, 0x27, 0x0,

    /* U+0064 "d" */
    0x8, 0x42, 0xfc, 0xc6, 0x31, 0xcb, 0xc0,

    /* U+0065 "e" */
    0x73, 0x28, 0xbf, 0x83, 0x27, 0x80,

    /* U+0066 "f" */
    0x34, 0x4e, 0x44, 0x44, 0x44,

    /* U+0067 "g" */
    0x7e, 0x63, 0x18, 0xe5, 0xe1, 0x8b, 0x80,

    /* U+0068 "h" */
    0x84, 0x21, 0xe8, 0xc6, 0x31, 0x8c, 0x40,

    /* U+0069 "i" */
    0xbf, 0x80,

    /* U+006A "j" */
    0x45, 0x55, 0x57,

    /* U+006B "k" */
    0x84, 0x21, 0x2b, 0x73, 0x94, 0x94, 0x40,

    /* U+006C "l" */
    0xff, 0xc0,

    /* U+006D "m" */
    0xf7, 0x44, 0x62, 0x31, 0x18, 0x8c, 0x46, 0x22,

    /* U+006E "n" */
    0xf4, 0x63, 0x18, 0xc6, 0x20,

    /* U+006F "o" */
    0x7b, 0x38, 0x61, 0x87, 0x37, 0x80,

    /* U+0070 "p" */
    0xf4, 0xe3, 0x18, 0xcf, 0xd0, 0x84, 0x0,

    /* U+0071 "q" */
    0x7e, 0x63, 0x18, 0xe5, 0xe1, 0x8, 0x40,

    /* U+0072 "r" */
    0xf2, 0x49, 0x20,

    /* U+0073 "s" */
    0x74, 0x60, 0xe0, 0xc5, 0xc0,

    /* U+0074 "t" */
    0x4b, 0xa4, 0x92, 0x60,

    /* U+0075 "u" */
    0x8c, 0x63, 0x18, 0xc5, 0xe0,

    /* U+0076 "v" */
    0x89, 0x24, 0x94, 0x30, 0xc2, 0x0,

    /* U+0077 "w" */
    0x88, 0xa4, 0x95, 0x4a, 0xa5, 0x51, 0x10, 0x88,

    /* U+0078 "x" */
    0x49, 0x23, 0xc, 0x31, 0x24, 0x80,

    /* U+0079 "y" */
    0x89, 0x24, 0x94, 0x30, 0xc3, 0x8, 0x23, 0x0,

    /* U+007A "z" */
    0xf8, 0xc4, 0x44, 0x63, 0xe0,

    /* U+007B "{" */
    0x29, 0x24, 0xa2, 0x49, 0x22,

    /* U+007C "|" */
    0xff, 0xe0,

    /* U+007D "}" */
    0x89, 0x24, 0x8b, 0x49, 0x28,

    /* U+007E "~" */
    0xe6, 0x70,

    /* U+00C0 "À" */
    0x20, 0x60, 0x0, 0x83, 0x85, 0xa, 0x36, 0x45,
    0xfa, 0xc, 0x10,

    /* U+00C1 "Á" */
    0x8, 0x30, 0x0, 0x83, 0x85, 0xa, 0x36, 0x45,
    0xfa, 0xc, 0x10,

    /* U+00C2 "Â" */
    0x10, 0x50, 0x0, 0x83, 0x85, 0xa, 0x36, 0x45,
    0xfa, 0xc, 0x10,

    /* U+00C3 "Ã" */
    0x34, 0xb0, 0x0, 0x83, 0x85, 0xa, 0x36, 0x45,
    0xfa, 0xc, 0x10,

    /* U+00C4 "Ä" */
    0x48, 0x0, 0x41, 0xc2, 0x85, 0x1b, 0x22, 0xfd,
    0x6, 0x8,

    /* U+00C5 "Å" */
    0x10, 0x50, 0xa1, 0x81, 0x7, 0xa, 0x14, 0x6c,
    0x89, 0xf4, 0x18, 0x20,

    /* U+00C6 "Æ" */
    0x7, 0xe0, 0xc0, 0x28, 0xd, 0x1, 0x3c, 0x64,
    0xf, 0x82, 0x10, 0x43, 0xe0,

    /* U+00C7 "Ç" */
    0x38, 0x8a, 0x14, 0x8, 0x10, 0x21, 0x22, 0x38,
    0x20, 0x20, 0x80,

    /* U+00C8 "È" */
    0x41, 0x1, 0xf8, 0x42, 0x1f, 0x84, 0x21, 0xf0,

    /* U+00C9 "É" */
    0x11, 0x1, 0xf8, 0x42, 0x1f, 0x84, 0x21, 0xf0,

    /* U+00CA "Ê" */
    0x22, 0x81, 0xf8, 0x42, 0x1f, 0x84, 0x21, 0xf0,

    /* U+00CB "Ë" */
    0x58, 0x3f, 0x8, 0x43, 0xf0, 0x84, 0x3e,

    /* U+00CC "Ì" */
    0x91, 0x55, 0x55,

    /* U+00CD "Í" */
    0x62, 0xaa, 0xaa,

    /* U+00CE "Î" */
    0x54, 0x24, 0x92, 0x49, 0x20,

    /* U+00CF "Ï" */
    0xb0, 0x44, 0x44, 0x44, 0x44, 0x40,

    /* U+00D0 "Ð" */
    0x78, 0x89, 0xa, 0x1f, 0x28, 0x50, 0xa2, 0x78,

    /* U+00D1 "Ñ" */
    0x24, 0xb0, 0x4, 0x1c, 0x3c, 0x6c, 0xc9, 0x9b,
    0x1e, 0x1c, 0x10,

    /* U+00D2 "Ò" */
    0x20, 0x20, 0x1, 0xc4, 0x50, 0x60, 0xc1, 0x83,
    0x5, 0x11, 0xc0,

    /* U+00D3 "Ó" */
    0x8, 0x20, 0x1, 0xc4, 0x50, 0x60, 0xc1, 0x83,
    0x5, 0x11, 0xc0,

    /* U+00D4 "Ô" */
    0x10, 0x50, 0x1, 0xc4, 0x50, 0x60, 0xc1, 0x83,
    0x5, 0x11, 0xc0,

    /* U+00D5 "Õ" */
    0x34, 0xb0, 0x1, 0xc4, 0x50, 0x60, 0xc1, 0x83,
    0x5, 0x11, 0xc0,

    /* U+00D6 "Ö" */
    0x68, 0x0, 0xe2, 0x28, 0x30, 0x60, 0xc1, 0x82,
    0x88, 0xe0,

    /* U+00D7 "×" */
    0x8e, 0x98, 0xe9, 0x0,

    /* U+00D8 "Ø" */
    0x2, 0x79, 0x14, 0x59, 0x32, 0x68, 0xd1, 0x44,
    0xf2, 0x0,

    /* U+00D9 "Ù" */
    0x20, 0x80, 0x21, 0x86, 0x18, 0x61, 0x86, 0x1c,
    0xde,

    /* U+00DA "Ú" */
    0x10, 0x40, 0x21, 0x86, 0x18, 0x61, 0x86, 0x1c,
    0xde,

    /* U+00DB "Û" */
    0x31, 0x40, 0x21, 0x86, 0x18, 0x61, 0x86, 0x1c,
    0xde,

    /* U+00DC "Ü" */
    0x48, 0x8, 0x61, 0x86, 0x18, 0x61, 0x87, 0x37,
    0x80,

    /* U+00DD "Ý" */
    0x8, 0x20, 0x6, 0x34, 0x4d, 0x8a, 0x1c, 0x10,
    0x20, 0x40, 0x80,

    /* U+00DE "Þ" */
    0x82, 0xf, 0xa1, 0x86, 0x1f, 0xa0, 0x80,

    /* U+00DF "ß" */
    0x62, 0x49, 0x24, 0x92, 0x48, 0xa1, 0x86, 0xe0,

    /* U+00E0 "à" */
    0x61, 0x0, 0xe8, 0x85, 0xf1, 0x8b, 0xc0,

    /* U+00E1 "á" */
    0x11, 0x0, 0xe8, 0x85, 0xf1, 0x8b, 0xc0,

    /* U+00E2 "â" */
    0x32, 0x80, 0xe8, 0x85, 0xf1, 0x8b, 0xc0,

    /* U+00E3 "ã" */
    0x6c, 0x80, 0xe8, 0x85, 0xf1, 0x8b, 0xc0,

    /* U+00E4 "ä" */
    0x50, 0x1d, 0x10, 0xbe, 0x31, 0x78,

    /* U+00E5 "å" */
    0x72, 0x9c, 0x7, 0x44, 0x2f, 0x8c, 0x5e,

    /* U+00E6 "æ" */
    0x77, 0x66, 0x42, 0x2f, 0xf8, 0x84, 0x65, 0xdc,

    /* U+00E7 "ç" */
    0x7b, 0x28, 0x20, 0x83, 0x27, 0xc, 0x10, 0xc0,

    /* U+00E8 "è" */
    0x60, 0x80, 0x1c, 0xca, 0x2f, 0xe0, 0xc9, 0xe0,

    /* U+00E9 "é" */
    0x10, 0x80, 0x1c, 0xca, 0x2f, 0xe0, 0xc9, 0xe0,

    /* U+00EA "ê" */
    0x31, 0x40, 0x1c, 0xca, 0x2f, 0xe0, 0xc9, 0xe0,

    /* U+00EB "ë" */
    0xd0, 0x7, 0x32, 0x8b, 0xf8, 0x32, 0x78,

    /* U+00EC "ì" */
    0x91, 0x55, 0x50,

    /* U+00ED "í" */
    0x62, 0xaa, 0xa0,

    /* U+00EE "î" */
    0x54, 0x24, 0x92, 0x48,

    /* U+00EF "ï" */
    0xa1, 0x24, 0x92, 0x40,

    /* U+00F0 "ð" */
    0x41, 0x8c, 0x37, 0xc6, 0x31, 0xdb, 0x80,

    /* U+00F1 "ñ" */
    0x6d, 0x81, 0xe8, 0xc6, 0x31, 0x8c, 0x40,

    /* U+00F2 "ò" */
    0x60, 0x80, 0x1e, 0xce, 0x18, 0x61, 0xcd, 0xe0,

    /* U+00F3 "ó" */
    0x18, 0x40, 0x1e, 0xce, 0x18, 0x61, 0xcd, 0xe0,

    /* U+00F4 "ô" */
    0x31, 0x20, 0x1e, 0xce, 0x18, 0x61, 0xcd, 0xe0,

    /* U+00F5 "õ" */
    0x69, 0x60, 0x1e, 0xce, 0x18, 0x61, 0xcd, 0xe0,

    /* U+00F6 "ö" */
    0xd0, 0x7, 0xb3, 0x86, 0x18, 0x73, 0x78,

    /* U+00F7 "÷" */
    0x20, 0xf, 0xc0, 0x0, 0x80,

    /* U+00F8 "ø" */
    0x9, 0xec, 0xe5, 0xa6, 0x9c, 0xde, 0x0,

    /* U+00F9 "ù" */
    0x41, 0x1, 0x18, 0xc6, 0x31, 0x8b, 0xc0,

    /* U+00FA "ú" */
    0x11, 0x1, 0x18, 0xc6, 0x31, 0x8b, 0xc0,

    /* U+00FB "û" */
    0x22, 0x81, 0x18, 0xc6, 0x31, 0x8b, 0xc0,

    /* U+00FC "ü" */
    0xd0, 0x23, 0x18, 0xc6, 0x31, 0x78,

    /* U+00FD "ý" */
    0x10, 0x40, 0x22, 0x49, 0x25, 0xc, 0x30, 0xc2,
    0x8, 0xc0,

    /* U+00FE "þ" */
    0x84, 0x21, 0xe9, 0xc6, 0x31, 0x9f, 0xa1, 0x8,
    0x0,

    /* U+00FF "ÿ" */
    0x48, 0x8, 0x92, 0x49, 0x43, 0xc, 0x30, 0x82,
    0x30,

    /* U+0100 "Ā" */
    0x7c, 0x0, 0x41, 0xc2, 0x85, 0x1b, 0x22, 0xfd,
    0x6, 0x8,

    /* U+0101 "ā" */
    0x78, 0x1d, 0x10, 0xbe, 0x31, 0x78,

    /* U+0102 "Ă" */
    0x0, 0x70, 0x0, 0x83, 0x85, 0xa, 0x36, 0x45,
    0xfa, 0xc, 0x10,

    /* U+0103 "ă" */
    0x4b, 0x80, 0xe0, 0x85, 0xf1, 0x8b, 0xc0,

    /* U+0104 "Ą" */
    0x10, 0x70, 0xa1, 0x46, 0xc8, 0xbf, 0x41, 0x82,
    0x4, 0x10, 0x30,

    /* U+0105 "ą" */
    0x74, 0x42, 0xf8, 0xc5, 0xe2, 0x10, 0xc0,

    /* U+0106 "Ć" */
    0x18, 0x40, 0xe, 0x46, 0x18, 0x20, 0x82, 0x14,
    0x4e,

    /* U+0107 "ć" */
    0x10, 0x80, 0x1e, 0xca, 0x8, 0x20, 0xc9, 0xc0,

    /* U+0108 "Ĉ" */
    0x31, 0x20, 0xe, 0x46, 0x18, 0x20, 0x82, 0x14,
    0x4e,

    /* U+0109 "ĉ" */
    0x21, 0x40, 0x1e, 0xca, 0x8, 0x20, 0xc9, 0xc0,

    /* U+010A "Ċ" */
    0x30, 0x0, 0xe, 0x46, 0x18, 0x20, 0x82, 0x14,
    0x4e,

    /* U+010B "ċ" */
    0x60, 0x7, 0xb2, 0x82, 0x8, 0x32, 0x70,

    /* U+010C "Č" */
    0x28, 0x40, 0xe, 0x46, 0x18, 0x20, 0x82, 0x14,
    0x4e,

    /* U+010D "č" */
    0x50, 0x80, 0x1e, 0xca, 0x8, 0x20, 0xc9, 0xc0,

    /* U+010E "Ď" */
    0x50, 0x80, 0x3c, 0x8a, 0x18, 0x61, 0x86, 0x18,
    0xbc,

    /* U+010F "ď" */
    0xa, 0x14, 0x23, 0xcc, 0x91, 0x22, 0x44, 0xc8,
    0xf0,

    /* U+0110 "Đ" */
    0x78, 0x89, 0xa, 0x1f, 0x28, 0x50, 0xa2, 0x78,

    /* U+0111 "đ" */
    0x8, 0x70, 0x9e, 0xca, 0x28, 0xa2, 0xc9, 0xe0,

    /* U+0112 "Ē" */
    0x78, 0x3f, 0x8, 0x43, 0xf0, 0x84, 0x3e,

    /* U+0113 "ē" */
    0x78, 0x7, 0x32, 0x8b, 0xf8, 0x32, 0x78,

    /* U+0114 "Ĕ" */
    0x53, 0x81, 0xf8, 0x42, 0x1f, 0x84, 0x21, 0xf0,

    /* U+0115 "ĕ" */
    0x49, 0xc0, 0x1c, 0xca, 0x2f, 0xe0, 0xc9, 0xe0,

    /* U+0116 "Ė" */
    0x20, 0x3f, 0x8, 0x43, 0xf0, 0x84, 0x3e,

    /* U+0117 "ė" */
    0x20, 0x7, 0x32, 0x8b, 0xf8, 0x32, 0x78,

    /* U+0118 "Ę" */
    0xfc, 0x21, 0xf, 0xc2, 0x10, 0xf8, 0x84, 0x30,

    /* U+0119 "ę" */
    0x73, 0x28, 0xbf, 0x83, 0x27, 0x84, 0x20, 0x40,

    /* U+011A "Ě" */
    0x51, 0x1, 0xf8, 0x42, 0x1f, 0x84, 0x21, 0xf0,

    /* U+011B "ě" */
    0x50, 0x80, 0x1c, 0xca, 0x2f, 0xe0, 0xc9, 0xe0,

    /* U+011C "Ĝ" */
    0x30, 0x90, 0x1, 0xe4, 0x70, 0x20, 0x47, 0x83,
    0x5, 0x9, 0xe0,

    /* U+011D "ĝ" */
    0x22, 0x80, 0xfc, 0xc6, 0x31, 0xcb, 0xc3, 0x17,
    0x0,

    /* U+011E "Ğ" */
    0x68, 0x70, 0x1, 0xe4, 0x70, 0x20, 0x47, 0x83,
    0x5, 0x9, 0xe0,

    /* U+011F "ğ" */
    0x52, 0x8c, 0xfc, 0xc6, 0x31, 0xcb, 0xc3, 0x17,
    0x0,

    /* U+0120 "Ġ" */
    0x30, 0x0, 0x1, 0xe4, 0x70, 0x20, 0x47, 0x83,
    0x5, 0x9, 0xe0,

    /* U+0121 "ġ" */
    0x20, 0x1f, 0x98, 0xc6, 0x39, 0x78, 0x62, 0xe0,

    /* U+0122 "Ģ" */
    0x3c, 0x8e, 0x4, 0x9, 0xf0, 0x60, 0xa1, 0x3c,
    0x20, 0x40, 0x80,

    /* U+0123 "ģ" */
    0x21, 0x8, 0xfc, 0xc6, 0x31, 0xcb, 0xc3, 0x17,
    0x0,

    /* U+0124 "Ĥ" */
    0x10, 0x50, 0x4, 0x18, 0x30, 0x60, 0xff, 0x83,
    0x6, 0xc, 0x10,

    /* U+0125 "ĥ" */
    0x62, 0xa1, 0x8, 0x7a, 0x31, 0x8c, 0x63, 0x10,

    /* U+0126 "Ħ" */
    0x42, 0x42, 0xff, 0x42, 0x7e, 0x42, 0x42, 0x42,
    0x42,

    /* U+0127 "ħ" */
    0x43, 0xc4, 0x16, 0x65, 0x14, 0x51, 0x45, 0x10,

    /* U+0128 "Ĩ" */
    0x55, 0x80, 0x42, 0x10, 0x84, 0x21, 0x8, 0x40,

    /* U+0129 "ĩ" */
    0x6d, 0x80, 0x42, 0x10, 0x84, 0x21, 0x0,

    /* U+012A "Ī" */
    0xe1, 0x24, 0x92, 0x49, 0x0,

    /* U+012B "ī" */
    0xe1, 0x24, 0x92, 0x40,

    /* U+012C "Ĭ" */
    0xbc, 0x24, 0x92, 0x49, 0x20,

    /* U+012D "ĭ" */
    0xbc, 0x24, 0x92, 0x48,

    /* U+012E "Į" */
    0x55, 0x55, 0x6b,

    /* U+012F "į" */
    0x45, 0x55, 0x6b,

    /* U+0130 "İ" */
    0xbf, 0xe0,

    /* U+0131 "ı" */
    0xfe,

    /* U+0132 "Ĳ" */
    0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0xb1, 0x91,
    0x9e,

    /* U+0133 "ĳ" */
    0x90, 0x99, 0x99, 0x99, 0x91, 0x13,

    /* U+0134 "Ĵ" */
    0x4, 0x14, 0x0, 0x20, 0x40, 0x81, 0x2, 0x4,
    0x89, 0x13, 0xc0,

    /* U+0135 "ĵ" */
    0x54, 0x24, 0x92, 0x49, 0x28,

    /* U+0136 "Ķ" */
    0x8a, 0x69, 0x28, 0xe2, 0xc9, 0x22, 0x8c, 0x82,
    0x0,

    /* U+0137 "ķ" */
    0x84, 0x21, 0x3b, 0x53, 0x96, 0x94, 0x48, 0x46,
    0x0,

    /* U+0138 "ĸ" */
    0x9c, 0xa9, 0xca, 0x4a, 0x20,

    /* U+0139 "Ĺ" */
    0x44, 0x1, 0x8, 0x42, 0x10, 0x84, 0x21, 0xf0,

    /* U+013A "ĺ" */
    0x7a, 0xaa, 0xaa, 0x80,

    /* U+013B "Ļ" */
    0x84, 0x21, 0x8, 0x42, 0x10, 0xf9, 0x8, 0x40,

    /* U+013C "ļ" */
    0xff, 0xf8,

    /* U+013D "Ľ" */
    0x94, 0xa1, 0x8, 0x42, 0x10, 0xf8,

    /* U+013E "ľ" */
    0xb6, 0x49, 0x24, 0x90,

    /* U+013F "Ŀ" */
    0x84, 0x21, 0x68, 0x42, 0x10, 0xf8,

    /* U+0140 "ŀ" */
    0x92, 0x49, 0x64, 0x90,

    /* U+0141 "Ł" */
    0x41, 0x4, 0x14, 0xe1, 0x4, 0x10, 0x7c,

    /* U+0142 "ł" */
    0x49, 0x27, 0x92, 0x48,

    /* U+0143 "Ń" */
    0xc, 0x20, 0x4, 0x1c, 0x3c, 0x6c, 0xc9, 0x9b,
    0x1e, 0x1c, 0x10,

    /* U+0144 "ń" */
    0x11, 0x1, 0xe8, 0xc6, 0x31, 0x8c, 0x40,

    /* U+0145 "Ņ" */
    0x83, 0x87, 0x8d, 0x99, 0x33, 0x63, 0xc3, 0x82,
    0x20, 0x41, 0x80,

    /* U+0146 "ņ" */
    0xf4, 0x63, 0x18, 0xc6, 0x24, 0x21, 0x0,

    /* U+0147 "Ň" */
    0x4c, 0x60, 0x4, 0x1c, 0x3c, 0x6c, 0xc9, 0x9b,
    0x1e, 0x1c, 0x10,

    /* U+0148 "ň" */
    0x51, 0x1, 0xe8, 0xc6, 0x31, 0x8c, 0x40,

    /* U+0149 "ŉ" */
    0x82, 0x0, 0x1e, 0x45, 0x14, 0x51, 0x45, 0x10,

    /* U+014A "Ŋ" */
    0x87, 0x1c, 0x69, 0xb6, 0x58, 0xe3, 0x84, 0x10,
    0x42,

    /* U+014B "ŋ" */
    0xf4, 0x63, 0x18, 0xc6, 0x21, 0x9, 0x80,

    /* U+014C "Ō" */
    0x7c, 0x0, 0xe2, 0x28, 0x30, 0x60, 0xc1, 0x82,
    0x88, 0xe0,

    /* U+014D "ō" */
    0x78, 0x7, 0xb3, 0x86, 0x18, 0x73, 0x78,

    /* U+014E "Ŏ" */
    0x28, 0x70, 0x1, 0xc4, 0x50, 0x60, 0xc1, 0x83,
    0x5, 0x11, 0xc0,

    /* U+014F "ŏ" */
    0x48, 0xc0, 0x1e, 0xce, 0x18, 0x61, 0xcd, 0xe0,

    /* U+0150 "Ő" */
    0x14, 0x50, 0x1, 0xc4, 0x50, 0x60, 0xc1, 0x83,
    0x5, 0x11, 0xc0,

    /* U+0151 "ő" */
    0x28, 0xa0, 0x1e, 0xce, 0x18, 0x61, 0xcd, 0xe0,

    /* U+0152 "Œ" */
    0x3f, 0xd1, 0x8, 0x42, 0x10, 0x87, 0xe1, 0x8,
    0x41, 0x10, 0x3f, 0xc0,

    /* U+0153 "œ" */
    0x7b, 0xb3, 0x98, 0x46, 0x1f, 0x84, 0x33, 0x87,
    0xb8,

    /* U+0154 "Ŕ" */
    0x18, 0xc0, 0x3e, 0x86, 0x18, 0x7e, 0x9a, 0x28,
    0xe1,

    /* U+0155 "ŕ" */
    0x28, 0x79, 0x24, 0x90,

    /* U+0156 "Ŗ" */
    0xfa, 0x18, 0x61, 0xfa, 0x68, 0xa3, 0x84, 0x82,
    0x8,

    /* U+0157 "ŗ" */
    0xf2, 0x49, 0x24, 0x90,

    /* U+0158 "Ř" */
    0x48, 0xc0, 0x3e, 0x86, 0x18, 0x7e, 0x9a, 0x28,
    0xe1,

    /* U+0159 "ř" */
    0xa8, 0x79, 0x24, 0x90,

    /* U+015A "Ś" */
    0xc, 0x10, 0x1, 0xe4, 0x68, 0x58, 0xc, 0x6,
    0x85, 0x9, 0xe0,

    /* U+015B "ś" */
    0x11, 0x0, 0xe8, 0xc1, 0xc1, 0x8b, 0x80,

    /* U+015C "Ŝ" */
    0x18, 0x50, 0x1, 0xe4, 0x68, 0x58, 0xc, 0x6,
    0x85, 0x9, 0xe0,

    /* U+015D "ŝ" */
    0x22, 0x80, 0xe8, 0xc1, 0xc1, 0x8b, 0x80,

    /* U+015E "Ş" */
    0x3c, 0x8d, 0xb, 0x1, 0x80, 0xd0, 0xa1, 0x3c,
    0x30, 0x20, 0xc0,

    /* U+015F "ş" */
    0x74, 0x70, 0xe0, 0xc5, 0xc4, 0x11, 0x80,

    /* U+0160 "Š" */
    0x2c, 0x30, 0x1, 0xe4, 0x68, 0x58, 0xc, 0x6,
    0x85, 0x9, 0xe0,

    /* U+0161 "š" */
    0x51, 0x0, 0xe8, 0xc1, 0xc1, 0x8b, 0x80,

    /* U+0162 "Ţ" */
    0xfe, 0x20, 0x40, 0x81, 0x2, 0x4, 0x8, 0x10,
    0x20, 0x20, 0xc0,

    /* U+0163 "ţ" */
    0x44, 0xe4, 0x44, 0x46, 0x62, 0x12,

    /* U+0164 "Ť" */
    0x28, 0x20, 0x7, 0xf1, 0x2, 0x4, 0x8, 0x10,
    0x20, 0x40, 0x80,

    /* U+0165 "ť" */
    0x11, 0x54, 0xe4, 0x44, 0x44, 0x60,

    /* U+0166 "Ŧ" */
    0xfe, 0x20, 0x41, 0xc1, 0x2, 0x4, 0x8, 0x10,

    /* U+0167 "ŧ" */
    0x4b, 0xae, 0x92, 0x60,

    /* U+0168 "Ũ" */
    0x69, 0x60, 0x21, 0x86, 0x18, 0x61, 0x86, 0x1c,
    0xde,

    /* U+0169 "ũ" */
    0x6d, 0x81, 0x18, 0xc6, 0x31, 0x8b, 0xc0,

    /* U+016A "Ū" */
    0x78, 0x8, 0x61, 0x86, 0x18, 0x61, 0x87, 0x37,
    0x80,

    /* U+016B "ū" */
    0x78, 0x23, 0x18, 0xc6, 0x31, 0x78,

    /* U+016C "Ŭ" */
    0x48, 0xc0, 0x21, 0x86, 0x18, 0x61, 0x86, 0x1c,
    0xde,

    /* U+016D "ŭ" */
    0x53, 0x81, 0x18, 0xc6, 0x31, 0x8b, 0xc0,

    /* U+016E "Ů" */
    0x38, 0xa3, 0x80, 0x86, 0x18, 0x61, 0x86, 0x18,
    0x73, 0x78,

    /* U+016F "ů" */
    0x72, 0x9c, 0x8, 0xc6, 0x31, 0x8c, 0x5e,

    /* U+0170 "Ű" */
    0x28, 0xa0, 0x21, 0x86, 0x18, 0x61, 0x86, 0x1c,
    0xde,

    /* U+0171 "ű" */
    0x2a, 0x81, 0x18, 0xc6, 0x31, 0x8b, 0xc0,

    /* U+0172 "Ų" */
    0x86, 0x18, 0x61, 0x86, 0x18, 0x73, 0x78, 0x41,
    0x6,

    /* U+0173 "ų" */
    0x8c, 0x63, 0x18, 0xc5, 0xe2, 0x10, 0xc0,

    /* U+0174 "Ŵ" */
    0x4, 0x2, 0x80, 0x3, 0x11, 0x4c, 0x52, 0x94,
    0xad, 0x2a, 0x32, 0x8c, 0x63, 0x18, 0xc4,

    /* U+0175 "ŵ" */
    0x8, 0xa, 0x0, 0x11, 0x14, 0x92, 0xa9, 0x54,
    0xaa, 0x22, 0x11, 0x0,

    /* U+0176 "Ŷ" */
    0x10, 0x50, 0x6, 0x34, 0x4d, 0x8a, 0x1c, 0x10,
    0x20, 0x40, 0x80,

    /* U+0177 "ŷ" */
    0x31, 0x0, 0x22, 0x49, 0x25, 0xc, 0x30, 0xc2,
    0x8, 0xc0,

    /* U+0178 "Ÿ" */
    0x2c, 0x3, 0x1a, 0x26, 0xc5, 0xe, 0x8, 0x10,
    0x20, 0x40,

    /* U+0179 "Ź" */
    0x8, 0x40, 0x3f, 0x4, 0x21, 0x8, 0x21, 0x8,
    0x3f,

    /* U+017A "ź" */
    0x11, 0x1, 0xf1, 0x88, 0x88, 0xc7, 0xc0,

    /* U+017B "Ż" */
    0x20, 0xf, 0xc1, 0x8, 0x42, 0x8, 0x42, 0xf,
    0xc0,

    /* U+017C "ż" */
    0x60, 0x3e, 0x31, 0x11, 0x18, 0xf8,

    /* U+017D "Ž" */
    0x48, 0xc0, 0x3f, 0x4, 0x21, 0x8, 0x21, 0x8,
    0x3f,

    /* U+017E "ž" */
    0x51, 0x1, 0xf1, 0x88, 0x88, 0xc7, 0xc0,

    /* U+017F "ſ" */
    0x72, 0x49, 0x24, 0x90
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 48, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 50, .box_w = 1, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 3, .adv_w = 62, .box_w = 3, .box_h = 3, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 5, .adv_w = 118, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 12, .adv_w = 108, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 21, .adv_w = 141, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 30, .adv_w = 119, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 38, .adv_w = 34, .box_w = 1, .box_h = 3, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 39, .adv_w = 66, .box_w = 3, .box_h = 13, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 44, .adv_w = 67, .box_w = 3, .box_h = 13, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 49, .adv_w = 83, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 53, .adv_w = 109, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 58, .adv_w = 38, .box_w = 2, .box_h = 3, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 59, .adv_w = 53, .box_w = 3, .box_h = 1, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 60, .adv_w = 51, .box_w = 1, .box_h = 1, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 61, .adv_w = 79, .box_w = 5, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 68, .adv_w = 108, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 74, .adv_w = 108, .box_w = 3, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 78, .adv_w = 108, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 84, .adv_w = 108, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 90, .adv_w = 108, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 97, .adv_w = 108, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 103, .adv_w = 108, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 109, .adv_w = 108, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 116, .adv_w = 108, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 122, .adv_w = 108, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 128, .adv_w = 47, .box_w = 1, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 129, .adv_w = 41, .box_w = 1, .box_h = 8, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 130, .adv_w = 98, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 134, .adv_w = 105, .box_w = 5, .box_h = 4, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 137, .adv_w = 100, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 141, .adv_w = 91, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 147, .adv_w = 172, .box_w = 10, .box_h = 12, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 162, .adv_w = 125, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 170, .adv_w = 120, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 177, .adv_w = 125, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 184, .adv_w = 126, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 191, .adv_w = 109, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 197, .adv_w = 106, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 203, .adv_w = 131, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 211, .adv_w = 137, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 219, .adv_w = 52, .box_w = 1, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 221, .adv_w = 106, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 228, .adv_w = 120, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 235, .adv_w = 103, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 241, .adv_w = 168, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 250, .adv_w = 137, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 258, .adv_w = 132, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 266, .adv_w = 121, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 273, .adv_w = 132, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 282, .adv_w = 118, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 289, .adv_w = 114, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 297, .adv_w = 115, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 305, .adv_w = 125, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 312, .adv_w = 122, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 320, .adv_w = 170, .box_w = 10, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 332, .adv_w = 120, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 340, .adv_w = 115, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 348, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 355, .adv_w = 51, .box_w = 2, .box_h = 13, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 359, .adv_w = 79, .box_w = 5, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 366, .adv_w = 51, .box_w = 2, .box_h = 13, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 370, .adv_w = 80, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 374, .adv_w = 87, .box_w = 5, .box_h = 1, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 375, .adv_w = 59, .box_w = 3, .box_h = 2, .ofs_x = 0, .ofs_y = 8},
    {.bitmap_index = 376, .adv_w = 104, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 381, .adv_w = 108, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 388, .adv_w = 101, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 394, .adv_w = 108, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 401, .adv_w = 102, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 407, .adv_w = 67, .box_w = 4, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 412, .adv_w = 108, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 419, .adv_w = 106, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 426, .adv_w = 47, .box_w = 1, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 428, .adv_w = 46, .box_w = 2, .box_h = 12, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 431, .adv_w = 97, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 438, .adv_w = 47, .box_w = 1, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 440, .adv_w = 168, .box_w = 9, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 448, .adv_w = 106, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 453, .adv_w = 110, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 459, .adv_w = 108, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 466, .adv_w = 109, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 473, .adv_w = 65, .box_w = 3, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 476, .adv_w = 99, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 481, .adv_w = 63, .box_w = 3, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 485, .adv_w = 106, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 490, .adv_w = 93, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 496, .adv_w = 144, .box_w = 9, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 504, .adv_w = 95, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 510, .adv_w = 91, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 518, .adv_w = 95, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 523, .adv_w = 65, .box_w = 3, .box_h = 13, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 528, .adv_w = 47, .box_w = 1, .box_h = 11, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 530, .adv_w = 65, .box_w = 3, .box_h = 13, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 535, .adv_w = 131, .box_w = 6, .box_h = 2, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 537, .adv_w = 125, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 548, .adv_w = 125, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 559, .adv_w = 125, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 570, .adv_w = 125, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 581, .adv_w = 125, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 591, .adv_w = 125, .box_w = 7, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 603, .adv_w = 179, .box_w = 11, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 616, .adv_w = 125, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 627, .adv_w = 109, .box_w = 5, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 635, .adv_w = 109, .box_w = 5, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 643, .adv_w = 109, .box_w = 5, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 651, .adv_w = 109, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 658, .adv_w = 52, .box_w = 2, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 661, .adv_w = 52, .box_w = 2, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 664, .adv_w = 52, .box_w = 3, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 669, .adv_w = 52, .box_w = 4, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 675, .adv_w = 129, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 683, .adv_w = 137, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 694, .adv_w = 132, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 705, .adv_w = 132, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 716, .adv_w = 132, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 727, .adv_w = 132, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 738, .adv_w = 132, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 748, .adv_w = 102, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 752, .adv_w = 132, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 762, .adv_w = 125, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 771, .adv_w = 125, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 780, .adv_w = 125, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 789, .adv_w = 125, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 798, .adv_w = 115, .box_w = 7, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 809, .adv_w = 113, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 816, .adv_w = 114, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 824, .adv_w = 104, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 831, .adv_w = 104, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 838, .adv_w = 104, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 845, .adv_w = 104, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 852, .adv_w = 104, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 858, .adv_w = 104, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 865, .adv_w = 162, .box_w = 9, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 873, .adv_w = 101, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 881, .adv_w = 102, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 889, .adv_w = 102, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 897, .adv_w = 102, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 905, .adv_w = 102, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 912, .adv_w = 48, .box_w = 2, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 915, .adv_w = 48, .box_w = 2, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 918, .adv_w = 48, .box_w = 3, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 922, .adv_w = 48, .box_w = 3, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 926, .adv_w = 113, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 933, .adv_w = 106, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 940, .adv_w = 110, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 948, .adv_w = 110, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 956, .adv_w = 110, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 964, .adv_w = 110, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 972, .adv_w = 110, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 979, .adv_w = 110, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 984, .adv_w = 109, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 991, .adv_w = 106, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 998, .adv_w = 106, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1005, .adv_w = 106, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1012, .adv_w = 106, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1018, .adv_w = 91, .box_w = 6, .box_h = 13, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 1028, .adv_w = 111, .box_w = 5, .box_h = 13, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 1037, .adv_w = 91, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 1046, .adv_w = 125, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1056, .adv_w = 104, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1062, .adv_w = 125, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1073, .adv_w = 104, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1080, .adv_w = 125, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 1091, .adv_w = 104, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 1098, .adv_w = 125, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1107, .adv_w = 101, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1115, .adv_w = 125, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1124, .adv_w = 101, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1132, .adv_w = 125, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1141, .adv_w = 101, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1148, .adv_w = 125, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1157, .adv_w = 101, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1165, .adv_w = 126, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1174, .adv_w = 122, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1183, .adv_w = 129, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1191, .adv_w = 115, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1199, .adv_w = 109, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1206, .adv_w = 102, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1213, .adv_w = 109, .box_w = 5, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1221, .adv_w = 102, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1229, .adv_w = 109, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1236, .adv_w = 102, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1243, .adv_w = 109, .box_w = 5, .box_h = 12, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 1251, .adv_w = 102, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 1259, .adv_w = 109, .box_w = 5, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1267, .adv_w = 102, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1275, .adv_w = 131, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1286, .adv_w = 108, .box_w = 5, .box_h = 13, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 1295, .adv_w = 131, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1306, .adv_w = 108, .box_w = 5, .box_h = 13, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 1315, .adv_w = 131, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1326, .adv_w = 108, .box_w = 5, .box_h = 12, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 1334, .adv_w = 131, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 1345, .adv_w = 108, .box_w = 5, .box_h = 13, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 1354, .adv_w = 137, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1365, .adv_w = 106, .box_w = 5, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1373, .adv_w = 135, .box_w = 8, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1382, .adv_w = 109, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1390, .adv_w = 52, .box_w = 5, .box_h = 12, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 1398, .adv_w = 48, .box_w = 5, .box_h = 10, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 1405, .adv_w = 52, .box_w = 3, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1410, .adv_w = 48, .box_w = 3, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1414, .adv_w = 52, .box_w = 3, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1419, .adv_w = 48, .box_w = 3, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1423, .adv_w = 52, .box_w = 2, .box_h = 12, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 1426, .adv_w = 47, .box_w = 2, .box_h = 12, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 1429, .adv_w = 52, .box_w = 1, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1431, .adv_w = 48, .box_w = 1, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1432, .adv_w = 158, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1441, .adv_w = 93, .box_w = 4, .box_h = 12, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 1447, .adv_w = 106, .box_w = 7, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1458, .adv_w = 48, .box_w = 3, .box_h = 13, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 1463, .adv_w = 120, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 1472, .adv_w = 97, .box_w = 5, .box_h = 13, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 1481, .adv_w = 107, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1486, .adv_w = 103, .box_w = 5, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1494, .adv_w = 47, .box_w = 2, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1498, .adv_w = 103, .box_w = 5, .box_h = 12, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 1506, .adv_w = 47, .box_w = 1, .box_h = 13, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 1508, .adv_w = 103, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1514, .adv_w = 61, .box_w = 3, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1518, .adv_w = 103, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1524, .adv_w = 67, .box_w = 3, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1528, .adv_w = 103, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1535, .adv_w = 52, .box_w = 3, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1539, .adv_w = 137, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1550, .adv_w = 106, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1557, .adv_w = 137, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 1568, .adv_w = 106, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 1575, .adv_w = 137, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1586, .adv_w = 106, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1593, .adv_w = 106, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1601, .adv_w = 133, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 1610, .adv_w = 109, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 1617, .adv_w = 132, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1627, .adv_w = 110, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1634, .adv_w = 132, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1645, .adv_w = 110, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1653, .adv_w = 132, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1664, .adv_w = 110, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1672, .adv_w = 183, .box_w = 10, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1684, .adv_w = 174, .box_w = 10, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1693, .adv_w = 118, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1702, .adv_w = 65, .box_w = 3, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1706, .adv_w = 118, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 1715, .adv_w = 65, .box_w = 3, .box_h = 10, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 1719, .adv_w = 118, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1728, .adv_w = 65, .box_w = 3, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1732, .adv_w = 114, .box_w = 7, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1743, .adv_w = 99, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1750, .adv_w = 114, .box_w = 7, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1761, .adv_w = 99, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1768, .adv_w = 114, .box_w = 7, .box_h = 12, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 1779, .adv_w = 99, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 1786, .adv_w = 114, .box_w = 7, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1797, .adv_w = 99, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1804, .adv_w = 115, .box_w = 7, .box_h = 12, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 1815, .adv_w = 63, .box_w = 4, .box_h = 12, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 1821, .adv_w = 115, .box_w = 7, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1832, .adv_w = 67, .box_w = 4, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1838, .adv_w = 115, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1846, .adv_w = 63, .box_w = 3, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1850, .adv_w = 125, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1859, .adv_w = 106, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1866, .adv_w = 125, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1875, .adv_w = 106, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1881, .adv_w = 125, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1890, .adv_w = 106, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1897, .adv_w = 125, .box_w = 6, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1907, .adv_w = 106, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1914, .adv_w = 125, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1923, .adv_w = 106, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1930, .adv_w = 125, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 1939, .adv_w = 106, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 1946, .adv_w = 170, .box_w = 10, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1961, .adv_w = 144, .box_w = 9, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1973, .adv_w = 115, .box_w = 7, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1984, .adv_w = 91, .box_w = 6, .box_h = 13, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 1994, .adv_w = 115, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2004, .adv_w = 115, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 2013, .adv_w = 95, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 2020, .adv_w = 115, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 2029, .adv_w = 95, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 2035, .adv_w = 115, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 2044, .adv_w = 95, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 2051, .adv_w = 48, .box_w = 3, .box_h = 10, .ofs_x = 1, .ofs_y = 0}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/



/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 95, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 192, .range_length = 192, .glyph_id_start = 96,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    }
};

/*-----------------
 *    KERNING
 *----------------*/


/*Pair left and right glyphs for kerning*/
static const uint16_t kern_pair_glyph_ids[] =
{
    3, 3,
    3, 8,
    8, 3,
    8, 8,
    13, 3,
    13, 8,
    15, 3,
    15, 8,
    16, 16
};

/* Kerning between the respective left and right glyphs
 * 4.4 format which needs to scaled with `kern_scale`*/
static const int8_t kern_pair_values[] =
{
    -10, -10, -10, -10, -16, -16, -16, -16,
    -21
};

/*Collect the kern pair's data in one place*/
static const lv_font_fmt_txt_kern_pair_t kern_pairs =
{
    .glyph_ids = kern_pair_glyph_ids,
    .values = kern_pair_values,
    .pair_cnt = 9,
    .glyph_ids_size = 1
};

/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = &kern_pairs,
    .kern_scale = 16,
    .cmap_num = 2,
    .bpp = 1,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif
};



/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t roboto_12pt = {
#else
lv_font_t roboto_12pt = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 16,          /*The maximum line height required by the font*/
    .base_line = 3,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -1,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if ROBOTO_12PT*/

