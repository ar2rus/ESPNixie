#include "Nixie.h"

#include <SPI.h>
#include <Ticker.h>

volatile char nixie_cnt;
volatile char nixie_digits[NIXIE_DIGITS_COUNT];

Ticker esp_ticker;

void nixie_update() {
    if (++nixie_cnt > NIXIE_GROUP_COUNT){
        nixie_cnt = 0;
    }
    
    SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
    digitalWrite(SPI_PIN_SS, LOW);
    char d_h = nixie_digits[0+nixie_cnt];
    SPI.transfer(digit_group_h(d_h, nixie_cnt==2, nixie_cnt==1, nixie_cnt==0));
    char d_l = nixie_digits[3+nixie_cnt];
    SPI.transfer(digit_group_l(d_l, nixie_cnt==2, nixie_cnt==1, nixie_cnt==0));
    digitalWrite(SPI_PIN_SS, HIGH); 
    SPI.endTransaction();
}

void nixie_init(){
  pinMode(SPI_PIN_SS, OUTPUT);
  SPI.begin();
  
  nixie_clear();
  esp_ticker.attach_ms(NIXIE_UPDATE_PERIOD, nixie_update);
}

void nixie_set(char d5, char d4, char d3, char d2, char d1, char d0){
    nixie_digits[0]=d0;
    nixie_digits[1]=d1;
    nixie_digits[2]=d2;
    nixie_digits[3]=d3;
    nixie_digits[4]=d4;
    nixie_digits[5]=d5;
}

void nixie_set(char* digit_codes){
  nixie_set(digit_codes[5], digit_codes[4], digit_codes[3], digit_codes[2], digit_codes[1], digit_codes[0]);
}

void nixie_clear(){
  char d = digit_off;
  nixie_set(d, d, d, d, d, d);
}

void nixie_clear_force(){
  nixie_clear();
  for (int i=0; i<NIXIE_GROUP_COUNT; i++){
    nixie_update();
  }
}

uint8_t nixie_int(uint32_t v, uint8_t pos_1, uint8_t min_num_digits, char* digits){
  if (min_num_digits >=1 && pos_1 >= 0 && pos_1 < NIXIE_DIGITS_COUNT){

    int8_t pos = pos_1;
    do{
      digits[pos] = v % 10;
      v /= 10;
    }while(++pos < NIXIE_DIGITS_COUNT && (v > 0 || (pos-pos_1 < min_num_digits)));
    
    for (int i=0; i<NIXIE_DIGITS_COUNT; i++){
      digits[i] = digit_code(digits[i], (i>=pos_1 && i<pos), 0);
    }
    return 1;
  }
  return 0;
}

void nixie_set(char* digits, uint8_t offset, uint8_t count, uint8_t pos_1){
  char code[NIXIE_DIGITS_COUNT];

  for (int i=0; i<NIXIE_DIGITS_COUNT; i++){
    if (i>=pos_1 && i-pos_1 < count){
      code[i] = digit_code(digits[i-pos_1+offset], 1, 0);
    }else{
      code[i] = digit_off;
    }
  }
  nixie_set(code);
}

void nixie_set(uint32_t v, uint8_t pos_1){
  char digits[NIXIE_DIGITS_COUNT];

  if (nixie_int(v, pos_1, 1, digits)){
    nixie_set(digits);
  }
}

void nixie_set(float v, uint8_t pos_1, int num_frac){
  for (int i=0; i<num_frac; i++){
    v *= 10;
  }
  uint32_t b = (uint32_t)abs(v);

  char digits[NIXIE_DIGITS_COUNT];
  if (nixie_int(b, pos_1-num_frac, num_frac+1, digits)){
    digits[pos_1] = digit_code(digit_value(digits[pos_1]), 1, num_frac>0); //add point
    nixie_set(digits);
  }
}
