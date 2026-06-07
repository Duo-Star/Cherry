#include <SPI.h>
// EPD
#include "Display_EPD_W21_spi.h"
#include "Display_EPD_W21.h"
#include "Ap_29demo.h"

void setup()
{
  // 1. 配置墨水屏控制引脚的模式
  pinMode(4, INPUT);  // BUSY 引脚为输入
  pinMode(5, OUTPUT); // RES/RST 引脚为输出
  pinMode(6, OUTPUT); // DC 引脚为输出
  pinMode(7, OUTPUT); // CS1 引脚为输出
  //
  SPI.begin(12, -1, 11, 7);
  // SPI
  SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
  SPI.begin();
}

// Tips//
/*
1.Flickering is normal when EPD is performing a full screen update to clear ghosting from the previous image so to ensure better clarity and legibility for the new image.
2.There will be no flicker when EPD performs a partial refresh.
3.Please make sue that EPD enters sleep mode when refresh is completed and always leave the sleep mode command. Otherwise, this may result in a reduced lifespan of EPD.
4.Please refrain from inserting EPD to the FPC socket or unplugging it when the MCU is being powered to prevent potential damage.)
5.Re-initialization is required for every full screen update.
6.When porting the program, set the BUSY pin to input mode and other pins to output mode.
*/
void loop()
{
  unsigned char i;
#if 1 // Full screen refresh, fast refresh, and partial refresh demostration.

  EPD_HW_Init();                               // Full screen refresh initialization.
  EPD_WhiteScreen_ALL(gImage_BW1, gImage_RW1); // To Display one image using full screen refresh.
  EPD_DeepSleep();                             // Enter the sleep mode and please do not delete it, otherwise it will reduce the lifespan of the screen.
  delay(3000);                                 // Delay for 3s.

#if 0 // Fast refresh demostration.
			EPD_HW_Init_Fast(); //Full screen refresh initialization.
		  EPD_WhiteScreen_ALL_Fast(gImage_BW1,gImage_RW1); //To Display one image using full screen refresh.
		  EPD_DeepSleep(); //Enter the sleep mode and please do not delete it, otherwise it will reduce the lifespan of the screen.
			delay(3000); //Delay for 3s.

#endif
#if 1 // Partial refresh demostration.
  // Partial refresh demo support displaying a clock at 5 locations with 00:00.  If you need to perform partial refresh more than 5 locations, please use the feature of using partial refresh at the full screen demo.
  // After 5 partial refreshes, implement a full screen refresh to clear the ghosting caused by partial refreshes.
  //////////////////////Partial refresh time demo/////////////////////////////////////
  EPD_HW_Init();                                               // Electronic paper initialization.
  EPD_SetRAMValue_BaseMap(gImage_BWbasemap, gImage_RWbasemap); // Please do not delete the background color function, otherwise it will cause unstable display during partial refresh.

  for (i = 0; i < 6; i++)
    EPD_Dis_Part_Num(64, 136 + 32 * 0, Num[5 - i],      // x-A,y-A,DATA-A
                     64, 136 + 32 * 1, Num[i],          // x-B,y-B,DATA-B
                     64, 136 + 32 * 2, gImage_dot,      // x-C,y-C,DATA-C
                     64, 136 + 32 * 3, Num[1], 32, 64); // x-D,y-D,DATA-D,Resolution  32*64
  EPD_Dis_Part_Num(64, 136 + 32 * 0, Num[8],            // x-A,y-A,DATA-A
                   64, 136 + 32 * 1, Num[1],            // x-B,y-B,DATA-B
                   64, 136 + 32 * 2, gImage_dot,        // x-C,y-C,DATA-C
                   64, 136 + 32 * 3, Num[2], 32, 64);   // x-D,y-D,DATA-D,Resolution  32*64

  EPD_DeepSleep(); // Enter the sleep mode and please do not delete it, otherwise it will reduce the lifespan of the screen.
  delay(3000);     // Delay for 3s.
  // Full screen update clear the screen.
  EPD_HW_Init();           // Full screen refresh initialization.
  EPD_WhiteScreen_White(); // Clear screen function.
  EPD_DeepSleep();         // Enter the sleep mode and please do not delete it, otherwise it will reduce the lifespan of the screen.
  delay(3000);             // Delay for 3s.
#endif

#if 0 // Demonstration of full screen refresh with 180-degree rotation, to enable this feature, please change 0 to 1.
		/************Full display(2s)*******************/
		EPD_HW_Init_180(); //Full screen refresh initialization.
		EPD_WhiteScreen_ALL(gImage_BW1,gImage_RW1); //To Display one image using full screen refresh.
		EPD_DeepSleep(); //Enter the sleep mode and please do not delete it, otherwise it will reduce the lifespan of the screen.
		delay(3000); //Delay for 3s.
#endif
#endif
  while (1)
    ; // The program stops here
}

//////////////////////////////////END//////////////////////////////////////////////////
