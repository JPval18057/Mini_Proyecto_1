/*
 * File:   main.c
 * Author: cabal
 * Description: TEMP SLAVE
 * Created on 18 de febrero de 2021, 11:48 AM
 */

#define _XTAL_FREQ 8000000

// PIC16F887 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
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

//PROTOTIPO DE FUNCIONES
void leer_datos(void);
void semaforo(void);
void mandar_datos(void);


//VARIABLES
unsigned char temp = 0;
unsigned char new_temp = 0;
unsigned char check = 0;

//INTERRUPCIONES
void __interrupt() ISR(void) {
    //REVISAR BANDERA DEL ADC
    if (PIR1bits.ADIF){
        PIR1bits.ADIF = 0;//REINICIAMOS LA BANDERA
        temp = ADRESL;
        temp = temp>>1; //corrimiento de bits a la derecha
    }    
    if (PIR1bits.SSPIF ==1){
        PIR1bits.SSPIF = 0;
        check = SSPBUF;
        //SSPBUF = pot;
    }
}

//CONFIGURACION
void setup(void){
    //CONFIGURACION PUERTOS
    TRISA = 0b00000101; //PINES RA1 Y RA3 COMO ENTRADAS ANALÓGICAS
    ANSEL = 0b00000101; 
    ANSELH = 0;
    TRISB = 0; //PRUEBAS DEL ADC
    TRISD = 0;
    TRISC = 0;//PRUEBAS DEL ADC
    PORTB = 0; //LO APAGAMOS PARA MIENTRAS
    TRISCbits.TRISC3 = 1; //RC3 como entrada clock in
    TRISCbits.TRISC5 = 0; //RC5 como salida data out
    
    //CONFIGURACION ADC
    ADCON0 = 0b10000001;
    //CANAL AN0, ADC ENCENDIDO, FOSC/32
    ADCON1 = 0b10001000; 
    //JUSTIFICADO A LA IZQUIERDA, REFERENCIA A GND Y AL PIN RA3
    
    
    //CONFIGURACION RELOJ
    OSCCON = 0b01110101; //RELOJ INTERNO A 8MHZ
    
    //CONFIGURACIÓN DE INTERRUPCIONES
    INTCONbits.GIE = 1;//ENCENDEMOS INTERRUPCIONES GLOBALES
    INTCONbits.PEIE = 1;//INTERRUPCIONES PERIFERICAS
    PIE1bits.ADIE = 1;//ENCENDEMOS INTERRUPCIONES DEL ADC
    PIR1bits.ADIF = 0;//APAGAMOS LA BANDERA DEL ADC

    //CONFIGURACION SPI
    SSPCONbits.SSPEN = 1;
    //SLAVE SELECT
    TRISAbits.TRISA5 = 1; //entrada Slave select
    //Slave mode clock
    TRISCbits.TRISC3 = 1; //clock entra en el sistema
    TRISCbits.TRISC5 = 0; //serial data out
    TRISCbits.TRISC4 = 1; //serial data in
    SSPCON = 0b00100100;
    //SLAVE MODE SS enabled, Ide clock is 0, no overflow, no collision
    
    
    //SELECCIONAR TRANSMISION EN EL FLANCO DE BAJADA
    SSPSTAT = 0b01000000; //se transmiten los datos en flanco negativo
    PORTB = 0;
    check = SSPBUF;
    SSPCONbits.WCOL = 0;
    SSPCONbits.SSPOV = 0;
    
    return;
}

void main(void) {
    setup();
    while(1){
        leer_datos();   
        mandar_datos();
        PORTB = temp;
        semaforo();
        __delay_ms(1);
    }
    return;
}

void leer_datos(void){    
    if (ADCON0bits.GO_DONE==0){  
        asm("NOP");
        asm("NOP");
        asm("NOP");
        asm("NOP");
        asm("NOP");
        asm("NOP");
        asm("NOP");
        asm("NOP");
        asm("NOP");
        asm("NOP");
        asm("NOP");
        asm("NOP");
        asm("NOP");
        asm("NOP");
        asm("NOP");
        asm("NOP");
        asm("NOP");
        asm("NOP");
        asm("NOP");
        asm("NOP");
        asm("NOP");
        asm("NOP");
        asm("NOP");
        asm("NOP");
        asm("NOP");
        asm("NOP");
        asm("NOP");
        asm("NOP");
    //ESPERAMOS 14us antes de hacer la conversión, lo dice el datasheet
        ADCON0bits.GO_DONE = 1; //inicia la conversión
    }
    
    return;
}

void semaforo(void){
    if (temp<25){
        //ENCENDER VERDE
        PORTD = 0;
        PORTDbits.RD2 = 1;
        
    }
    if ((25<temp) && (temp<36)){
        //ENCENDER AMARILLO
        PORTD = 0;
        PORTDbits.RD1 = 1;
    }
    if (temp>36){
        //ENCENDER ROJO
        PORTD = 0;
        PORTDbits.RD0 = 1;
    }
    return;
}

void mandar_datos(void){
    //Significa que está activado el Slave
    //SSPCONbits.SSPEN = 1;
    SSPBUF = temp;
    //SSPCONbits.SSPEN = 0;
    return;
