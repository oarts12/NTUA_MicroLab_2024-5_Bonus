# NTUA_MicroLab_2024-5_Bonus

Bonus Exercise for the undergraduate course Microprocessors Laboratory (Semester 7 - Flow Y) of School of Electrical and Computer Engineering, NTUA.

A simulation of a USB connection with 2 AVR microcontrollers (using ntuAboard_G1), with one being the host and the other simulating a Keyboard.

This project was at first meant to be a USB driver for AVR, where we were supposed to communicate with an actual keyboard. We failed that task, most likely (according to our professor) because we were sending pulses with applitude of 5 Volt, instead of 3.3 Volt.

The simulation, which we then set out to implement is also unfinished. The code is correct for the higher level functions (converting ASCII to J/Ks and vice versa) but the code for transmitting information from one board to another IS NOT implemented correctly.

Required Setup of ntuAboard_G1 (for both boards):
The necessary connections for the AVR microcontroller to be functioning correctly, as described by Μιχαήλ Βάκης in his thesis.
DIP Switches for PORTs ON.
Jumpers at connectors J12 and J13 (for the correct function of TWI)
Jumpers at connectors at J19, connecting the pins of the LCD screen with those of PORT Expander.
Connection with cables of the following: PD2, PD3 and Ground of the 2 boards.


Software Used: MPLAB X IDE v6.20

Students involved:
Παναγιώτης Κατσαλίφης, Φίλιππος Αλέξανδρος Πλουμής, Οδυσσέας Τσουκνίδας

Supervisor:
Γιώργος Αλεξανδρής

