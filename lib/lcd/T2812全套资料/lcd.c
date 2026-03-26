#include "c8051F340.h"
#include <intrins.h>
#include <absacc.h>
#include <main.h>
#include <mmc_sd\mmc_sd.h>
#include <lcd\lcd.h>
unsigned char a,b,c,d;
xdata unsigned int reg_data[20];	
//----------------------------------------------------------------------

unsigned char SPI_SendData(unsigned char i)
{ 
#ifdef    SPI_3W
   unsigned char n;
  
   for(n=0; n<8; n++)			
   {  
	  if(i&0x80) SPI_DI=1;
      	else SPI_DI=0;
      i<<= 1;

	  SPI_CLK=0;
      SPI_CLK=1;			
   }
//	     return i;  
#endif 
	 
#ifdef    SPI_4W
	    unsigned char ret;

    SPI0DAT = i;
    while(!SPIF);                      
    SPIF = 0;
    ret = SPI0DAT;
//    return ret;      
#endif 
	 
	     return i;   
}

void WriteComm(uint8 i)
{	
	//-------------------------------------------------	
#ifdef MCU8bit	
		RS=0;	
		CS0=0;
		RD0=1;
		WR0=0;  
		DBL=i;	
		WR0=1; 	
		CS0=1; 
	#endif
//-------------------------------------------------		
	#ifdef MCU16bit	
		RS=0;	
		CS0=0;
		RD0=1;
		WR0=0;  
		DBH=i>>8;
		DBL=i;	
		WR0=1; 	
		CS0=1; 
	#endif
	
//-------------------------------------------------	
	#ifdef SPI	

			#ifdef SPI_4W
			SPI_CS=0;
			SPI_RS=0;
			#endif

    #ifdef SPI_3W
			SPI_CS=0;
			SPI_DI=0;	
			SPI_CLK=0;
			SPI_CLK=1;
			#endif

		SPI_SendData(i);
		SPI_CS=1;
	
#endif	
//-------------------------------------------------		
}

void WriteData(uint8 i)
{
#ifdef MCU8bit		
		RS=1;		
		CS0=0;
		RD0=1;
		WR0=0;  	
		DBL=i; 	
		WR0=1; 
		CS0=1; 
	#endif
#ifdef MCU16bit		
		RS=1;		
		CS0=0;
		RD0=1;
		WR0=0;  	
		DBH=i>>8;
		DBL=i; 	
		WR0=1; 
		CS0=1; 
	#endif
	
#ifdef SPI

	#ifdef SPI_4W
		SPI_CS=0;
		SPI_RS=1;		
	#endif

	#ifdef SPI_3W
		SPI_CS=0;
		SPI_DI=1;	 
		SPI_CLK=0;
		SPI_CLK=1;
	#endif

		SPI_SendData(i);
		SPI_CS=1;
#endif
	
	
}
void DispGrayHor16(void)	 
{
		unsigned int i,j,k;
   	BlockWrite(0,COL-1,0,ROW-1);    	
		CS0=0;
		RD0=1;
		RS=1;		

	
#ifdef MCU8bit

	for(i=0;i<ROW;i++)
	{
		for(j=0;j<COL%16;j++)
		{
			DBL=0;
			WR0=0;
			WR0=1;
		}
		
		for(j=0;j<16;j++)
		{
	        for(k=0;k<COL/16;k++)
			{		
				DBL=((j*2)<<3)|((j*4)>>3);	
				WR0=0;
				WR0=1;
					 
				DBL=((j*4)<<5)|(j*2);
				WR0=0;
				WR0=1;
			} 
		}
	}

#endif
#ifdef MCU16bit
	for(i=0;i<ROW;i++)
	{
		for(j=0;j<COL%16;j++)
		{
			DBH=0;
			DBL=0;
			WR0=0;
			WR0=1;
		}
		
		for(j=0;j<16;j++)
		{
	        for(k=0;k<COL/16;k++)
			{		
				DBH=((j*2)<<3)|((j*4)>>3);		 
				DBL=((j*4)<<5)|(j*2);
				WR0=0;
				WR0=1;
			} 
		}	
	}
#endif
	
	
#ifdef SPI
//i=0;
		for(j=0;j<16;j++)
		{
	        for(k=0;k<COL/16;k++)
			{		
				
							 WriteData(((j*2)<<3)|((j*4)>>3));  

				      WriteData(((j*4)<<5)|(j*2));  	 

			} 
		}	
		
	///--------------------------------------------	
		for(i=0;i<ROW;i++)
	{
		for(j=0;j<COL%16;j++)
		{
WriteData(0x00);
			WR0=0;
			WR0=1;
		}
		
		for(j=0;j<16;j++)
		{
	        for(k=0;k<COL/16;k++)
			{		
			WriteData(((j*2)<<3)|((j*4)>>3));					 
			WriteData(((j*4)<<5)|(j*2));

			} 
		}
	}	
		
		
		
		
	#endif	

	
	CS0=1;
}

void DispPic(unsigned int code *picture)
{
    unsigned int *p;
	unsigned int  i,j; //i-row,j-col
	unsigned char n,k; //n-row repeat count,k-col repeat count

	BlockWrite(0,COL-1,0,ROW-1);

	CS0=0;
	RS =1;
	RD0=1;
#ifdef MCU8bit
	for(n=0;n<ROW/PIC_HEIGHT;n++)         //n-row repeat count
	{
		for(i=0;i<PIC_HEIGHT;i++)
		{
			p=picture;
			for(k=0;k<COL/PIC_WIDTH;k++) //k-col repeat count
		    {
				for(j=0;j<PIC_WIDTH;j++)
		    	{
				    DBL=(*(p+i*PIC_HEIGHT+j))>>8; 
					WR0=0;
					WR0=1; 		
					DBL=*(p+i*PIC_HEIGHT+j);  
					WR0=0;
					WR0=1;
				}

		  	}

			p=picture;
			for(j=0;j<COL%PIC_WIDTH;j++)
		    {
				    DBL=(*(p+i*PIC_HEIGHT+j))>>8; 
					WR0=0;
					WR0=1; 
		
					DBL=*(p+i*PIC_HEIGHT+j);  
					WR0=0;
					WR0=1;
		  	}
		}
	}

	for(i=0;i<ROW%PIC_HEIGHT;i++)
	{
		p=picture;
		for(k=0;k<COL/PIC_WIDTH;k++) //k-col repeat count
	    {
			for(j=0;j<PIC_WIDTH;j++)
	    	{
			    DBL=(*(p+i*PIC_HEIGHT+j))>>8; 
				WR0=0;
				WR0=1; 
	
				DBL=*(p+i*PIC_HEIGHT+j);  
				WR0=0;
				WR0=1;
			}

	  	}

		p=picture;
		for(j=0;j<COL%PIC_WIDTH;j++)
	    {
			    DBL=(*(p+i*PIC_HEIGHT+j))>>8; 
				WR0=0;
				WR0=1; 
	
				DBL=*(p+i*PIC_HEIGHT+j);  
				WR0=0;
				WR0=1;
	  	}
	}
#endif

#ifdef MCU16bit

	for(n=0;n<ROW/PIC_HEIGHT;n++)         //n-row repeat count
	{
		for(i=0;i<PIC_HEIGHT;i++)
		{
			p=picture;
			for(k=0;k<COL/PIC_WIDTH;k++) //k-col repeat count
		    {
				for(j=0;j<PIC_WIDTH;j++)
		    	{
				    DBH=(*(p+i*PIC_HEIGHT+j))>>8; 
					//WR0=0;
					//WR0=1; 
		
					DBL=*(p+i*PIC_HEIGHT+j);  
					WR0=0;
					WR0=1;
				}

		  	}

			p=picture;
			for(j=0;j<COL%PIC_WIDTH;j++)
		    {
				    DBH=(*(p+i*PIC_HEIGHT+j))>>8; 
					//WR0=0;
					//WR0=1; 
		
					DBL=*(p+i*PIC_HEIGHT+j);  
					WR0=0;
					WR0=1;
		  	}
		}
	}

	for(i=0;i<ROW%PIC_HEIGHT;i++)
	{
		p=picture;
		for(k=0;k<COL/PIC_WIDTH;k++) //k-col repeat count
	    {
			for(j=0;j<PIC_WIDTH;j++)
	    	{
			    DBH=(*(p+i*PIC_HEIGHT+j))>>8; 
				//WR0=0;
				//WR0=1; 
	
				DBL=*(p+i*PIC_HEIGHT+j);  
				WR0=0;
				WR0=1;
			}

	  	}

		p=picture;
		for(j=0;j<COL%PIC_WIDTH;j++)
	    {
			    DBH=(*(p+i*PIC_HEIGHT+j))>>8; 
				//WR0=0;
				//WR0=1; 
	
				DBL=*(p+i*PIC_HEIGHT+j);  
				WR0=0;
				WR0=1;
	  	}
	}

#endif
#ifdef SPI
		for(n=0;n<ROW/PIC_HEIGHT;n++)         //n-row repeat count
	{
		for(i=0;i<PIC_HEIGHT;i++)
		{
			p=picture;
			for(k=0;k<COL/PIC_WIDTH;k++) //k-col repeat count
		    {
				for(j=0;j<PIC_WIDTH;j++)
		    	{
							WriteData(((*(p+i*PIC_HEIGHT+j))>>8));  	
              WriteData(*(p+i*PIC_HEIGHT+j)); 
				}

		  	}

			p=picture;
			for(j=0;j<COL%PIC_WIDTH;j++)
		    {
					WriteData(((*(p+i*PIC_HEIGHT+j))>>8));  	
                    WriteData(*(p+i*PIC_HEIGHT+j));
		  	}
		}
	}

	for(i=0;i<ROW%PIC_HEIGHT;i++)
	{
		p=picture;
		for(k=0;k<COL/PIC_WIDTH;k++) //k-col repeat count
	    {
			for(j=0;j<PIC_WIDTH;j++)
	    	{
					WriteData(((*(p+i*PIC_HEIGHT+j))>>8));  	
                    WriteData(*(p+i*PIC_HEIGHT+j));
			}

	  	}

		p=picture;
		for(j=0;j<COL%PIC_WIDTH;j++)
	    {
					WriteData(((*(p+i*PIC_HEIGHT+j))>>8));  	
                    WriteData(*(p+i*PIC_HEIGHT+j));
	  	}
	}
	
	#endif


	CS0=1;
}

void DispScaleVer_Gray(void)
{
	unsigned int i,j,k;
	
	BlockWrite(0,COL-1,0,ROW-1);

	CS0=0;
	RD0=1;
	RS=1;

#ifdef MCU8bit
	for(k=0;k<ROW%32;k++)
	{
		for(j=0;j<COL;j++)
		{
			DBL=0;
			WR0=0;
			WR0=1;
		}			
	}
	
	for(k=0;k<32;k++)
	{	
		for(i=0;i<ROW/32;i++)
		{	
			// GRAY										  
			for(j=0;j<COL;j++)
			{
				DBL=((k<<3)|((k*2)>>3));
				WR0=0;
				WR0=1;

				DBL=((k*2)<<5)|k;
				WR0=0;
				WR0=1;
			}
		}
	
	}
#endif

#ifdef MCU16bit
	for(k=0;k<ROW%32;k++)
	{
		for(j=0;j<COL;j++)
		{
			DBH=0;
			DBL=0;
			WR0=0;
			WR0=1;

		}			
	}
	
	for(k=0;k<32;k++)
	{	
		for(i=0;i<ROW/32;i++)
		{	
			// GRAY										  
			for(j=0;j<COL;j++)
			{
				DBH=(k<<3)|((k*2)>>3);
				DBL=((k*2)<<5)|k;
				WR0=0;
				WR0=1;
			}
		}
	
	}
#endif


#ifdef SPI//-----------------------------------------------------------------------------------------------------------------------------------------------
	
		for(k=0;k<32;k++)
	{	
		for(i=0;i<ROW/32;i++)
		{	
			// GRAY										  
			for(j=0;j<COL;j++)
			{
				
			WriteData((k<<3)|((k*2)>>3));  	
			WriteData(((k*2)<<5)|k);  

			}
		}
	
	}
	
#endif



	CS0=1;
}

void sleep(void)
{
WriteComm(0x28);
Delay(10);
WriteComm(0x10);
Delay(10);
}

void BlockWrite(unsigned int Xstart,unsigned int Xend,unsigned int Ystart,unsigned int Yend) reentrant
{
//	WriteComm(0x2a);   
//	WriteData(Xstart>>8);
//	WriteData(Xstart&0xff);
//	WriteData(Xend>>8);
//	WriteData(Xend&0xff);

//	WriteComm(0x2b);   
//	WriteData(Ystart>>8);
//	WriteData(Ystart&0xff);
//	WriteData(Yend>>8);
//	WriteData(Yend&0xff);

//	WriteComm(0x2c);
	
	
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


void DispColor(unsigned int color)
{
	unsigned int i,j;

	BlockWrite(0,COL-1,0,ROW-1);

	CS0=0; 
	RS=1;
	RD0=1;
#ifdef MCU8bit
			for(i=0;i<ROW;i++)
			{
					for(j=0;j<COL;j++)
				{    
					 DBL=color>>8;
					 WR0=0;  
					 WR0=1;  
			
					 DBL=color;
					 WR0=0;  
					 WR0=1;  
				}
			}

#endif
#ifdef MCU16bit
	DBH=color>>8;
	DBL=color; 

	for(i=0;i<ROW;i++)
	{
	    for(j=0;j<COL;j++)
		{    
			WR0=0;
			WR0=1;
		}
	}

#endif
	
#ifdef SPI
	
	for(i=0;i<ROW;i++)
	{
	    for(j=0;j<COL;j++)
		{    
			 WriteData(color>>8);  	
             WriteData(color);  
		}
	}	
#endif	

	CS0=1; 
}



void DispBand(void)	 
{
	unsigned int i,j,k;
	//unsigned int color[8]={0x001f,0x07e0,0xf800,0x07ff,0xf81f,0xffe0,0x0000,0xffff};
	unsigned int color[8]={0xf800,0xf800,0x07e0,0x07e0,0x001f,0x001f,0xffff,0xffff};//0x94B2
	//unsigned int gray16[]={0x0000,0x1082,0x2104,0x3186,0x42,0x08,0x528a,0x630c,0x738e,0x7bcf,0x9492,0xa514,0xb596,0xc618,0xd69a,0xe71c,0xffff};

   	BlockWrite(0,COL-1,0,ROW-1);
    	
	CS0=0;
	RD0=1;
	RS=1;																									  
#ifdef MCU8bit
	for(i=0;i<8;i++)
	{
		for(j=0;j<ROW/8;j++)
		{
	        for(k=0;k<COL;k++)
			{
				DBL=color[i]>>8;      
				WR0=0;  
				WR0=1;  
   
				DBL=color[i]; 
				WR0=0;  
				WR0=1;  
			} 
		}
	}
	for(j=0;j<ROW%8;j++)
	{
        for(k=0;k<COL;k++)
		{
			DBL=color[7]>>8;      
			WR0=0;  
			WR0=1;  

			DBL=color[7]; 
			WR0=0;  
			WR0=1;  
		} 
	}
	#endif
	
	#ifdef MCU16bit
	for(i=0;i<8;i++)
	{
		for(j=0;j<ROW/8;j++)
		{
	        for(k=0;k<COL;k++)
			{
				DBL=color[i];     
				DBH=color[i]>>8;  
				WR0=0;  
				WR0=1;  
			} 
		}
	}
	for(j=0;j<ROW%8;j++)
	{
        for(k=0;k<COL;k++)
		{
			DBL=color[7];     
			DBH=color[7]>>8;  
			WR0=0;  
			WR0=1;  
		} 
	}	
	
	#endif
	
	#ifdef SPI
	

	for(i=0;i<8;i++)
	{
		for(j=0;j<ROW/8;j++)
		{
	        for(k=0;k<COL;k++)
			{

			 WriteData(color[i]>>8);  	
             WriteData(color[i]);  
			} 
		}
	}
	for(j=0;j<ROW%8;j++)
	{
        for(k=0;k<COL;k++)
		{

			WriteData(color[7]>>8);  	
             WriteData(color[7]); 
		} 
	}	
	
	
	#endif
	
	
	CS0=1;
}

void DispFrame(void)
{
	unsigned int i,j;
	
	BlockWrite(0,COL-1,0,ROW-1);

	CS0=0;
	RD0=1;
	RS=1;
#ifdef MCU8bit
	DBL=0xf8;WR0=0;WR0=1;DBL=0x00,WR0=0;WR0=1;
	for(i=0;i<COL-2;i++){DBL=0xFF;WR0=0;WR0=1;DBL=0xFF;WR0=0;WR0=1;}
	DBL=0x00;WR0=0;WR0=1;DBL=0x1f;WR0=0;WR0=1;

	for(j=0;j<ROW-2;j++)
	{
		DBL=0xf8;WR0=0;WR0=1;DBL=0x00;WR0=0;WR0=1;
		for(i=0;i<COL-2;i++){DBL=0x00;WR0=0;WR0=1;DBL=0x00;WR0=0;WR0=1;}
		DBL=0x00;WR0=0;WR0=1;DBL=0x1F;WR0=0;WR0=1;
	}

	DBL=0xf8;WR0=0;WR0=1;DBL=0x00;WR0=0;WR0=1;
	for(i=0;i<COL-2;i++){DBL=0xFF;WR0=0;WR0=1;DBL=0xFF;WR0=0;WR0=1;}
	DBL=0x00;WR0=0;WR0=1;DBL=0x1F;WR0=0;WR0=1;
#endif
	
#ifdef MCU16bit
	
		DBH=0xf8,DBL=0x00,WR0=0; WR0=1;
	for(i=0;i<COL-2;i++){DBH=0xFF,DBL=0xFF,WR0=0;WR0=1;}
	DBH=0x00,DBL=0x1f,WR0=0;WR0=1;

	for(j=0;j<ROW-2;j++)
	{
		DBH=0xf8,DBL=0x00,WR0=0;WR0=1;
		for(i=0;i<COL-2;i++){DBH=0x00,DBL=0x00,WR0=0;WR0=1;}
		DBH=0x00,DBL=0x1F,WR0=0;WR0=1;
	}

	DBH=0xf8,DBL=0x00,WR0=0;WR0=1;
	for(i=0;i<COL-2;i++){DBH=0xFF,DBL=0xFF,WR0=0;WR0=1;}
	DBH=0x00,DBL=0x1F,WR0=0;WR0=1;
	
#endif
	
#ifdef SPI
	
	WriteData(WHITE>>8);  	
    WriteData(WHITE);
	for(i=0;i<COL-2;i++)
	{
	WriteData(WHITE>>8);  	
    WriteData(WHITE);
	}
    WriteData(WHITE>>8);  	
    WriteData(WHITE);

	for(j=0;j<ROW-2;j++)
	{
		WriteData(WHITE>>8);  	
        WriteData(WHITE);
		for(i=0;i<COL-2;i++)
		{
		WriteData(BLACK>>8);  	
        WriteData(BLACK);
		}
		WriteData(WHITE>>8);  	
        WriteData(WHITE);

	}

	WriteData(WHITE>>8);  	
    WriteData(WHITE);
	for(i=0;i<COL-2;i++)
	{
	WriteData(WHITE>>8);  	
	WriteData(WHITE);
	}
	WriteData(WHITE>>8);  	
    WriteData(WHITE);	
	
#endif	
		
	CS0=1;
}






void WriteOneDot(unsigned int color)
{ 
#ifdef MCU8bit	
	CS0=0;  
	RD0=1;
	RS=1;	

	DBL=color>>8; 
	WR0=0;
	WR0=1;

	DBL=color;		
	WR0=0;
	WR0=1;

	CS0=1;
#endif		
	
	#ifdef MCU16bit	
	CS0=0;  
	RD0=1;
	RS=1;	
	DBH=color>>8; 
	DBL=color;		
	WR0=0;
	WR0=1;
	CS0=1;
#endif	

#ifdef SPI_3W	

		SPI_CS=0;
		SPI_DI=1;	 
		SPI_CLK=0;
		SPI_CLK=1;
		SPI_SendData(color>>8);
		
		SPI_DI=1;	 
		SPI_CLK=0;
		SPI_CLK=1;
		SPI_SendData(color);		
		SPI_CS=1;	
#endif	

#ifdef SPI_4W	


		SPI_CS=0;
		SPI_RS=1;		
		SPI_SendData(color>>8);
		
		SPI_RS=1;		
		SPI_SendData(color);		

		SPI_CS=1;
#endif	
	
}

unsigned char ToOrd(unsigned char ch)
{
	if(ch<32)
	{
		ch=95;
	}
	else if((ch>=32)&&(ch<=47)) //(32~47)空格~/
	{
		ch=(ch-32)+10+62;
	}
	else if((ch>=48)&&(ch<=57))//(48~57)0~9
	{
		ch=ch-48;
	}
	else if((ch>=58)&&(ch<=64))//(58~64):~@
	{
		ch=(ch-58)+10+62+16;
	}
	else if((ch>=65)&&(ch<=126))//(65~126)A~~
	{
		ch=(ch-65)+10;
	}
	else if(ch>126)
	{		
		ch=95;
	}

	return ch;
}

void  DispOneChar(unsigned char ord,unsigned int Xstart,unsigned int Ystart,unsigned int TextColor,unsigned int BackColor)	 // ord:0~95
{													  
   unsigned char i,j;
   unsigned char  *p;
   unsigned char dat;
   unsigned int index;

   BlockWrite(Xstart,Xstart+(FONT_W-1),Ystart,Ystart+(FONT_H-1));

   index = ord;

   if(index>95)	   //95:ASCII CHAR NUM
   		index=95;

   index = index*((FONT_W/8)*FONT_H);	 

   p = ascii;
   p = p+index;

   for(i=0;i<(FONT_W/8*FONT_H);i++)
    {
       dat=*p++;
       for(j=0;j<8;j++)
        {
           if((dat<<j)&0x80)
             {
                WriteOneDot(TextColor);
             }      
           else 
             {
                WriteOneDot(BackColor);	  
             }
         }
     }
}


void DispStr(unsigned char *str,unsigned int Xstart,unsigned int Ystart,unsigned int TextColor,unsigned int BackColor)
{

	while(!(*str=='\0'))
	{
		DispOneChar(ToOrd(*str++),Xstart,Ystart,TextColor,BackColor);

		if(Xstart>((COL-1)-FONT_W))
		{
			Xstart=0;
		    Ystart=Ystart+FONT_H;
		}
		else
		{
			Xstart=Xstart+FONT_W;
		}

		if(Ystart>((ROW-1)-FONT_H))
		{
			Ystart=0;
		}
	}	
	BlockWrite(0,COL-1,0,ROW-1);
}


void DispInt(unsigned int i,unsigned int Xstart,unsigned int Ystart,unsigned int TextColor,unsigned int BackColor)
{
	if(Xstart>((COL-1)-FONT_W*4))
	{
		Xstart=(COL-1)-FONT_W*4;
	}
	if(Ystart>((ROW-1)-FONT_H))
	{
		Ystart=(Ystart-1)-FONT_H;
	}
			
//	DispOneChar((i>>12)%16,Xstart,Ystart,TextColor,BackColor); //ID value
//	DispOneChar((i>>8)%16,Xstart+FONT_W,Ystart,TextColor,BackColor);
	DispOneChar((i>>4)%16,Xstart+FONT_W*2,Ystart,TextColor,BackColor);
	DispOneChar(i%16,Xstart+FONT_W*3,Ystart,TextColor,BackColor); 

	BlockWrite(0,COL-1,0,ROW-1);
}

void Disp_ID_Lcd(void)
{	
		u8 table_V[]="0123456789ABCDEF";	 
		unsigned char i;
		DispStr("READ ID:",15,5,BLUE,BLACK);	
    for(i=0;i<REG_NUM;i++)
	{
			if(i<=6)//小屏只显示7个参数，大于自动忽略
			{
					OLED_ShowChar((i*18),6,table_V[((reg_data[i]&0xf0)>>4)]);
					OLED_ShowChar(((i*18)+8),6,table_V[(reg_data[i]&0x0f)]);
			}
		DispInt(reg_data[i],i*20,22,GREEN,BLACK);	
	}	
	
}


void Disp_ID_clr(void)//清除缓存的ID数据
{
		unsigned char i;
	for(i=0;i<REG_NUM;i++)
	{
		reg_data[i]=0x00;
	}
}


unsigned int ReadData(void)
{
#ifdef MCU8bit
  unsigned int temp;
		DBL = 0xFF; 
		DBH = 0xff;

	CS0 = 0;
	RS  = 1;        
	WR0 = 1;
	RD0 = 0;

	P3MDOUT = 0x00;	 //Set Input Mode Before Read 
	P0MDOUT = 0x00;
	P0MDIN  |= 0xFF; 	
	P3MDIN  |= 0xFF; 
	
  _nop_();  _nop_();_nop_();  _nop_();
  RD0 = 1;  	
  _nop_();  _nop_();_nop_();  _nop_();
	

  temp=DBH;
   temp<<=8;
  temp|=DBL;

  P3MDOUT = 0xFF;
  P0MDOUT = 0xFF;
		
  RD0 = 1;  
	Delay(1);
                    
  CS0 = 1;
  return temp;
#endif	
	
#ifdef MCU16bit
  unsigned int temp;
		DBL = 0xFF; 
		DBH = 0xff;

	CS0 = 0;
	RS  = 1;        
	WR0 = 1;
	RD0 = 0;

	P3MDOUT = 0x00;	 //Set Input Mode Before Read 
	P0MDOUT = 0x00;
	P0MDIN  |= 0xFF; 	
	P3MDIN  |= 0xFF; 
	
  _nop_();  _nop_();_nop_();  _nop_();
  RD0 = 1;  	
  _nop_();  _nop_();_nop_();  _nop_();
	

  temp=DBH;
   temp<<=8;
  temp|=DBL;

  P3MDOUT = 0xFF;
  P0MDOUT = 0xFF;
		
  RD0 = 1;  
	Delay(1);
                    
  CS0 = 1;
  return temp;
#endif	


#ifdef SPI
unsigned int i=0,tmp=0;
P0MDOUT &= ~(1 << 2);		  
P0 |=  (1 << 2); 

		for(i = 0; i < 8; i++)
	{			
		SPI_CLK=0;
		SPI_CLK=1;
		tmp <<= 1;
		if(SPI_DI) 
			tmp |= 0x01;
	}
P0MDOUT |= 0Xff;	
return tmp;
	
#endif	 

}



void DispRegValue(unsigned int RegIndex,unsigned char ParNum)
{
		unsigned char i;
//		xdata unsigned int reg_data[20];	
	

//--------Reset_LCD----------	
//		CS0=0;		
//		RST=0;
//		Delay(10);
//		RST=1;
//		Delay(10);
	
//---------PASSWORD----------
	

//---------------------------	

	
#ifdef MCU
	WriteComm(RegIndex);	
#endif

#ifdef  SPI_3W
		SPI_CS=0;
		SPI_DI=0;	
		SPI_CLK=0;
		SPI_CLK=1;
		SPI_SendData(RegIndex);
#endif

#ifdef  SPI_4W
			SPI_CS=0;
			SPI_RS=0;
			SPI_SendData(RegIndex);			
			XBR0 &= ~(1 << 1);						
#endif


	if(ParNum>20)
		ParNum=20;

	for(i=0;i<ParNum;i++)
	{
		reg_data[i]=ReadData();
	}
	
	
#ifdef  SPI_4W
	XBR0     |= 0x02;	
#endif	
	
	
Disp_ID_Lcd();

}


void LCD_Init(void)
{
	
	
	#ifdef MCU	 //SPI RESET
	CS0=0;	
	
	RST=1;  
	Delay(10);

	RST=0;
	Delay(10);

	RST=1;
	Delay(10);
#endif
		
#ifdef SPI	 //SPI RESET
	SPI_CS=0;	
	SPI_RES=1;  
	Delay(10);
	SPI_RES=0;
	Delay(10);
	SPI_RES=1;
	Delay(50);
#endif


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

void Disp_Pro_Name(void)
{

		OLED_ShowString(0,0,"KM200_TEST      ");
		OLED_ShowString(0,2,"HX8347D_CMI2.8  ");
		OLED_ShowString(0,4,"T2812-M106-7D   ");
}

