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

//Measuring and Calculating RPM
volatile uint16_t u16CounterDiff = 1;
volatile uint16_t u16TmpDiff = 0;
volatile uint16_t u16Period = 0;
//Detecting fan stop
volatile uint8_t u8stopFlag = 0;
volatile uint16_t u16ICPCounter = 0;
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
	
	u16ICPCounter++;
}

ISR(TIMER2_OVF_vect) //Detecting fan stop interrupt 
{
	if(u16ICPCounter == 0)
	{
		u8stopFlag = 1;
	}
	else
	u8stopFlag = 0;
	u16ICPCounter = 0;
}

int main(void)
{
    /* I/O ports settings */
	
	/* PWM module config */
	
	/* Input capture config */
	
	/* Stop detecting config */ //8 bitów zamiast 16!!!!!
	
	/* USART config */
	
	/* Enabling global interrupts */
	sei();
	
	/* Infinite loop */
    while (1) 
    {
		
    }
}

