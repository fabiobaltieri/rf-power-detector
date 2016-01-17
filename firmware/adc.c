/*
 * Copyright 2016 Fabio Baltieri (fabio.baltieri@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 */

#include "board.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#include "adc.h"

void adc_init(void)
{
	ADCSRA = ((1 << ADEN)  | /* enable           */
		  (0 << ADSC)  | /* start conversion */
		  (1 << ADATE) | /* free running     */
		  (1 << ADIF)  | /* clear interrupts */
		  (0 << ADIE)  | /* interrupt enable */
		  (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0) );

	/* 16.5Mhz / 128 = 129kHz */

	/* trigger on T0 overflow */
	ADCSRB = ((1 << ADTS2) | (0 << ADTS1) | (0 << ADTS0));

	/* timer overflows every 16.5MHz / 2 / 1024 / 256 */
	TCCR0A = 0x00;
	TCCR0B = ((1 << CS02) | (0 << CS01) | (1 << CS00));
	TIMSK = (1 << TOIE0);

	/* turn on bandgap */
	ADMUX = ADC_INPUT;

	/* start the converter */
	ADCSRA |= _BV(ADSC) | _BV(ADIE);
}

ISR(TIM0_OVF_vect)
{
}

void adc_stop(void)
{
	ADCSRA = 0x00; /* ADC disable */
}

uint16_t adc_get(void)
{
	return ADCW;
}
