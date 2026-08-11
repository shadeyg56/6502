#ifndef _UTIL_H
#define _UTIL_H

#include <stdint.h>

uint8_t bcd_add(uint8_t a, uint8_t b, uint8_t *carry);
uint8_t bcd_subtract(uint8_t a, uint8_t b, uint8_t *borrow);

#endif