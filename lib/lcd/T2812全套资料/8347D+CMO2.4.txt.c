/*************************************************/
void initi(void)
{

   res=1;
   delay(10);
   res=0;
   delay(5);
   res=1;
   delay(10);
//***************************************************************//LCD SETING
  write_command(0xEA);//PTBA[15:8]
  write_data(0x00,0x00);
  write_command(0xEB);//PTBA[7:0]
  write_data(0x00,0x20);
  write_command(0xEC);//STBA[15:8]
  write_data(0x00,0x08);//0C
  write_command(0xED);//STBA[7:0]
  write_data(0x00,0xC4);
  write_command(0xE8);//OPON[7:0]
  write_data(0x00,0x40);
  write_command(0xE9);//OPON1[7:0]
  write_data(0x00,0x38);
  write_command(0xF1);//OTPS1B
  write_data(0x00,0x01);
  write_command(0xF2);//GEN
  write_data(0x00,0x10);
  write_command(0x27);
  write_data(0x00,0xA3);

//************************GAMMA SETING***********************************************//
  write_command(0x40);
  write_data(0x00,0x01);
  write_command(0x41);
  write_data(0x00,0x07);
  write_command(0x42);
  write_data(0x00,0x07);
  write_command(0x43);
  write_data(0x00,0x13);
  write_command(0x44);
  write_data(0x00,0x11);
  write_command(0x45);
  write_data(0x00,0x24);
  write_command(0x46);
  write_data(0x00,0x10);
  write_command(0x47);
  write_data(0x00,0x57);
  write_command(0x48);
  write_data(0x00,0x09);
  write_command(0x49);
  write_data(0x00,0x14);
  write_command(0x4A);
  write_data(0x00,0x19);
  write_command(0x4B);
  write_data(0x00,0x19);
  write_command(0x4C);
  write_data(0x00,0x16);
/////////////////
  write_command(0x50);
  write_data(0x00,0x1b);
  write_command(0x51);
  write_data(0x00,0x2e);
  write_command(0x52);
  write_data(0x00,0x2c);
  write_command(0x53);
  write_data(0x00,0x38);
  write_command(0x54);
  write_data(0x00,0x38);
  write_command(0x55);
  write_data(0x00,0x3e);
  write_command(0x56);
  write_data(0x00,0x2a);
  write_command(0x57);
  write_data(0x00,0x6f);
  write_command(0x58);
  write_data(0x00,0x09);
  write_command(0x59);
  write_data(0x00,0x06);
  write_command(0x5A);
  write_data(0x00,0x06);
  write_command(0x5B);
  write_data(0x00,0x0b);
  write_command(0x5C);
  write_data(0x00,0x16);
  write_command(0x5D);
  write_data(0x00,0xCC);

//***********************//Power Voltage Setting**********************************
  write_command(0x1B);//VRH=4.65V
  write_data(0x00,0x1B);
  write_command(0x1A);//BT (VGH~15V,VGL~-10V,DDVDH~5V)
  write_data(0x00,0x01);
  write_command(0x24);//VMH
  write_data(0x00,0x2F);//2f
  write_command(0x25);//VML
  write_data(0x00,0x57);//55
//****VCOM offset**///
  write_command(0x23);//for Flicker adjust //can reload from OTP
  write_data(0x00,0x95);//92
///////POWER SETING/////////////////////////////////////////////
   write_command(0x18);//I/P_RADJ,N/P_RADJ, Normal mode 75Hz
   write_data(0x00,0x37);
   write_command(0x19);//OSC_EN='1', start Osc
   write_data(0x00,0x01);
   write_command(0x01);//DP_STB='0', out deep sleep
   write_data(0x00,0x00);
   write_command(0x1F);// GAS=1, VOMG=00, PON=0, DK=1, XDK=0, DVDH_TRI=0, STB=0
   write_data(0x00,0x88);
   delay(5);
   write_command(0x1F);// GAS=1, VOMG=00, PON=0, DK=0, XDK=0, DVDH_TRI=0, STB=0
   write_data(0x00,0x80);
   delay(5);
   write_command(0x1F);// GAS=1, VOMG=00, PON=1, DK=0, XDK=0, DVDH_TRI=0, STB=0
   write_data(0x00,0x90);
   delay(5);
   write_command(0x1F);// GAS=1, VOMG=10, PON=1, DK=0, XDK=0, DDVDH_TRI=0, STB=0
   write_data(0x00,0xD0);
   delay(5);
//****************262k/65k color selection***************
   write_command(0x17);
   write_data(0x00,0x05); //default 0x06 262k color // 0x05 65k color
//***********SET PANEL**********************
   write_command(0x36);//SS_P, GS_P,REV_P,BGR_P
   write_data(0x00,0x00);
//**Display ON Setting*************************
    write_command(0x28);//GON=1, DTE=1, D=1000
    write_data(0x00,0x38);
    delay(40);
    write_command(0x28);//GON=1, DTE=1, D=1100
    write_data(0x00,0x3C);
//***************Set GRAM Area *************************
   write_command(0x02);
   write_data(0x00,0x00);
   write_command(0x03);//Column Start
   write_data(0x00,0x00);
   write_command(0x04);
   write_data(0x00,0x00);
   write_command(0x05);
   write_data(0x00,0xEF);//Column End

   write_command(0x06);
   write_data(0x00,0x00);
   write_command(0x07); //Row Start
   write_data(0x00,0x00);
   write_command(0x08);
   write_data(0x00,0x01);
   write_command(0x09); //Row End
   write_data(0x00,0x3F); 
   //*************************************************************
   write_command(0x22); //Start GRAM write   
}
//****************************************************************************

void EnterSleep (void)
{
    write_command(0x28);//GON=1, DTE=1, D=1000
    write_data(0x00,0x38);
    delay(40);
    write_command(0x1F);// GAS=1, VOMG=00, PON=0, DK=1, XDK=0, DVDH_TRI=0, STB=1
    write_data(0x00,0x89);
    delay(40);
   write_command(0x28);//GON=¡¯0¡¯ DTE=?
    write_data(0x00,0x04);
   delay(40);
   //write_command(0x28);//GON=¡¯1 DTE=¡¯0¡¯ D[1:0]=?
    //write_data(0x00,0x24);
   //delay(40);
   write_command(0x19);//OSC_EN=¡¯0?
    write_data(0x00,0x00);
   delay(5);
  }

//*********************************************************
void ExitSleep (void)

 {
    write_command(0x18);//I/P_RADJ,N/P_RADJ, Normal mode 75Hz
    write_data(0x00,0x36);
    write_command(0x19);//OSC_EN='1', start Osc
    write_data(0x00,0x01);
    write_command(0x1F);// GAS=1, VOMG=00, PON=0, DK=1, XDK=0, DVDH_TRI=0, STB=0?
    write_data(0x00,0x88);
    delay(5);
   write_command(0x1F);// GAS=1, VOMG=00, PON=0, DK=0, XDK=0, DVDH_TRI=0, STB=0
   write_data(0x00,0x80);
   delay(5);
   write_command(0x1F);// GAS=1, VOMG=00, PON=1, DK=0, XDK=0, DVDH_TRI=0, STB=0
   write_data(0x00,0x90);
   delay(5);
   write_command(0x1F);// GAS=1, VOMG=10, PON=1, DK=0, XDK=0, DDVDH_TRI=0, STB=0
   write_data(0x00,0xD0);
   delay(5);
   write_command(0x28);//GON=¡¯1 DTE=¡¯0¡¯ D[1:0]
    write_data(0x00,0x38);
   delay(40);
   write_command(0x28);//OSC_EN=¡¯0?
    write_data(0x00,0x3C);

   }
