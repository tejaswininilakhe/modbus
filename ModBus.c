
#include "LCD.h"





void vClearBuffer(void);
 void vSendPacket(unsigned char *Data,unsigned char ucLen);
#define START_TIMER_1   CS10_bit = 0;CS11_bit = 1;CS12_bit = 0;
#define STOP_TIMER_1   CS10_bit = 0;CS11_bit = 0;CS12_bit = 0;
#define CLEAR_TIMER_1  TCNT1H=0;TCNT1L=0;
bit bSec;

#define LED_DDR  DDB5_bit
#define LED_DATA PORTB5_bit

#define F_CPU 16000000
#define USART_BAUDRATE_9600 9600
#define USART_BAUDRATE_250000 250000

#define BAUD_PRESCALE_9600 (((F_CPU / (USART_BAUDRATE_9600 * 16UL))) - 1)
#define BAUD_PRESCALE_250000 (((F_CPU / (USART_BAUDRATE_250000 * 16UL))) - 1)



unsigned char  ucVoltage[] = {0x01,0x03,0x63,0x06,0x00,0x01,0x7A,0x4F};
unsigned char  ucCurrent[] = {0x01,0x03,0x63,0x02,0x00,0x01,0x3B,0x8E};
unsigned char  ucFreq[]    = {0x01,0x03,0x63,0x01,0x00,0x01,0xCB,0x8E};


unsigned char ucPacketIndex;
unsigned char ucPacket[20];

bit bCommStatus;

unsigned char ucKeyPress;
void USART_Init(void);
void USART_SendByte(unsigned char u8Data);
void vInitTimer_1(void);
unsigned int uiVoltage;
unsigned int uiCurrent;
unsigned int uiFreq;
unsigned char ucBuff[50];
void main()
{

 unsigned char ucData,ucData1;
 unsigned char i;
 LED_DDR = 1;
 USART_Init();
 vInitTimer_1();
 SREG_I_bit = 1;
 Lcd_Init();                        // Initialize LCD
  Lcd_Cmd(_LCD_CLEAR);               // Clear display
  Lcd_Cmd(_LCD_CURSOR_OFF);          // Cursor off
  Lcd_Out(1,1,"  Lakshmi Solar ");                 // Write text in first row

 USART_SendByte('L');
 USART_SendByte('A');
 USART_SendByte('K');
 USART_SendByte('S');
 USART_SendByte('H');
 USART_SendByte('M');
 USART_SendByte('I');
 LED_DATA = 0;
 vClearBuffer();
 while(1)
 {
   ucPacketIndex=0;
   SREG_I_bit = 0;
   RXEN0_bit = 0;
   vSendPacket(&ucVoltage,8);
   SREG_I_bit = 1;
   RXEN0_bit = 1;
   delay_ms(1000);
   if(ucPacket[2] != 0x83 && ucPacket[1] != 0x00)
   {
    uiVoltage = ((unsigned int)(ucPacket[4]<<8))|ucPacket[5];
    bCommStatus = 1;
   }
   else
   {
    bCommStatus = 0;
   }
   vClearBuffer();
   
   ucPacketIndex=0;
   SREG_I_bit = 0;
   RXEN0_bit = 0;
   vSendPacket(&ucCurrent,8);
   SREG_I_bit = 1;
   RXEN0_bit = 1;
   delay_ms(1000);
   if(ucPacket[2] != 0x83 && ucPacket[1] != 0x00)
   {
    uiCurrent = ((unsigned int)(ucPacket[4]<<8))|ucPacket[5];
    bCommStatus = 1;
   }
   else
   {
    bCommStatus = 0;
   }
   vClearBuffer();

   ucPacketIndex=0;
   SREG_I_bit = 0;
   RXEN0_bit = 0;
   vSendPacket(&ucFreq,8);
   SREG_I_bit = 1;
   RXEN0_bit = 1;
   delay_ms(1000);
   if(ucPacket[2] != 0x83 && ucPacket[1] != 0x00)
   {
    uiFreq = ((unsigned int)(ucPacket[4]<<8))|ucPacket[5];
    bCommStatus = 1;
   }
   else
   {
    bCommStatus = 0;
   }
   vClearBuffer();

   if(bCommStatus)
   {
    sprintf(ucBuff,"V%3u I%5.1f F%3u",uiVoltage,(uiCurrent/(float)10),(uiFreq));
    
    sprintf(ucBuff,"V%3u I%3.1f F%3.1f",uiVoltage,(uiCurrent/(float)10),(float)(uiFreq/(float)100));
    Lcd_Out(2,1,ucBuff);
   }
   else
   {
    sprintf(ucBuff,"V--- I--- F-----");
    Lcd_Out(2,1,ucBuff);
   }
 }
}



void vSerialRx_ISR() org IVT_ADDR_USART__RX      // This funtion is Interrupt Funtion
{
 unsigned char dmxByte;
 if ( FE0_bit)
 {
  dmxByte = UDR0;
 }
 else
 {
  dmxByte = UDR0;
  ucPacket[ucPacketIndex]=dmxByte;
  ucPacketIndex++;
  LED_DATA = ~LED_DATA;
 }
}

  
  
  
  




void USART_Init(void){
   if(1)
    {
      UBRR0L = BAUD_PRESCALE_9600;// Load lower 8-bits into the low byte of the UBRR register
      UBRR0H = (BAUD_PRESCALE_9600 >> 8);
    }
    else
    {
    }
   
   

  TXEN0_bit = 1;
  RXEN0_bit = 1;
  RXCIE0_bit = 1;
  //UCSR0B = ((1<<TXEN)|(1<<RXEN) | (1<<RXCIE));
}

void USART_SendByte(unsigned char u8Data){

  // Wait until last byte has been transmitted


  // Transmit data
  UDR0 = u8Data;
  while(UDRE0_bit == 0);
}


void vSendPacket(unsigned char *Data,unsigned char ucLen)
{
 unsigned char ucCount=0;
 while(ucLen)
 {
  ucLen--;
  USART_SendByte(Data[ucCount]);
  ucCount++;
 }
}

unsigned int uci;
void vTimer1_ISR() org IVT_ADDR_TIMER1_OVF
{
 //STOP_TIMER_1;
 TCNT1H = 0XF8;
 TCNT1L = 0X2F;

 uci++;
 if(uci>1000)
 {
  uci = 0;
  bSec = 1;
 }

}

void vInitTimer_1(void)
{

 //Init Timer 1
        CS10_bit = 0;
        CS11_bit = 1;
        CS12_bit = 0;

        TOIE1_bit = 1;

        TCNT1H = 0;
        TCNT1L = 0;
}

void vInitDefaultData(void)
{


}

void vClearBuffer(void)
{
 unsigned char ucCounter=0;
 for(ucCounter=0;ucCounter<20;ucCounter++)
 {
  ucPacket[ucCounter] = 0x00;
 }
}