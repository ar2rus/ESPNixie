#ifndef NIXIE_h
#define NIXIE_h

#include <stdint.h>

#define SPI_PIN_SS 15

//ms
#define NIXIE_UPDATE_PERIOD 5

#define NIXIE_GROUP_COUNT 2

#define NIXIE_DIGITS_COUNT 6

typedef char nixie_codes[NIXIE_DIGITS_COUNT];

#define _BV(bit) (1 << (bit))
#define _TB(v, bit) (v & _BV(bit))


#define digit_code(digit_value, digit_enable, point_enable) (((digit_enable & 0x01)<<0x07) | ((point_enable & 0x01)<<0x06) | (digit_value & 0x0F))
#define digit_off (digit_code(0,0,0))

#define digit_value(code) (code & 0x0F)
#define digit_enable(code)(_TB(code, 7))
#define point_enable(code)(_TB(code, 6))

#define digit_code_group_h(d) ((_TB(d, 0) ? _BV(4) : 0) | (_TB(d, 1) ? _BV(2) : 0) | (_TB(d, 2) ? _BV(1) : 0) | (_TB(d, 3) ? _BV(3) : 0))
#define digit_code_group_l(d) ((_TB(d, 0) ? _BV(7) : 0) | (_TB(d, 1) ? _BV(5) : 0) | (_TB(d, 2) ? _BV(4) : 0) | (_TB(d, 3) ? _BV(6) : 0))

#define digit_group_h(d, r0, r1, r2)(digit_code_group_h(digit_value(d)) | (point_enable(d) ? _BV(7) : 0) |\
  (digit_enable(d) && r0 ? _BV(0) : 0) | (digit_enable(d) && r1 ? _BV(5) : 0) | (digit_enable(d) && r2 ? _BV(6) : 0))
#define digit_group_l(d, r0, r1, r2)(digit_code_group_l(digit_value(d)) | (point_enable(d) ? _BV(3) : 0) |\
  (digit_enable(d) && r0 ? _BV(2) : 0) | (digit_enable(d) && r1 ? _BV(1) : 0) | (digit_enable(d) && r2 ? _BV(0) : 0))

void nixie_init();

uint8_t nixie_int(uint32_t v, uint8_t pos_1, uint8_t min_num_digits, char* digits);

void nixie_set(char d5, char d4, char d3, char d2, char d1, char d0);

void nixie_set(char* digit_codes);

void nixie_set(uint32_t v, uint8_t pos_1);

void nixie_set(float v, uint8_t pos_1, int num_frac);

//выводит из массива digits count цифр начиная с цифры с индексом offset в позицию pos_1
void nixie_set(char* digits, uint8_t offset, uint8_t count, uint8_t pos_1);

void nixie_clear();

void nixie_clear_force();

#endif
