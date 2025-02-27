#define F_CPU 16000000UL
#include "lcd_expander.h"
#include "twi_pca.h"
#include "avr/io.h"
#include <util/delay.h>
#include <stdbool.h>
#include <stdint.h>
#include <avr/interrupt.h>

uint8_t fake_d = 0x00; //global to simulate PORTD

void write_2_nibbles(uint8_t input) {
	unsigned char temp = input & 0xF0;
	unsigned char pind  = fake_d & 0x0F;
	fake_d = pind + temp;
	PCA9555_0_write(REG_OUTPUT_0, fake_d);
	
	fake_d |= 0x08;
	PCA9555_0_write(REG_OUTPUT_0, fake_d);
	
	asm("nop");
	asm("nop");
	
	fake_d &= ~(0x08);
	PCA9555_0_write(REG_OUTPUT_0, fake_d);
	
	
	temp = input & 0x0F;
	temp = temp << 4;
	
	fake_d = pind + temp;
	PCA9555_0_write(REG_OUTPUT_0, fake_d);
	
	fake_d |= 0x08;
	PCA9555_0_write(REG_OUTPUT_0, fake_d);
	
	asm("nop");
	asm("nop");
	
	fake_d &= ~(0x08);
	PCA9555_0_write(REG_OUTPUT_0, fake_d);
	
	return;
}

void lcd_data (uint8_t input) {
	
	fake_d |= 0x04;
	PCA9555_0_write(REG_OUTPUT_0, fake_d);
	
	write_2_nibbles(input);
	_delay_ms(1);
	return;
}

void lcd_command (uint8_t input) {
	
	fake_d &= ~(0x04);
	PCA9555_0_write(REG_OUTPUT_0, fake_d);
	
	write_2_nibbles(input);
	_delay_ms(1);
	return;
}

void lcd_clear_display () {
	lcd_command(0x01);
	_delay_ms(5);
	return;
}

void lcd_change_line () {
	//I send command 0xC0 so I write in DDRAM Address the 100 0000
	lcd_command(0xC0);
	_delay_ms(5);
	return;
}

void lcd_init () {
	_delay_ms(200);
	
	fake_d = 0x30;
	PCA9555_0_write(REG_OUTPUT_0, fake_d);
	
	for (int i=0; i<3; i++) {
		
		fake_d |= 0x08;
		PCA9555_0_write(REG_OUTPUT_0, fake_d);
		
		asm("nop");
		asm("nop");
		
		fake_d &= ~(0x08);
		PCA9555_0_write(REG_OUTPUT_0, fake_d);
		
		_delay_ms(1);
	}
	
	fake_d = 0x20;
	PCA9555_0_write(REG_OUTPUT_0, fake_d);
	
	fake_d |= 0x08;
	PCA9555_0_write(REG_OUTPUT_0, fake_d);
	
	asm("nop");
	asm("nop");
	
	fake_d &= ~(0x08);
	PCA9555_0_write(REG_OUTPUT_0, fake_d);
	
	_delay_ms(1);
	
	lcd_command(0x28);
	lcd_command(0x0c);
	lcd_clear_display();
	lcd_command(0x06);
	return;

}
