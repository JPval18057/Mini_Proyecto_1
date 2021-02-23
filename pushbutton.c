/*
 * File:   main.c
 * Author: JUAN PABLO VALENZUELA
 * Description: PUSHBUTTON SLAVE
 * Created on 18 de febrero de 2021, 11:23 AM
 */



// PIC16F887 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FOSC = EXTRC_NOCLKOUT// Oscillator Selection bits (RCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, RC on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>



//Variables
unsigned char count = 0;
//PORTB DEBOUNCING
unsigned char boton1 = 0;
unsigned char boton2 = 0;


//Interrupt subroutine
void __interrupt() ISR(void) {
    //CHECK THE PORTB FLAG
    if (INTCONbits.RBIF==1){
        INTCONbits.RBIF = 0;//TURN OFF THE FLAG
        if(PORTBbits.RB0==1){
            boton1 = 1;
        }
        if (PORTBbits.RB2==1){
            boton2 = 1;
        }
        
    }
    
}



void setup(void){
    //CONFIGURAR LOS PUERTOS
    TRISD = 0;
    ANSEL = 0;
    ANSELH = 0;
    TRISB = 0b00001111; //RB0 Y RB1 COMO ENTRADAS
    PORTB = 0;
    //CONFIGURAR LAS INTERRUPCIONES
    ei();
    INTCONbits.PEIE = 1; //ACTIVAR INTERRUPCIONES PERIFERICAS
    INTCONbits.RBIE = 1; //ACTIVAR INTERRUPCIONES EN PUERTO B
    IOCBbits.IOCB0 = 1; //ACTIVAR INTERRUPCIONES EN RB0 Y RB1
    IOCBbits.IOCB2 = 1;
    INTCONbits.RBIF = 0; //APAGAR LA BANDERA
    
    //CONFIGURAR LA COMUNICACIÓN
    
    
    
    return;
}

void main(void) {
    setup();
    while(1){
        if (boton1==1){ //this activates when the button is pressed and has just released
            if (PORTBbits.RB0==0){
                count++;
                boton1 = 0;
            }
        }
        if (boton2==1){ //when the button is pressed and releases, that instant this code will execute
            if (PORTBbits.RB2==0){ 
                count = count -1;
                boton2 = 0;
            }
        }
        //ACTUALIZAR EL CONTADOR
        PORTD = count;
    }
    return;
}
