
void WriteComm(uint8 i)
{	

		RS=0;	
		CS0=0;
		RD0=1;
		WR0=0;  
		DBH=i>>8;
		DBL=i;	
		WR0=1; 	
		CS0=1; 

}

void WriteData(uint8 i)
{

		RS=1;		
		CS0=0;
		RD0=1;
		WR0=0;  	
		DBH=i>>8;
		DBL=i; 	
		WR0=1; 
		CS0=1; 

}

void BlockWrite(unsigned int Xstart,unsigned int Xend,unsigned int Ystart,unsigned int Yend) reentrant
{	
WriteComm(0x02);WriteData(Xstart>>8);
WriteComm(0x03);WriteData(Xstart&0xff); //Column Start
WriteComm(0x04);WriteData(Xend>>8);
WriteComm(0x05);WriteData(Xend&0xff); //Column End
	
	
WriteComm(0x06);WriteData(Ystart>>8);
WriteComm(0x07);WriteData(Ystart&0xff); //Row Start
WriteComm(0x08);WriteData(Yend>>8);
WriteComm(0x09);WriteData(Yend&0xff); //Row End	
	
	
//WriteComm(0x02);WriteData(0x00);
//WriteComm(0x03);WriteData(0x00); //Column Start
//WriteComm(0x04);WriteData(0x00);
//WriteComm(0x05);WriteData(0xEF); //Column End
//WriteComm(0x06);WriteData(0x00);
//WriteComm(0x07);WriteData(0x00); //Row Start
//WriteComm(0x08);WriteData(0x01);
//WriteComm(0x09);WriteData(0x3F); //Row End
WriteComm(0x22); //Start GRAM write	
	
}



void LCD_Init(void)
{
	
	

	RST=1;  
	Delay(5);

	RST=0;
	Delay(10);

	RST=1;
	Delay(120);



//------------------------------------LCD_CODE------------------------------------------------------------------------------------------------------------------------//
WriteComm(0x2E);WriteData(0x79); //
WriteComm(0xEE);WriteData(0x0C); //
//Driving ability Setting
WriteComm(0xEA);WriteData(0x00); //PTBA[15:8]
WriteComm(0xEB);WriteData(0x20); //PTBA[7:0]
WriteComm(0xEC);WriteData(0x08); //STBA[15:8]
WriteComm(0xED);WriteData(0xC4); //STBA[7:0]
WriteComm(0xE8);WriteData(0x40); //OPON[7:0]
WriteComm(0xE9);WriteData(0x38); //OPON1[7:0]
WriteComm(0xF1);WriteData(0x01); //OTPS1B
WriteComm(0xF2);WriteData(0x10); //GEN
WriteComm(0x27);WriteData(0xA3); //
WriteComm(0x2f);WriteData(0x00);

//Gamma 2.2 Setting
WriteComm(0x40);WriteData(0x00); //
WriteComm(0x41);WriteData(0x00); //
WriteComm(0x42);WriteData(0x01); //
WriteComm(0x43);WriteData(0x13); //
WriteComm(0x44);WriteData(0x10); //
WriteComm(0x45);WriteData(0x26); //
WriteComm(0x46);WriteData(0x08); //
WriteComm(0x47);WriteData(0x51); //
WriteComm(0x48);WriteData(0x02); //
WriteComm(0x49);WriteData(0x12); //
WriteComm(0x4A);WriteData(0x18); //
WriteComm(0x4B);WriteData(0x19); //
WriteComm(0x4C);WriteData(0x14); //
WriteComm(0x50);WriteData(0x19); //
WriteComm(0x51);WriteData(0x2F); //
WriteComm(0x52);WriteData(0x2C); //
WriteComm(0x53);WriteData(0x3E); //
WriteComm(0x54);WriteData(0x3F); //
WriteComm(0x55);WriteData(0x3F); //
WriteComm(0x56);WriteData(0x2E); //
WriteComm(0x57);WriteData(0x77); //
WriteComm(0x58);WriteData(0x0B); //
WriteComm(0x59);WriteData(0x06); //
WriteComm(0x5A);WriteData(0x07); //
WriteComm(0x5B);WriteData(0x0D); //
WriteComm(0x5C);WriteData(0x1D); //
WriteComm(0x5D);WriteData(0xCC); //
//Power Voltage Setting
WriteComm(0x1B);WriteData(0x1B); //VRH=4.65V
WriteComm(0x1A);WriteData(0x01); //BT (VGH~15V);WriteData(VGL~-10V);WriteData(DDVDH~5V)
WriteComm(0x24);WriteData(0x2F); //VMH(VCOM High voltage ~3.2V)
WriteComm(0x25);WriteData(0x57); //VML(VCOM Low voltage -1.2V)
//****VCOM offset**///
WriteComm(0x23);WriteData(0x92); //for Flicker adjust //can reload from OTP
//Power on Setting
WriteComm(0x18);WriteData(0x3b); //I/P_RADJ);WriteData(N/P_RADJ);WriteData( Normal mode 75Hz
WriteComm(0x19);WriteData(0x01); //OSC_EN='1');WriteData( start Osc
WriteComm(0x01);WriteData(0x00); //DP_STB='0');WriteData( out deep sleep
WriteComm(0x1F);WriteData(0x88);// GAS=1);WriteData( VOMG=00);WriteData( PON=0);WriteData( DK=1);WriteData( XDK=0);WriteData( DVDH_TRI=0);WriteData( STB=0
Delay(5);
WriteComm(0x1F);WriteData(0x80);// GAS=1);WriteData( VOMG=00);WriteData( PON=0);WriteData( DK=0);WriteData( XDK=0);WriteData( DVDH_TRI=0);WriteData( STB=0
Delay(5);
WriteComm(0x1F);WriteData(0x90);// GAS=1);WriteData( VOMG=00);WriteData( PON=1);WriteData( DK=0);WriteData( XDK=0);WriteData( DVDH_TRI=0);WriteData( STB=0
Delay(5);
WriteComm(0x1F);WriteData(0xD0);// GAS=1);WriteData( VOMG=10);WriteData( PON=1);WriteData( DK=0);WriteData( XDK=0);WriteData( DDVDH_TRI=0);WriteData( STB=0
Delay(5);
//262k/65k color selection
WriteComm(0x17);WriteData(0x05); //default 0x06 262k color // 0x05 65k color
//SET PANEL
WriteComm(0x36);WriteData(0x00); //SS_P);WriteData( GS_P);WriteData(REV_P);WriteData(BGR_P
//Display ON Setting
WriteComm(0x28);WriteData(0x38); //GON=1);WriteData( DTE=1);WriteData( D=1000
Delay(40);
WriteComm(0x28);WriteData(0x3C); //GON=1);WriteData( DTE=1);WriteData( D=1100
//Set GRAM Area
WriteComm(0x02);WriteData(0x00);
WriteComm(0x03);WriteData(0x00); //Column Start
WriteComm(0x04);WriteData(0x00);
WriteComm(0x05);WriteData(0xEF); //Column End
WriteComm(0x06);WriteData(0x00);
WriteComm(0x07);WriteData(0x00); //Row Start
WriteComm(0x08);WriteData(0x01);
WriteComm(0x09);WriteData(0x3F); //Row End
WriteComm(0x22); //Start GRAM write
}

