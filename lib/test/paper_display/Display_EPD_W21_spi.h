#ifndef _DISPLAY_EPD_W21_SPI_
#define _DISPLAY_EPD_W21_SPI_
#include "Arduino.h"

#define isEPD_W21_BUSY digitalRead(4) // BUSY 接 GPIO 4

#define EPD_W21_RST_0 digitalWrite(5, LOW) // RST 接 GPIO 5
#define EPD_W21_RST_1 digitalWrite(5, HIGH)

#define EPD_W21_DC_0 digitalWrite(6, LOW) // DC 接 GPIO 6
#define EPD_W21_DC_1 digitalWrite(6, HIGH)

#define EPD_W21_CS_0 digitalWrite(7, LOW) // CS1 接 GPIO 7
#define EPD_W21_CS_1 digitalWrite(7, HIGH)

void SPI_Write(unsigned char value);
void EPD_W21_WriteDATA(unsigned char datas);
void EPD_W21_WriteCMD(unsigned char command);

#endif
