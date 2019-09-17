/*
 * mkmx_mega32_fanctrl_feedback.cpp
 *
 * Created: 2019-05-31 18:45:38
 * Author : Rafal Stolarczyk
 */

/* Defined in Toolchain -> Compiler -< symbols */ 
//#define F_CPU 8000000UL

#include <util/delay.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#include "Protocol.h"

//Measuring and Calculating RPM
volatile uint16_t u16CounterDiff = 1; //difference between interruptions
volatile uint16_t u16TmpDiff = 0;
volatile uint16_t u16Period = 0;
//Detecting fan stop
volatile uint8_t u8stopFlag = 0;
volatile uint8_t u8ICPCounter = 0;
volatile uint8_t u8IntCounter = 0;
//Returning RPM value
volatile uint16_t u16LastRPM = 0;
volatile uint16_t u16SetRPM = 0;
//Device address in net
volatile uint8_t u8DeviceAdres = 0;
//Seting Fan speed
volatile uint8_t u8SpeedAdjustFlag = 0;
volatile uint8_t u8Speed = 0;

ISR(TIMER1_CAPT_vect) //measuring speed interrupt
{							//done on every timer_1 overflow
	u16CounterDiff = (TCNT1 - u16TmpDiff);
	u16TmpDiff = TCNT1;
	
	TCCR1B ^= (1<<ICES1);
	u8SpeedAdjustFlag = 1; //Allow to change Fan speed
	
	if(u16CounterDiff > 700)
	{
		u16Period = u16CounterDiff;
	}	
	u8ICPCounter++;
}

ISR(TIMER2_OVF_vect) //Detecting fan stop interrupt 
{
	u8IntCounter++;
	if(u8IntCounter == 20) //additional prescaler 
	{
		if(u8ICPCounter == 0)
		{
			u8stopFlag = 1;
		}
		else
		u8stopFlag = 0;
		u8ICPCounter = 0;
		
		//uart_putc(0x55); //debuging
		u8IntCounter = 0;
	}
}

// ISR(TIMER2_OVF_vect)
// {
// 	
// }

int main(void)
{
    /* I/O ports settings */
		   //76543210 pin numbers 1-output
	DDRA = 0b00000000; //Dedicated for address pins
// 	DDRB = 0b
// 	DDRC = 0b
	DDRD |= (1<<DDD6); //input capture pin
	DDRD |= (1<<DDD5); //pwm output pin
	
	/* PWM module config */ //fast pwm, presc. =8
	TCCR0 |= (1<<WGM00)|(1<<WGM01)|(1<<COM01)|	(1<<CS01);
	OCR0 = 0; //full speed
	
	/* Input capture config */
	TCCR1A = 0x00;
	TCCR1B |= (1<<ICNC1)|(1<<ICES1)|(1<<CS12);
	TIMSK |= (1<<TICIE1);
	
	/* Stop detecting config */ //8 bitów zamiast 16!!!!!
	TCCR2 |= (1<<CS02)|(1<<CS01)|(1<<CS00); //free running, presc = 1024
	TIMSK |= (1<<TOV2);										//overflow 30 per sec.
	
	/* USART config */
	Protocolinit();
	
	/* Enabling global interrupts */
	sei();
	
	/* Infinite loop */
    while (1) 
    {
		u16LastRPM = SpeedRetrun(u16Period, u8stopFlag);
		if(u8SpeedAdjustFlag = 1)
		{
			PID(u8Speed,u16LastRPM);
			u8SpeedAdjustFlag = 0; //wait for next interrupt
		}
		u16LastRPM = 1956;
		ParseData();		
    }
}

