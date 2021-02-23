/*
 * File:   main.c
 * Author: JUAN PABLO
 * DESCRIPTION: ESTE ES EL CÓDIGO QUE CONTROLA EL MASTER
 * CON AYUDA DE LA LIBRERIA DE LCD DE ELECTROSOME
 * Created on 21 de febrero de 2021, 08:37 PM
 */

#define _XTAL_FREQ 8000000

#define RS RD2
#define EN RD3
#define D4 RD4
#define D5 RD5
#define D6 RD6
#define D7 RD7

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

//INCLUIR LIBRERIAS
#include <xc.h>
#include <stdio.h>
#include <pic16f887.h> //se necesita para usar sprintf y formatear strings
#include "lcd.h"

//PROTOTIPOS DE FUNCION
void leer_esclavos(void);

//Variables
unsigned char recibo = 0;
char s[20];
char slave = 0;
unsigned char temp = 0;
unsigned char push = 0;
unsigned char push2 = 0;

//INTERRUPCIONES
void __interrupt() ISR(void) {
    //Revisar si se recibió algún dato
    if (PIR1bits.SSPIF==1){        
        if (slave==3){
            push2 = SSPBUF; //leemos el dato rápidamente
            PIR1bits.SSPIF = 0; //APAGAMOS LA BANDERA
            //slave = 0;
        } 
        if (slave==2){
            push = SSPBUF; //leemos el dato rápidamente
            PIR1bits.SSPIF = 0; //APAGAMOS LA BANDERA
            //slave=0;
        }        
        if (slave==1){
            temp = SSPBUF; //leemos el dato rápidamente
            PIR1bits.SSPIF = 0; //APAGAMOS LA BANDERA
            //slave++;
        }
        if (slave==0){
            recibo = SSPBUF; //leemos el dato rápidamente
            PIR1bits.SSPIF = 0; //APAGAMOS LA BANDERA
            //slave++;
        }
    }
    
}

//CONFIGURACION GENERAL
void setup(){
    //CONFIGURACION DE I/O
    TRISD = 0; //PUERTO D COMO SALIDA
    TRISD = 0; //PUERTO C COMO SALIDA
    TRISB = 0; //PUERTO B COMO SALIDA PARA PRUEBAS
    TRISA = 255; //PUERTO A COMO ENTRADA
    //CONFIGURACION DE RELOJ
    OSCCON = 0b01110101; //RELOJ INTERNO A 8MHZ
    //CONFIGURACION DE SPI
    /*PINES
     RA5 -> SLAVE SELECT
     SERIAL CLOCK -> RC3 
     SERIAL DATA OUT (SDO) -> RC5 MASTER OUT SLAVE IN/ SACA DATOS DEL ESCLAVO
     SERIAL DATA IN (SDI)-> RC4 SLAVE IN MASTER OUT/ ENTRAN DATOS DEL MAESTRO
     
    */
    TRISCbits.TRISC4 = 1; //DATA IN
    TRISCbits.TRISC0 = 0; //SS botones
    TRISCbits.TRISC1 = 0; //SS adc
    TRISCbits.TRISC2 = 0; //SS temp
    PORTCbits.RC0 = 1;
    PORTCbits.RC1 = 1;
    PORTCbits.RC2 = 1; //apagamos todos los slaves por default
    
    TRISCbits.TRISC3 = 0; //MASTER MODE SCK COMO SALIDA
    TRISCbits.TRISC5 = 0; //SERIAL DATA OUT como entrada porque este es el maestro
    TRISAbits.TRISA5 = 1; //SLAVE SELECT como entrada
    //SELECCIONAR TRANSMISION EN EL FLANCO DE BAJADA
    SSPSTAT = 0b01000000; //se transmiten los datos en flanco negativo
    /*
     * Vamos a usar el puerto C para comunicación
     * RC5 ES SERIAL DATA OUT
     * RC4 ES SERIAL DATA IN CONTROLADO POR EL MÓDULO
     * SE PUEDE DESHABILITAR EL SERIAL DATA OUT DEL MAESTRO (RC5)     
     * VAMOS A USAR RC0, RC1 Y RC2 COMO SLAVE SELECTS (SALIDAS EN EL MAESTRO)
     * El maestro seleccionar el esclavo 1 y cuando el esclavo 1 detecte el ss=0
     * va a mandar sus datos, el maestro va a detectar un dato final específico
     * y va a desactivar el esclavo y pasar al siguiente
     * el siguiente va a hacer lo mismo y luego cuando el maestro termine
     * de revisar todos los esclavos va a mostrar los datos en la pantalla y
     * al final de todo eso los va a mandar por eusart
     * RC0 = BOTONES
     * RC1 = ADC
     * RC2 = TEMP
     */
    SSPCON = 0b00100000;
    //MASTER MODE Fosc/4, Idle clock is 0, SPI ENABLED, No overflow and no collision
    
    //CONFIGURACION DE INTERRUPCIONES
    ei();//global interrupt enable
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1; //PERIPHERAL INTERRUPT ENABLE
    PIE1bits.SSPIE = 1; //activar interrupcion de SPI
    PIR1bits.SSPIF = 0;//apagar la bandera
    //CONFIGURACION DE PANTALLA
    Lcd_Init();
    
    //Pruebas de codigo
    //PORTCbits.RC1 = 0; //SLAVE SELECT TEMP (ESTÁ NEGADO)
    return;
}


//CODIGO PRINCIPAL
void main(void) {
    //esta parte solo se ejecuta 1 vez
    setup();
    //esta parte se ejecuta continuamente
    while(1){
        //lectura de esclavos
        leer_esclavos();
        //mostrar resultados en pantalla
        Lcd_Clear(); //limpia la pantalla
        Lcd_Set_Cursor(1,1); //poner el cursor en fila 1 caracter 1
        Lcd_Write_String("POT TEM CONT");
        
        Lcd_Set_Cursor(2,1);
        sprintf(s,"%u",recibo);
        Lcd_Write_String(s);//escribimos el dato que se recibio
        
        Lcd_Set_Cursor(2,5);
        sprintf(s,"%u",temp);
        Lcd_Write_String(s);//escribimos el dato que se recibio
        
        Lcd_Set_Cursor(2,9);
        sprintf(s,"%u",push2);
        Lcd_Write_String(s);//escribimos el dato que se recibio
        
        //Mandar eusart
        
    }
    return;
}

void leer_esclavos(void){
    PORTCbits.RC0 = 1; //SLAVE SELECT TEMP (ESTÁ NEGADO)
    PORTCbits.RC1 = 1; 
    PORTCbits.RC2 = 1; 
    if (slave==3){ //ADC
        PORTCbits.RC0 = 1; //SLAVE SELECT TEMP (ESTÁ NEGADO)
        PORTCbits.RC1 = 1; 
        PORTCbits.RC2 = 1; 
        SSPBUF = 0x61; //mandamos el dato para que se intercambien los datos
        recibo = SSPBUF; //leemos el dato rápidamente
        PORTB = recibo;
        __delay_ms(100); //no tocarlo
        
        PORTCbits.RC0 = 1; //SLAVE SELECT TEMP (ESTÁ NEGADO)
        PORTCbits.RC1 = 1; 
        PORTCbits.RC2 = 1; 
    }
    if (slave==2){
        //temperatura
        PORTCbits.RC0 = 1; //SLAVE SELECT TEMP (ESTÁ NEGADO)
        PORTCbits.RC1 = 1; 
        PORTCbits.RC2 = 0; 
        SSPBUF = 0x61; //mandamos el dato para que se intercambien los datos
        temp = SSPBUF; //leemos el dato rápidamente
        //PORTB = recibo;
        __delay_ms(100); //no tocarlo
        
        PORTCbits.RC0 = 1; //SLAVE SELECT TEMP (ESTÁ NEGADO)
        PORTCbits.RC1 = 1; 
        PORTCbits.RC2 = 1; 
        slave=0;
    }
    
    if (slave==1){
        //pushbuttons
        PORTCbits.RC0 = 1; //SLAVE SELECT TEMP (ESTÁ NEGADO)
        PORTCbits.RC1 = 0; 
        PORTCbits.RC2 = 1; 
        SSPBUF = 0x61; //mandamos el dato para que se intercambien los datos
        push = SSPBUF; //leemos el dato rápidamente
        //PORTB = recibo;
        __delay_ms(100); //no tocarlo
        
        PORTCbits.RC0 = 1; //SLAVE SELECT TEMP (ESTÁ NEGADO)
        PORTCbits.RC1 = 1; 
        PORTCbits.RC2 = 1; 
        slave++;
    }
    if (slave==0){
        //pushbuttons
        PORTCbits.RC0 = 0; //SLAVE SELECT TEMP (ESTÁ NEGADO)
        PORTCbits.RC1 = 1; 
        PORTCbits.RC2 = 1; 
        SSPBUF = 0x61; //mandamos el dato para que se intercambien los datos
        push2 = SSPBUF; //leemos el dato rápidamente
        //PORTB = recibo;
        __delay_ms(100); //no tocarlo
        push2 = SSPBUF; //leemos el dato rápidamente
        PORTCbits.RC0 = 1; //SLAVE SELECT TEMP (ESTÁ NEGADO)
        PORTCbits.RC1 = 1; 
        PORTCbits.RC2 = 1; 
        slave++;
    }
    return;
}
