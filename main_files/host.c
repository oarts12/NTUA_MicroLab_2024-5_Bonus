#define F_CPU 16000000UL
#include "twi_pca.h"
#include "keypad.h"
#include "lcd_expander.h"
#include "avr/io.h"
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include <string.h>

#define J 0x02
#define K 0x04

#define bit_time 100

#define KEY_MOD_LCTRL  0x01
#define KEY_MOD_LSHIFT 0x02
#define KEY_MOD_LALT   0x04
#define KEY_MOD_LMETA  0x08
#define KEY_MOD_RCTRL  0x10
#define KEY_MOD_RSHIFT 0x20
#define KEY_MOD_RALT   0x40
#define KEY_MOD_RMETA  0x80


#define MAX_STREAM_SIZE 96 // 8 for SYNC, 8 for PID,64 for 8 bytes of Data, and then 16 for CRC

int shift_numbers[10] = {
    '!', '@','#','$','%','^','&','*','(',')'
};

int symbols[7] = {'\n', '\x1B', '\x8', '\t', ' ', '-', '='};

int shift_symbols[7] = {'\n','\x1B', '\x8', '\t', ' ', '_', '+'};

int previous_data = 0;

uint8_t jks[MAX_STREAM_SIZE]; //
uint8_t data[MAX_STREAM_SIZE - 8];

//Just for debugging
//---------------------------------------------------------
//DATA
//char str[] ="11000011 00000000 00000000 00100000 00000000 00000000 00000000 00000000 00000000";
char str[] ="000000110000000000000000000000000000000000000000000000000000000000000000";
//NAK
//char str[] ="010110100000000000000000000001100000000000000000000000000000000000000000";
//USELESS
//char str[] ="001111110000000000000000000001100000000000000000000000000000000000000000";


void data_to_jk()
{

    int j=0;
    jks[j++]= K;
    jks[j++]= J;
    jks[j++]= K;
    jks[j++]= J;
    jks[j++]= K;
    jks[j++]= J;
    jks[j++]= K;
    jks[j++]= K;
    char prev = K;
    for (int i = 0 ; i < 72; i++ ){
        if (str[i] == '0'){
            if (prev==K){
                prev = J;
                jks[8+i] = J;
            }
            else {
                prev = K;
                jks[8+i] = K;
            }
        }
        else {
            if (prev==K){
                prev = K;
                jks[8+i] = K;
            }
            else {
                prev = J;
                jks[8+i] = J;
            }
        }
    }
}

//---------------------------------------------------------
//End of just for debugging

uint8_t in_token[35] = {K, J, K, J, K, J, K, K, K, J, K, K, J, J, J, K, K, J, K, J, K, J, K, K, J, K, J, K, K, K, J, J, 0, 0, J};
uint8_t a,b,c, d, e, f;


//This is also a function used just for debugging
void receive_bit() {
    lcd_clear_display();
    DDRD = 0xFF;
    PORTD = K;
    
    //_delay_us(bit_time /2);
    
    DDRD = 0x00;
    _delay_ms(1);
    a = PIND & 0x06;
    //_delay_us(bit_time /2);
    b = PIND & 0x06;
    lcd_data('a'+ a);
    lcd_data('a'+ b);
    DDRD = 0xFF;
    PORTD = J;
    _delay_ms(100);
    return;
}

//This function's job was supposed to be to send the IN packet and read send the Data Packet
//but the following code is just a version of it, used for the failed debugging
void in_transaction() {
    lcd_clear_display();
    DDRD = 0xFF;
    /*
    for(int i=0; i<35; i++) {
        PORTD = in_token[i];
        _delay_us(bit_time);
    }
     */
    PORTD = K;
    
    DDRD = 0x00;
    //_delay_us(bit_time);
    /*
    for(int i=0; i<96; i++) {
        jks[i] = (PIND & 0x06);
        _delay_us(bit_time);
    }
    _delay_us(bit_time);
    _delay_us(bit_time);
    _delay_us(bit_time);
     */
    _delay_us(bit_time/2);
    a= PIND & 0x06;
    _delay_us(bit_time);
    b= PIND & 0x06;
    _delay_us(bit_time);
    c= PIND & 0x06;
    _delay_us(bit_time);
    d= PIND & 0x06;
    _delay_us(bit_time);
    e= PIND & 0x06;
    _delay_us(bit_time);
    f= PIND & 0x06;
    _delay_us(bit_time);
    
    lcd_data ('a' + a);
    lcd_data ('a' + b);
    lcd_data ('a' + c);
    lcd_data ('a' + d);
    lcd_data ('a' + e);
    lcd_data ('a' + f);
    DDRD = 0xFF;
    PORTD = J;
    return;
}

//Function that takes the J/Ks and then populates the data array with 0/1s
//Returns -3 when there is an error in SYNC pattern
int jk_to_data()
{	
	int i=0;
	for (i = 0; i < 6; i+=2){
		if (!(jks[i] == K && jks[i+1]==J))
			return -3;          //-3 means wrong SYNC
	}
	if (!(jks[6] == K && jks[7] == K))
		return -3;

	for ( i = 8 ; i < 80; i++ ) {
		data[i-8] = (jks[i] == jks[i-1]);	
	}
	return 0;
}

//Function that is called when we have received a Data Packet
//Its job is to find the ASCII that is represented by the incoming data packet
//Returns -1 if the keyboard (or AVR simulating the keyboard) has sent ERROR
//Returns -2 if we haven't implemented the recognisition of the ASCII sent
int data_packet()
{
	uint8_t value[8];
	for ( int j = 0 ; j < 8 ; j++){
        value[j] = 0;
		for ( int i = 0; i < 8; i++){
			value[j] |= (data[(j+1)*8+i]<<i);
		}
	}
	bool shift = false;
	if ((value[0]&0x0F) == KEY_MOD_LSHIFT || (value[0]&0xF0) == KEY_MOD_RSHIFT)
		shift = true;  
	
	if (value[2] == 0)
		return 0;
	if (value[2] == 1)
		return -1;

	if (value[2]>= 4 && value[2]<=0x1d){
		if (shift) return ('A' + value[2] - 4);
		else return ('a' + value[2] - 4);
	}

	if (value[2]>= 0x1e && value[2]<=0x27){
		if (shift) return (shift_numbers[value[2]-0x1e]);
		else return '0' + value[2] - 0x1e;
	}

	if (value[2] >= 0x28 && value[2] <= 0x2e){
		if (shift) return shift_symbols[value[2] - 0x28];
		else return symbols[value[2] - 0x28];
	}

	return -2; 	// not implemented

}

//Function that takes 0/1s as transmitted and finds which ASCII that is represented by the incoming packet
//Either by calling data_packet(), or by recognising that the incoming packet is NAK
//and therefore returning the previous ASCII sent
//Returns -4 when not recognising the PID 
int data_to_ascii()
{
	if (data[0] == 1 && data[1] == 1) {// assuming complement is the same
		return data_packet();
    }
	else if(data[0] == 0 && data[1] == 1){ // if NAK return previous key
        return previous_data;

    }
	else{
        return -4;  //-4 means wrong PID
    }
}

//Functions that is called when we want to read from keyboard (either a real one, or the AVR simulating it)
int read_keyboard()
{
	in_transaction();

	if (jk_to_data() == -3)
		return -3;
	
	int res = data_to_ascii();
	if (res <0)
		return res;
	previous_data = res;
	return res;
}

//This is a debugging version of the main function and not the one which the final project was supposed to have
int main() {
    twi_init();
    PCA9555_0_write(REG_CONFIGURATION_0, 0x00); //PORT0 as output (just to send stuff to LCD screen)
    lcd_init();
    lcd_clear_display();
    char error[] = "ERROR";
    char nothing1[] = "NOTHING IS";
    char nothing2[] = "PRESSED";
    DDRD = 0xFF;        //The default for this AVR is to have the signals as output in order to send INs
    DDRB = 0x00;
    PORTD = J;
    int dummy, letter, prev_letter=-9;
    
    _delay_ms(1000);
    while(1) {
        //OLD VERSION WITH PINB
        /*
        dummy = PINB & 0x01;
        if(dummy == 0) {
            letter = read_keyboard();
            data_to_jk(); //just for debugging
            if(letter == -1) {
                lcd_clear_display();
                for(int i=0; i<strlen(error); i++) {
                    lcd_data(error[i]);
                }
            }
            else if(letter ==0) {
                lcd_clear_display();
                for(int i=0; i<strlen(nothing1); i++) {
                    lcd_data(nothing1[i]);
                }
                lcd_change_line();
                for(int i=0; i<strlen(nothing2); i++) {
                    lcd_data(nothing2[i]);
                }
            }
            else {
                lcd_clear_display();
                lcd_data(letter);
            }
        }
         */
        //NEW VERSION WITH DELAYS
        //data_to_jk(); //just for debugging
        _delay_ms(2000);
        //letter = read_keyboard();
        letter = 80;
        //in_transaction();
        receive_bit();
        if(letter == prev_letter) { //This is to avoid reprinting things which causes flickering
            continue;
        }
        prev_letter = letter;
        if(letter == -1) {
            lcd_clear_display();
            for(int i=0; i<strlen(error); i++) {
                lcd_data(error[i]);
            }
        }
        else if(letter==0) {
            lcd_clear_display();
            for(int i=0; i<strlen(nothing1); i++) {
                lcd_data(nothing1[i]);
            }
            lcd_change_line();
            for(int i=0; i<strlen(nothing2); i++) {
                lcd_data(nothing2[i]);
            }
        }
        else if(letter == -2) {
            lcd_clear_display();
            lcd_data('-');
            lcd_data('2');
        }
        else if(letter == -3) {
            lcd_clear_display();
            lcd_data('-');
            lcd_data('3');
        }
        else if(letter == -4) {
            lcd_clear_display();
            lcd_data('-');
            lcd_data('4');
        }
        else if(letter == 80) {
            continue;
        }
        else {
            lcd_clear_display();
            lcd_data(letter);
        }
    }
}
