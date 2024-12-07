#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "types.h"

// Keyboard I/O ports
#define KB_COMMAND 0x64
#define KB_DATA 0x60
#define KB_OUTPUT 0x60
#define KB_STATUS 0x64

void keyboard_init();
void keyboard_handler();
 
char get_scancode_char(uint8_t scancode, bool shift);

// Mappings for scancodes

static unsigned char sc2ch[128] =
{
    0, 
    27,  
    '1',  
    '2',  
    '3',  
    '4',  
    '5',  
    '6',  
    '7',  
    '8', 
    '9', 
    '0', 
    '+', 
    0, 
    '\b',	/* Backspace */ 
    '\t',	/* Tab */ 
    'q', 
    'w',  
    'e',  
    'r', 
    't', 
    'y',  
    'u',  
    'i',  
    'o',  
    'p',  
    '[',  
    ']',  
    '\n',	/* Enter key */ 
    0,		/* 29   - Control */ 
    'a', 
    's', 
    'd', 
    'f', 
    'g', 
    'h', 
    'j', 
    'k', 
    'l', 
    ';', 
    '\'', 
    '<',    
    0,		/* Left shift */ 
    '\\', 
    'z', 
    'x', 
    'c', 
    'v',  
    'b',  
    'n', 
    'm', 
    ',', 
    '.', 
    '-', 
    0,				/* Right shift */ 
    '*', 
    0,	/* Alt */ 
    ' ',	/* Space bar */ 
    0,	/* Caps lock */ 
    0,	/* 59 - F1 key ... > */ 
    0, 
    0, 
    0,    
    0,    
    0,    
    0,    
    0,    
    0, 
    0,	/* < ... F10 */ 
    0,	/* 69 - Num lock*/ 
    0,	/* Scroll Lock */ 
    0,	/* Home key */ 
    0,	/* Up Arrow */ 
    0,	/* Page Up */ 
  '-', 
    0,	/* Left Arrow */ 
    0, 
    0,	/* Right Arrow */ 
  '+', 
    0,	/* 79 - End key*/ 
    0,	/* Down Arrow */ 
    0,	/* Page Down */ 
    0,	/* Insert Key */ 
    0,	/* Delete Key */ 
    0, 
    0,  
    '<', 
    0,	/* F11 Key */ 
    0,	/* F12 Key */ 
    0,	/* All other keys are undefined */ 
};

static unsigned char sc2ch_shift[128] = {
    0,
    27,
    '!',
    '\"',
    '#',
    '$',
    '%',
    '&',
    '/',
    '(',
    ')',
    '=',
    '?',
    '`',
    '\b',
    '\t',
    'Q',
    'W',
    'E',
    'R',
    'T',
    'Y',
    'U',
    'I',
    'O',
    'P',
    'A',
    'A',
    '\n',
    0,
    'A',
    'S',
    'D',
    'F',
    'G',
    'H',
    'J',
    'K',
    'L',
    'O',
    '\'',
    '>',
    0,
    '*',
    'Z',
    'X',
    'C',
    'V',
    'B',
    'N',
    'M',
    ';',
    ':',
    '_',
    0,
    '*',
    0,
    ' ',
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    '-',
    0,
    0,
    0,
    '+',
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    '>',
    0,
    0,
    0,
};


#endif
