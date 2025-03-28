#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// c usa complemento 2 (complemento a la base para int con signo)

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef	 uint64_t u64;

/* MACROS:
Las macros no son funciones reales en el codigo,
sino que son sustitucciones de texto que se hacen antes de la compilacion
(en el preprocesamiento)
*/

// get the 'n' bit of 'a'
#define BIT(a, n) ((a & (1 << n)) ? 1 : 0)

// set the 'n' bit of 'a' with 1 if 'on' or 0 if !'on'
//#define BIT_SET(a, n, on) (on ? (a) |= (1 << n) : (a) &= ~(1 << n))
#define BIT_SET(a, n, on) {if (on) (a) |= (1 << n); else (a) &= ~(1 << n);}

#define BETWEEN(a, b, c) ((a >= b) && (a <= c))

void delay(u32 ms);

// it's a utility for when a function or part of a functions is not yet implemented
#define NO_IMPLEMENTED { fprintf(stderr, "NOT YET IMPLEMENTED\n"); exit(-5); }

#endif