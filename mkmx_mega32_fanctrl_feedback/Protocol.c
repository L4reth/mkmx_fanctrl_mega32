
#include "Protocol.h"

struct									//data frame structure
{
	uint8_t u8Addr;						//Address
	uint8_t u8Cmd;						//Command
	uint8_t u8PayloadLen;				//Payload length
	uint8_t u8Payload[PL_LENGHT];		//Payload
	uint8_t u8CRC;						//Cyclic redundancy check
	uint8_t u8RawData[3+PL_LENGHT];		//All data in one table 
}sFrame;

enum				//State machine
{
	eIdle=0,		//Waiting for first start byte 0xA5  
	eWaitSOF,		//Waiting for second start byte 0x5A
	eAdress,		//Checking received address
	eCmd,			//Checking received command
	ePayloadLen,	//Reading payload length
	ePayload,		//Reading received data
	eCRC,			//Checking CRC
	
}eProtocolState;

//////
//////Protocol comunictation functions
//////
void SendData(uint8_t u8Addr, uint8_t u8Cmd, uint8_t *u8Payload, uint16_t u8PayloadLen)
{
	uint8_t u8Buff[PL_LENGHT+6];
	u8Buff[0]=0x5A;
	u8Buff[1]=0xA5;
	u8Buff[2]=u8Addr;
	u8Buff[3]=u8Cmd;
	u8Buff[4]=u8PayloadLen;
	
	for(uint8_t i=0; i<u8PayloadLen; i++)
	{
		u8Buff[5+i] = u8Payload[i];
	}

	u8Buff[5+u8PayloadLen]=CalcCRC(&u8Buff[2],3+u8PayloadLen);
	
	uart_putdata(u8Buff,5+u8PayloadLen+1);
}

uint8_t CalcCRC(uint8_t *pu8Data,uint16_t u16Len)
{
	uint8_t u8CRC=0;
	for(uint8_t i=0 ; i<u16Len ; i++)
	{
		u8CRC= _crc8_ccitt_update(u8CRC,pu8Data[i]);
	}
	return u8CRC;
}

void SendText(uint8_t u8Addr, char *pcStr)
{
	uint8_t u8Buff[PL_LENGHT+6];
	u8Buff[0]=0x5A;
	u8Buff[1]=0xA5;
	u8Buff[2]=u8Addr;
	u8Buff[3]='t';
	
	uint8_t u8PayloadLen=0;
	
	while (*pcStr !=0)
	{
		u8Buff[5+u8PayloadLen]=*pcStr;
		pcStr++;
		u8PayloadLen++;
	}
	
	u8Buff[4] = u8PayloadLen;
	

	u8Buff[5+u8PayloadLen]=CalcCRC(&u8Buff[2],3+u8PayloadLen);
	
	uart_putdata(u8Buff,5+u8PayloadLen+1);
}

uint8_t ReadArdress(void) //do poprawy - przejdzie na port A - i tak nie u¿ywamy ADC
{
	//piny ustawione na wejœciwe
// 	DDRA &=~ (1<<DDA0);
// 	DDRA &=~ (1<<DDA3);
// 	DDRA &=~ (1<<DDA5);
// 	DDRA &=~ (1<<DDA7);
	
	//usatwiony internal pull up
	
	//PUEA = 0b10101001;
	
	uint8_t u8Devadrr = 0;
	
// 	if((!(PINA & (1<<PINA5))) == 1)
// 	{
// 		u8Devadrr += 1;
// 	}
// 	if((!(PINA & (1<<PINA7))) == 1)
// 	{
// 		u8Devadrr += 2;
// 	}
// 	if((!(PINA & (1<<PINA3))) == 1)
// 	{
// 		u8Devadrr += 4;
// 	}
// 	if((!(PINA & (1<<PINA0))) == 1)
// 	{
// 		u8Devadrr += 8;
// 	}
// 	// 	else
// 	// 	u8Devadrr = 0;
// 
 	return u8Devadrr;
}

void ParseData(void)
{
	uint16_t u16Word = uart_getc();	
	if((u16Word & 0xFF00) == 0)
	{
		uint8_t u8Byte = u16Word & 0x00FF;
		static uint8_t u8PayloadIdx;
		static uint8_t u8PayloadLen;
		
		switch(eProtocolState)
		{
			//Idle state
			case eIdle:
			if (u8Byte == 0x5A)
			{
				eProtocolState = eWaitSOF;
			}
			break;
			
			case eWaitSOF:
			if(u8Byte==0xA5)
			{
				eProtocolState = eAdress;
			}
			else
			{
				eProtocolState  = eIdle;
			}
			break;
			
			case eAdress:
			sFrame.u8Addr = u8Byte;
			sFrame.u8RawData[0] = u8Byte;
			eProtocolState = eCmd;
			break;
			
			//sptawdzanie komend
			case eCmd:
			sFrame.u8Cmd = u8Byte;
			sFrame.u8RawData[1] = u8Byte;
			eProtocolState = ePayloadLen;
			break;
			
			//okreslenie dlugosci ramki
			case ePayloadLen:
			sFrame.u8PayloadLen = u8Byte;
			sFrame.u8RawData[2] = u8Byte;
			
			if(u8Byte !=0)
			{
				eProtocolState = ePayload;
			}
			else
			{
				eProtocolState = eCRC;
			}
			break;
			
			case ePayload:
			sFrame.u8Payload[u8PayloadIdx] = u8Byte;
			sFrame.u8RawData[3+u8PayloadIdx] = u8Byte;
			u8PayloadIdx++;
			if(u8PayloadIdx == sFrame.u8PayloadLen)
			{
				eProtocolState = eCRC;
			}
			break;
			
			case eCRC:
			
			sFrame.u8CRC = u8Byte;
			uint8_t u8CalcCRC = CalcCRC(sFrame.u8RawData,3+sFrame.u8PayloadLen);
			if(u8CalcCRC == sFrame.u8CRC)
			{
				ParseFrame();
			}			
			eProtocolState = eIdle;
			break;			
		}
	}
}

void Protocolinit(void)
{
	uart_init( UART_BAUD_SELECT(UART_BAUD_RATE,F_CPU) );
	//DDRA |= (1<<DDA4); //direction pin set as output
}

void ParseFrame(void)
{
	if(sFrame.u8Addr == ADRR)	 //sprawdzanie warunku naszwego adrsu
	{
		switch(sFrame.u8Cmd)
		{
			//komendy steruj¹ce wiatakiem
			case PING:			//0x01
			{
				uint8_t u8DataTab[8] = "Fan_ctrl";
				SendData(0x00, 't', u8DataTab, 8);
			}
			break;
			
			case SET_SPEED:		//0x02
			{
				//u16SetRPM  = (sFrame.u8Payload[0] << 8) + sFrame.u8Payload[1];
				szybkosc = sFrame.u8Payload[0];
			}
			break;
			
			case RETURN_SPEED_HEX:	//0x03
			{
				uint8_t u8SpeedTab[2];
				// 0 - older bit, 1 - younger bit of RPM
				u8SpeedTab[0] = ((u16LastRPM & 0xFF00) >> 8);
				u8SpeedTab[1] = (u16LastRPM & 0x00FF);
				SendData(0x00, 't', u8SpeedTab, 0x02);
				// adr, cmd, payload, pl-len
			}
			break;
			
			case RETURN_SPEED_ASCII: //0x04
			{
				uint8_t u8SpeedTab[5];
				itoa(u16LastRPM,u8SpeedTab,10);				
				SendData(0x00, 't', u8SpeedTab, 5);				
			}
			break;
			
			case RETURN_OCR:	//0x05
			{
				uint8_t u8DataTab[1] = {OCR0};
				SendData(0x00, 't', u8DataTab, 0x01);		
			}
			break;
			
			default:
			break;
		}
	}
}
//////
//////Serial port debugging functions
//////
void send16bit(uint16_t value)
{
	uint16_t temp1Value = value & 0xF000;
	uint16_t temp2Value = value & 0x0F00;
	uint16_t temp3Value = value & 0x00F0;
	uint16_t temp4Value = value & 0x000F;
	
	temp1Value = temp1Value >> 12;
	temp2Value = temp2Value >> 8;
	temp3Value = temp3Value >> 4;
	temp4Value = temp4Value;
	
	if(temp1Value > 9)
	{
		uart_putc(temp1Value + 55);
	}
	else
	uart_putc(temp1Value+'0');
	
	if(temp2Value > 9)
	{
		uart_putc(temp2Value + 55);
	}
	else
	uart_putc(temp2Value+'0');
	
	if(temp3Value > 9)
	{
		uart_putc(temp3Value + 55);
	}
	else
	uart_putc(temp3Value+'0');
	
	if(temp4Value > 9)
	{
		uart_putc(temp4Value + 55);
	}
	else
	uart_putc(temp4Value+'0');
}

void sendNumber(uint32_t _nb)
{
	uint8_t check = 0;
	uint16_t d=10000;
	
	if(_nb == 0)
	{
		uint8_t tempVal;
		tempVal = _nb/d;
		uart_putc(tempVal + '0');
	}
	else
	{
		while(d>=1)
		{
			uint8_t tempVal;
			tempVal = _nb/d;
			
			if(check == 1)
			{
				uart_putc(tempVal + '0');
			}
			else
			{
				if(tempVal > 0)
				{
					uart_putc(tempVal + '0');
					check = 1;
				}
			}
			_nb = _nb-tempVal*d;
			d = d/10;
		}
	}
}

void blink(void) //debug diode on pin PA5
{
	//DDRA |= (1<<DDA5);
	//PORTA ^= (1<<PORTA5);
}

//////
//////Fan control functions
//////
void SpeedUp(uint8_t value)
{
	OCR0 -= value;
}

void SpeedDown(uint8_t value)
{
	OCR0 += value;
}

void PID(uint16_t u16ExpRPM, uint16_t u16RPM)
{
	if(u16RPM > u16ExpRPM)
	SpeedDown(5);
	if(u16RPM < u16ExpRPM)
	SpeedUp(5);
}

volatile uint16_t u16tempRPM = 0;

uint16_t SpeedRetrun(uint16_t u16RecivedPeriod, uint8_t u8Checkstop)
{
	uint16_t u16CalculatedRPM;
	//u16CalculatedRPM = 248 * (250000 /u16period); //prescaler = 8
	//u16CalculatedRPM = 31 * (250000 / u16period); //prescaler = 64
	u16CalculatedRPM = 15 * (62500 / u16RecivedPeriod); //prescaler = 256 previous 31
	if (u8Checkstop == 1)
	{
		return 0;
	}
	else
	return u16CalculatedRPM;
}