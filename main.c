/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"


#define I2C_PORT i2c0

/*
  Our 7 Segment display has pins as follows:

  --A--
  F   B
  --G--
  E   C
  --D--

  By default we are allocating GPIO 2 to segment A, 3 to B etc.
  So, connect GPIO 2 to pin A on the 7 segment LED display etc. Don't forget
  the appropriate resistors, best to use one for each segment!

  Connect button so that pressing the switch connects the GPIO 9 (default) to
  ground (pull down)
*/
/*

A1->2
B1->9
C1->14
D1->7
E1->8
F1->3
G1->6
DP1->13


A2->17
B2->16
C2->10
D2->15
E2->12
F2->1
G2->0
DP2->11

*/


//               ={A, B, C,  D, E, F, G, DP}
int LCD1_GPIO [8]={2, 9, 14, 7, 8, 3, 6, 13};
int LCD2_GPIO [8]={10, 16, 17, 1, 0, 15, 12, 11};
// This array converts a number 0-9 to a bit pattern to send to the GPIOs
int bits1[10] = {
        0x438C,  // 0  0100 0011 1000 1100
        0x4200,  // 1  0100 0010 0000 0000
        0x03C4,  // 2  0000 0011 1100 0100
        0x42C4,  // 3  0100 0010 1100 0100
        0x4248,  // 4  0100 0010 0100 1000
        0x40CC,  // 5  0100 0000 1100 1100
        0x41CC,  // 6  0100 0001 1100 1100
        0x4204,  // 7  0100 0010 0000 0100
        0x43CC,  // 8  0100 0011 1100 1100
        0x42CC   // 9  0100 0010 1100 1100
};

int bits2[10] = {
        0x39402,  //0  0011 1001 0100 0000 0010
        0x10400,  //1  0001 0000 0100 0000 0000
        0x39001,  // 2  0011 1001 0000 0000 0001
        0x38401,  // 3  0011 1000 0100 0000 0001
        0x10403,  // 4  0001 0000 0100 0000 0011
        0x28403,  // 5  0010 1000 0100 0000 0011
        0x29403,  // 6  0010 1001 0100 0000 0011
        0x30400,  // 7  0011 0000 0100 0000 0000
        0x39403,  // 8  0011 1001 0100 0000 0011
        0x38403   // 9  0011 1000 0100 0000 0011
};

int maskDigit1All=0x43CC;   // 8  0100 0011 1100 1100
int maskDigit2All=0x39403;  // 8  0011 1001 0100 0000 0011
uint8_t TMP1075_ADDR = 0x48;
uint8_t REG_DEVID = 0x0f;
uint8_t TEMP_REG=0x00;
static const float TMP1075_RESOLUTION=0.0625; 


void initDisplay (){
for (int gpio = 0; gpio < 8; gpio++) {
        gpio_init(LCD1_GPIO[gpio]);
        gpio_init(LCD2_GPIO[gpio]);
        gpio_set_dir(LCD1_GPIO[gpio], GPIO_OUT);
        gpio_set_dir(LCD2_GPIO[gpio], GPIO_OUT);
    }
}

void afisareNumar (int nr){
    initDisplay();
    int unitati = nr%10;
    int zeci = (nr/10)%10;  
    gpio_set_mask(bits1[zeci]);
    gpio_set_mask(bits2[unitati]);
}

void TMP1075_init(){
    sleep_ms(6000);
    uint8_t ID[2];
    i2c_write_blocking(I2C_PORT, TMP1075_ADDR, &REG_DEVID, 1, true);
    i2c_read_blocking(I2C_PORT, TMP1075_ADDR, ID, 1, false);
    char str[8];
    //sprintf(str, "%d", ID[0]);
    if (ID[0]==0x75){
        printf("Conectat cu succes - DEVICE_ID=%x \n", ID[0]);
    }
    else{
         printf("Nu s-a putut realiza conexiunea cu senzorul");
    }
    // while (1){
    //     printf("%s \n", str);s
    //     sleep_ms(1000);
    // }
}

float TMP1075_readTemp(){
    float temp_result=0; 
    uint8_t temp[2];
    i2c_write_blocking(I2C_PORT, TMP1075_ADDR, &TEMP_REG, 1, true);
    i2c_read_blocking(I2C_PORT, TMP1075_ADDR, temp, 2, false);
    uint16_t result=(((uint16_t)0|temp[0])<<8)|(uint16_t)temp[1];
    printf("temp_binar result: %x \n", result);
    temp_result=result/16*TMP1075_RESOLUTION;  
    printf("Temperature: %f \n", temp_result);
    return temp_result; 
}


int main() {
    const uint sda_pin = 4;
    const uint scl_pin = 5;
    stdio_init_all();
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);
    TMP1075_init();
    while (1){
        float temp=TMP1075_readTemp();
        afisareNumar((int) temp);
        sleep_ms(500);
    }
    return 0;
}