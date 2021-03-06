/* File: main.cpp
 * Contains base main function and usually all the other stuff that avr does...
 */
/* Copyright (c) 2017 Domen Ipavec (domen@ipavec.net)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

//#include <util/delay.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
// #include <avr/eeprom.h>
#include <util/delay.h>

#include <stdint.h>

#include "bitop.h"
// #include "io.h"
#include "random8.h"

// CONFIG
static const uint8_t PANELS = 2;

// array of stars - x than y
static const uint8_t stars[] PROGMEM = {
// plates from left to right (looking from front)

// plate1
	// 0, 0,
	// 6, 3,
	// 8, 6,
	// 10, 9,
	// 9, 12,
    //
	// 0, 15,
	// 3, 14,
	// 2, 8,

// plate2
	// 1, 2,
    //
	// 11, 1,
	// 8, 7,

// plate3
	// 7, 6,
	// 7, 8,
	// 2, 6,
	// 3, 8,
	// 11, 9,
    //
	// 3, 15,
	// 4, 13,
	// 2, 2,
	// 5, 0,
	// 9, 1,

// plate4
	0, 0,

	11, 1,
	9, 3,
	5, 4,
	10, 6,
	2, 8,
	7, 9,
	8, 12,
	5, 14,
	0, 15,
	11, 15,

// plate5
	// 4, 17,
	// 8, 16,
	// 9, 13,
	// 11, 9,
    //
	// 3, 0,
	// 1, 3,
	// 5, 3,
	// 3, 6,
	// 11, 20,
	// 6, 23,
};

// class definitions
class SnowFlake {
public:
	SnowFlake();
	void update();
	void reset(uint8_t new_x, int8_t new_dx, int8_t new_dy);
	bool is_dead();
	void kill();

protected:
	static const uint8_t DECIMALS = 8;
	static const int16_t DEAD = 0xDEAD;
	int16_t x;
	int16_t y;
	int8_t dx;
	int8_t dy;

	void move();
	void draw();
	void undraw();
};

// CONSTANTS
static const uint8_t WIDTH = 12;
static const uint8_t HEIGHT = PANELS * 8;
static const uint8_t SNOWFLAKES = 32;

// STATES
static const uint8_t S_TEST_INIT = 0;
static const uint8_t S_TEST = 1;
static const uint8_t S_TEST_ONE = 2;
static const uint8_t S_SNOW_INIT = 3;
static const uint8_t S_SNOW = 4;
static const uint8_t S_SNOW_END = 5;
static const uint8_t S_STARS_INIT = 6;
static const uint8_t S_STARS_START = 7;
static const uint8_t S_STARS = 8;
static const uint8_t S_STARS_END = 9;

// gamma variate a=3, b=5
static const uint8_t d_gamma[] PROGMEM = {1, 2, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7,7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18, 18, 19, 19, 19, 19, 19, 19, 19, 19, 20, 20, 20, 20, 20, 20, 20, 20, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 22, 23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 25, 25, 25, 25, 26, 26, 26, 26, 27, 27, 27, 28, 28, 28, 29, 29, 29, 30, 30, 31, 31, 32, 32, 33, 33, 34, 35, 36, 37, 38, 40, 42, 45, 52};

// VARS
static uint8_t data[PANELS*12];
static SnowFlake snow_flakes[SNOWFLAKES];
static uint8_t spi_state = 0;
static uint8_t multiplex_state = 0;
static volatile uint8_t state = S_TEST_INIT;
static volatile uint8_t snow_speed = 0;

// helper functions
static inline void spi_send() {
	SPDR = data[12 * (spi_state >> 1) + 2 * multiplex_state + (spi_state & 0x1)];
}

static inline bool delay_eight_ms(uint16_t ms) {
	bool done = TCNT1 >= ms;
	if (done) {
		TCNT1 = 0;
	}
	return done;
}

static inline bool delay_ms(uint16_t ms) {
	bool done = (TCNT1 >> 3) >= ms;
	if (done) {
		TCNT1 = 0;
	}
	return done;
}

static inline void coordinates(const uint8_t x, const uint8_t y, uint8_t &index, uint8_t &bit) {
	uint8_t panel = y >> 3;
	uint8_t y_on_panel = y & 0x7;
	if (x < 8) {
		index = y_on_panel ^ 1;
		bit = x;
	} else {
		index = 8 + (((y_on_panel >> 1) ^ 1) & 3);
		bit = (x - 8) + ((y_on_panel & 1) << 2);
	}
	index += panel * 12;
}

static inline void set(uint8_t x, uint8_t y) {
	uint8_t index, bit;
	coordinates(x, y, index, bit);
	SETBIT(data[index], bit);
}

static inline void clear(uint8_t x, uint8_t y) {
	uint8_t index, bit;
	coordinates(x, y, index, bit);
	CLEARBIT(data[index], bit);
}

// subroutines
static inline void process_spi(const uint8_t pwm) {
	if (BITSET(SPSR, SPIF)) {
		if (spi_state < 2*PANELS - 1) {
			spi_state++;
			spi_send();
		} else if (TCNT0 > 128) {
			TCNT0 = 0;

			// disable output
			SETBIT(PORTA, PA0);

			// go to new column selector
			spi_state = 0;
			PORTB = multiplex_state;
			multiplex_state++;
			if (multiplex_state >= 6) {
				multiplex_state = 0;
			}

			// latch new data
			SETBIT(PORTA, PA1);
			CLEARBIT(PORTA, PA1);

			// enable output
			CLEARBIT(PORTA, PA0);
			spi_send();
		}
	}
	if (TCNT0 >= pwm) {
		SETBIT(PORTA, PA0);
	}
}

static inline void process_snowflakes() {
	for (uint8_t i = 0; i < SNOWFLAKES; i++) {
		snow_flakes[i].update();
	}
}

static inline void add_snowflake(uint8_t &next) {
	next--;
	if (next <= 0) {
		int8_t vy = 18 + (random8_normal()>>4);
		if (vy < 2) {
			vy = 2;
		}
		int16_t vx = random8_normal();
		vx *= vy;
		vx >>= 7;
		for (uint8_t i = 0; i < SNOWFLAKES; i++) {
			if (snow_flakes[i].is_dead()) {
				snow_flakes[i].reset(random8_uniform(WIDTH), vx, vy);
				break;
			}
		}

		next = pgm_read_byte(&d_gamma[random8()]);
		if ((snow_speed >> 6) == 0) {
			next >>= 1;
		} else {
			next *= (snow_speed >> 6);
		}
		next += (snow_speed >> 2);
	}
}

// classes
SnowFlake::SnowFlake() {
	kill();
}

void SnowFlake::update() {
	if (is_dead()) {
		return;
	}
	undraw();
	move();
	draw();
}

void SnowFlake::reset(uint8_t new_x, int8_t new_dx, int8_t new_dy) {
	y = 0;
	x = int16_t(new_x) << DECIMALS;
	dx = new_dx;
	dy = new_dy;
}
bool SnowFlake::is_dead() {
	return x == DEAD;
}

void SnowFlake::kill() {
	undraw();
	x = DEAD;
}

void SnowFlake::move() {
	x += dx;
	if (x < 0) {
		kill();
		return;
	}
	if (x >= (WIDTH << DECIMALS)) {
		kill();
		return;
	}

	y += dy;
	if (y < 0) {
		kill();
		return;
	}
	if (y >= (HEIGHT << DECIMALS)) {
		kill();
		return;
	}
}

void SnowFlake::draw() {
	if (is_dead()) {
		return;
	}
	set(x >> DECIMALS, y >> DECIMALS);
}

void SnowFlake::undraw() {
	clear(x >> DECIMALS, y >> DECIMALS);
}

// ISRs
static const uint8_t UART_ADDRESS = 0x0d;
ISR(USART0_RX_vect) {
	static uint8_t ignore = 0;
	static uint8_t uart_state = 0;
	static uint8_t x;

	uint8_t uart_data = UDR0;

	if (ignore > 0) {
		ignore--;
		return;
	}

	if (uart_state == 0) {
		if (UART_ADDRESS == (uart_data & 0x0f)) {
			uart_state = 1;
		} else if (0x0c == (uart_data & 0x0f)) {
			if (state == S_SNOW) {
				state = S_SNOW_END;
			}
		} else if (0x0b == (uart_data & 0x0f)) {
			if (state == S_STARS) {
				state = S_STARS_END;
			}
		} else {
			ignore = uart_data >> 4;
		}
	} else if (uart_state == 1) {
		if (uart_data == 0) {
			uart_state = 2;
		} else if (uart_data == 1) {
			uart_state = 3;
		} else {
			uart_state = 0;
		}
	} else if (uart_state == 2) {
		snow_speed = uart_data;
		uart_state = 0;
	} else if (uart_state == 3) {
		x = uart_data;
		uart_state = 4;
	} else if (uart_state == 4) {
		for (uint8_t i = 0; i < sizeof(data); i++) {
			data[i] = 0;
		}
		set(x, uart_data);
		state = S_TEST_ONE;
		uart_state = 0;
	} else {
		uart_state = 0;
	}
}

int main() {
	// init

	// output enable always on
	SETBIT(DDRA, PA0);
	CLEARBIT(PORTA, PA0);

	// latch init
	SETBIT(DDRA, PA1);
	CLEARBIT(PORTA, PA1);

	// column control pins as output
	SETBITS(DDRB, BIT(PB0) | BIT(PB1) | BIT(PB2));

	// spi init
	// mosi, sck, ss!(this is needed for spi to work)! as outputs
	SETBITS(DDRA, BIT(PA6)|BIT(PA4)|BIT(PA7));
	// spi enable, interrupt enable, master, fastest clock
	SPCR = BIT(SPE) | BIT(MSTR);
	// double speed (fclk/2)
	SETBIT(SPSR, SPI2X);

	// uart init
	// enable receive and receive interrupt
	UCSR0B = BIT(RXEN0) | BIT(RXCIE0);
	// 8 bits
	UCSR0C = BIT(UCSZ01) | BIT(UCSZ00);
	// 19200 baud rate
	UBRR0 = 25;

	// init timer 0 for latch timing and oe pwm
	TCNT0 = 0;
	TCCR0B = BIT(CS01);

	// init timer 1 for delay_ms function
	TCNT1 = 0;
	// clk/1024
	TCCR1B = BIT(CS12) | BIT(CS10);

	// enable interrupts
	sei();

	// start spi auto sending
	spi_send();

	uint8_t next = 1;
	uint8_t i = 0;
	uint8_t pwm = 128;

	uint16_t change_count = 0;

	for (;;) {
		switch (state) {
			case S_TEST_INIT:
				for (i = 0; i < sizeof(data); i++) {
					data[i] = 0xff;
				}
			case S_TEST_ONE:
				TCNT1 = 0;
				pwm = 0;
				i = 0;
				state = S_TEST;
				break;
			case S_TEST:
				if (i == 0) {
					if (delay_ms(5)) {
						pwm++;
						if (pwm >= 128) {
							i = 1;
						}
					}
				} else {
					if (delay_ms(5)) {
						pwm--;
						if (pwm == 0) {
							state = S_SNOW_INIT;
						}
					}
				}
				break;
			case S_SNOW_INIT:
				for (i = 0; i < sizeof(data); i++) {
					data[i] = 0;
				}
				for (i = 0; i < SNOWFLAKES; i++) {
					snow_flakes[i].kill();
				}
				next = 1;
				TCNT1 = 0;
				change_count = 0;
				i = 0;
				pwm = 128;
				state = S_SNOW;
				break;
			case S_SNOW:
				// delay enough to process each snowflake in one frame
				if (delay_eight_ms(8*16/SNOWFLAKES)) {
					snow_flakes[i].update();

					i++;
					if (i >= SNOWFLAKES) { // this is once per frame
						i = 0;

						add_snowflake(next);
					}
				}
				break;
			case S_SNOW_END:
				// delay enough to process each snowflake in one frame
				if (delay_eight_ms(8*16/SNOWFLAKES)) {
					snow_flakes[i].update();

					i++;
					if (i >= SNOWFLAKES) { // this is once per frame
						i = 0;

						change_count++;
						if (change_count >= 673) {
							state = S_STARS_INIT;
						}
					}
				}
				break;
			case S_STARS_INIT:
				for (uint8_t i = 0; i < sizeof(data); i++) {
					data[i] = 0;
				}
				for (uint8_t i = 0; i < (sizeof(stars)>>1); i++) {
					set(
						pgm_read_byte(&stars[2*i]),
						pgm_read_byte(&stars[2*i+1])
					);
				}
				TCNT1 = 0;
				pwm = 0;
				state = S_STARS_START;
				break;
			case S_STARS_START:
				if (pwm < 128) {
					if (delay_ms(20)) {
						pwm++;
					}
				} else {
					TCNT1 = 0;
					state = S_STARS;
				}
				break;
			case S_STARS:
				break;
			case S_STARS_END:
				if (pwm > 0) {
					if (delay_ms(20)) {
						pwm--;
						if (pwm == 0) {
							for (uint8_t i = 0; i < sizeof(data); i++) {
								data[i] = 0;
							}
						}
					}
				} else {
					if (delay_ms(500)) {
						TCNT1 = 0;
						state = S_SNOW_INIT;
					}
				}
				break;
			default:
				state = S_SNOW_INIT;
				break;
		}

		process_spi(pwm);
	}
}



