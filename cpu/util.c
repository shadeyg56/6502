#include "util.h"

uint8_t bcd_add(uint8_t a, uint8_t b, uint8_t *carry) {

    uint8_t carry_lo = 0;
    uint8_t result_lo = (a & 0x0F) + (b & 0x0F) + *carry;
    if (result_lo > 0x09) {
        result_lo += 6;
        carry_lo = 1;
    }

    uint8_t result_hi = ((a & 0xF0) >> 4) + ((b & 0xF0) >> 4) + carry_lo;
    if (result_hi > 0x09) {
        result_hi += 6;
        *carry = 1;
    } else {
        *carry = 0;
    }

    return ((result_hi & 0x0F) << 4) | (result_lo & 0x0F);
}

uint8_t bcd_subtract(uint8_t a, uint8_t b, uint8_t *borrow) {
    uint8_t borrow_lo = 0;
    uint8_t result_lo = 0;
    uint8_t result_hi = 0;
    uint8_t a_lo = a & 0x0F;
    uint8_t b_lo = b & 0x0F;
    uint8_t a_hi = (a & 0xF0) >> 4;
    uint8_t b_hi = (b & 0xF0) >> 4;
    if (a_lo < b_lo + *borrow) {
        result_lo = a_lo - b_lo + 10 - *borrow;
        borrow_lo = 1;
    } else {
        result_lo = a_lo - b_lo - *borrow;
    }

    if (a_hi < b_hi + borrow_lo) {
        result_hi = a_hi - b_hi + 10 - borrow_lo;
        *borrow = 1;
    } else {
        result_hi = a_hi - b_hi - borrow_lo;
        *borrow = 0;
    }

    return ((result_hi & 0x0F) << 4) | (result_lo & 0x0F);
}