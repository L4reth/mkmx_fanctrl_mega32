/*
 * mkmx_mega32_fanctrl_feedback.cpp
 *
 * Created: 2019-05-31 18:45:38
 * Author : Rafal
 */ 
#define F_CPU 8000000UL

#include <util/delay.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#include "Protocol.h"

//Measuring and Calculating RPM
volatile uint16_t u16CounterDiff = 1;
volatile uint16_t u16TmpDiff = 0;
volatile uint16_t u16Period = 0;
//Detecting fan stop
volatile uint8_t u8stopFlag = 0;
volatile uint8_t u8ICPCounter = 0;
//Returning RPM value
volatile uint16_t u16LastRPM = 0;
volatile uint16_t u16SetRPM = 0;
//Device adress in net
volatile uint8_t u8DeviceAdres = 0;

volatile uint8_t szybkosc = 0;

ISR(TIMER1_CAPT_vect) //measuring speed interrupt
{
	u16CounterDiff = (TCNT1 - u16TmpDiff);
	u16TmpDiff = TCNT1;
	
	TCCR1B ^= (1<<ICES1);
	
	if(u16CounterDiff > 700)
	{
		u16Period = u16CounterDiff;
	}	
	u8ICPCounter++;
}

ISR(TIMER2_OVF_vect) //Detecting fan stop interrupt 
{
	if(u8ICPCounter == 0)
	{
		u8stopFlag = 1;
	}
	else
	u8stopFlag = 0;
	u8ICPCounter = 0;
}

// ISR(TIMER2_OVF_vect)
// {
// 	
// }

int main(void)
{
    /* I/O ports settings */
		   //76543210 pin numbers 1-output
	DDRA = 0b00000000; //Dedicated for adress pins
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
	TCCR2 |= (1<<CS02)|(1<<CS01)|(1<<CS00); //free runing, presc = 1024
											//overflow 30/sec
	
	/* USART config */
	Protocolinit();
	
	/* Enabling global interrupts */
	sei();
	
	/* Infinite loop */
    while (1) 
    {
		
    }
}

