#define F_CPU 16000000UL
#include "twi_pca.h"
#include "keypad.h"
#include "lcd_expander.h"
#include "avr/io.h"
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdbool.h>

#define J 2
#define K 4
#define SE0 0
#define SE1 6

#define MAX_STREAM_SIZE 96
#define bit_time 10

uint8_t jks[MAX_STREAM_SIZE][2];
uint8_t data[MAX_STREAM_SIZE - 8];


int EOP = 0;
uint8_t val;
bool first_array = true;

//Interrupt Handler for INT0 that is caused when we go from idle to K (== start of packet)
//Its job was supposed to be to read the IN packet and then send the Data Packet
//but the following code is just a version of it, used for the failed debugging

//After a translation from the Board's Keyboard to J/K happens, the next one populates the other jks array
//alternating between 0 and 1
//Therefore when handler happens we need to send the J/Ks from the array that is not being written at the moment
//in order to ensure consistency
ISR(INT0_vect){
    DDRD = 0x00;
    while(1){
        _delay_us(10);
        val = PIND&(0x06); //PD1 & PD2   
        if (val == SE0)
            break;
    }
    _delay_us(bit_time);
    _delay_us(bit_time);
    _delay_us(bit_time);
    DDRD = 0xFF;
    for(int i=0; i<96; i++) {
        PORTD = jks[i][!first_array];
        _delay_us(bit_time);
       
    }
    PORTD = 0;
    _delay_us(bit_time);
    PORTD = 0;
    _delay_us(bit_time);
    PORTD = J;
    _delay_us(bit_time);
}


//Translating data (meaning the 1 and 0s meant to be transmitted) to J/Ks
void data_to_jk()
{

    int j=0;
    jks[j++][first_array]= K;
    jks[j++][first_array]= J;
    jks[j++][first_array]= K;
    jks[j++][first_array]= J;
    jks[j++][first_array]= K;
    jks[j++][first_array]= J;
    jks[j++][first_array]= K;
    jks[j++][first_array]= K;
    char prev = K;
    for (int i = 0 ; i < 72; i++ ){
        if (data[i] == 0){
            if (prev==K){
                prev = J;
                jks[8+i][first_array] = J;
            }
            else {
                prev = K;
                jks[8+i][first_array] = K;
            }
        }
        else {
            if (prev==K){
                prev = K;
                jks[8+i][first_array] = K;
            }
            else {
                prev = J;
                jks[8+i][first_array] = J;
            }
        }
    }
}

//Code for implementing ascii to data
//Starts here
//----------------------------------------------------- 
uint8_t sparse_byte[8];

int isletter(uint8_t ascii) {
    return (ascii > 64 && ascii < 91);
}

int isnumber(uint8_t ascii) {
    return (ascii > 48 && ascii < 58);
}

void make_sparse_byte(uint8_t byte) {
    int div = 128;
    for(int i=0; i<8; ++i) {
        sparse_byte[7-i] = byte / div;
        byte = byte%div;
        div = div/2;
    }
}

void data_init() {
    make_sparse_byte(0xC3);     //Data0 PID
    for(int i=0; i<8; ++i) {
        data[i] = sparse_byte[i];
    }
    for(int i=0; i<80; ++i) {
        data[8+i] = 0;
    }
}

void ascii_to_data(uint8_t ascii) {
    uint8_t shift_key = PINB;
    if(shift_key == 0x01){                  // Check for SHIFT key
        make_sparse_byte(0x02);         // Sparse byte for shift code
        for(int i=0; i<8; ++i) {
            data[8+i] = sparse_byte[i]; // Store in 1st data byte
        }
    }
    else {
        for(int i=0; i<8; ++i) {        // Else 1st byte = 0
            data[8+i] = 0;
        }
    }
    
    if( isletter(ascii) ) {             // Check for letter
        make_sparse_byte(ascii - 'A' + 4);
    }
    else if(isnumber(ascii)) {          // Check for number
        make_sparse_byte(ascii - '1' + 30);
    }
    else {
        if(ascii == '0') make_sparse_byte(0x27);    // 0 is calculated separately
        if(ascii == '*') make_sparse_byte(0x08);    // * = E
        if(ascii == '#') make_sparse_byte(0x09);    // # = F
        if(ascii == 0x00) make_sparse_byte(0x00);
    
    }
    for(int i=0; i<8; ++i) {
        data[24+i] = sparse_byte[i];
    }

    return;
}

//----------------------------------------------------- 
//Ends here

// This is the code that will run on the AVR that simulates the keyboard
int main() {
    //This enables INT0 on rising edge
    EICRA = (1<<ISC01)|(1<<ISC00); 
    EIMSK = (1<<INT0); 
    sei();
    twi_init();
	PCA9555_0_write(REG_CONFIGURATION_1, 0xF0); //PORT1[7:3] = input, [3:0] = output
    data_init();
    DDRB =0x00;         //B as input in order to simulate the SHIFT (PB0 == shift for the purposes of this simulation)
    DDRD = 0x00;        //The default for this AVR is to have the signals as input waiting a transition from J (idle) to K
    uint8_t ascii;
    while(1) {
        ascii = keypad_to_ascii();
        ascii_to_data(ascii);
        data_to_jk();
        first_array = !(first_array);        
    }
}
