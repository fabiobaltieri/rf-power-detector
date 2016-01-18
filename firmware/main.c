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
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "usbdrv.h"

#include "adc.h"
#include "requests.h"

static uint16_t return_value;
static volatile uint16_t raw_val;

static void reset_cpu(void)
{
	wdt_disable();

	wdt_enable(WDTO_15MS);

	for (;;);
}

#define div_round(a, b) (((a) + ((b)/2)) / (b))
ISR(ADC_vect)
{
	raw_val = adc_get();
}

static void led_b_step(void)
{
	if (raw_val < LED_B_TH)
		led_b_on();
	else
		led_b_off();
}

usbMsgLen_t usbFunctionSetup(uint8_t data[8])
{
	struct usbRequest *rq = (void *)data;

	switch (rq->bRequest) {
	case CUSTOM_RQ_GET_VALUE:
		return_value = raw_val;
		usbMsgPtr = (uint8_t *)&return_value;
		return sizeof(return_value);
	case CUSTOM_RQ_GET_PAD_DB:
		return_value = PAD_DB;
		usbMsgPtr = (uint8_t *)&return_value;
		return sizeof(return_value);
	case CUSTOM_RQ_RESET:
		reset_cpu();
		return 0;
	}

	return 0;
}

static void hello(void)
{
	uint8_t i;
	for (i = 0; i < 8; i++) {
		led_a_toggle();
		_delay_ms(33);
	}
}

int __attribute__((noreturn)) main(void)
{
	uint8_t i;

	led_init();
	led_a_off();

	adc_init();

	wdt_enable(WDTO_1S);

	hello();

	led_a_on();

	usbInit();
	usbDeviceDisconnect();

	i = 0;
	while (--i) {
		wdt_reset();
		_delay_ms(1);
	}

	usbDeviceConnect();

	sei();
	for (;;) {
		wdt_reset();
		usbPoll();
		led_b_step();
	}
}
