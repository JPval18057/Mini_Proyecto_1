# Mini Project 1

This project was intended for the development of a network of SPI communication using the PIC16F887 microcontroller.

The project consisted on the configuration of an SPI communication network between four microcontrollers. The master microcontroller will obtain the values of
slaves and display them on an LCD screen. Additionally, the values obtained were displayed on the virtual console as well as on the LCD screen.

It was broken in four parts:

Slave 1:
•	Design of a routine that reads the voltage of the potentiometer.
Slave 2:
•	Management of two switches that increase or decrease the counter value.
Slave 3:
•	Read the temperature using a LM35 sensor and turn on a series of LEDs depending on the temperature value.
o	Green: T < 25°C
o	Yellow: 25°C < T < 36°C
o	Red: T > 36°C
Master:
•	Implement the communication protocol between the master and 3 slaves.
•	Show the slaves data in an LCD screen.
•	Send the values shown in the screen to the computer via serial port.

