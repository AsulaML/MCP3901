#include "MCP3901.h"
#include "SPI.h"
#include "Adc.h"

/*
 * The MCP3901 has a SPI interface with the following settings:
 * MSB first
 * CPOL = 0, CPHA = 0 (MODE = 0)
 * - SCK low when idle
 * - Latch on rising edge
 * 
   -------------------------------------
  | A6 | A5 | A4 | A3 | A2 | A1 | R/W |
  -------------------------------------
  |         |                   |     |
  | Device  | Register Address  | R/W | 
 */


// The part stays within the same loop
// until CS returns high
void MCP3901_Init() 
{
    uint8_t word = CONFIG2;
    word <<= 1;
    word |= WRITE;   
            
    // RESET 
    AD_CS = 0;
    readwriteSPI1(word);    // Addr 
    readwriteSPI1(0b11000000); // Reset mode
    AD_CS = 1;
    
    // STATUS
    word = STATUS;
    word <<= 1;
    word |= WRITE;
    
    AD_CS = 0;
    readwriteSPI1(word);        // Addr     
    readwriteSPI1(0b11000000);  // Address counter loops on entire register map
    AD_CS = 1;

    
    word = PHASE;
    word <<= 1;                 // laisse la place pour le bit de Read or Write
    word |= WRITE;              // on force (pas oblig)
    
    AD_CS = 0;
    readwriteSPI1(word);    
    readwriteSPI1(INITPHASE);        
    readwriteSPI1(INITGAIN); 
    readwriteSPI1(INITSTAT);
    readwriteSPI1(INITCFG1);
    readwriteSPI1(INITCFG2);
    AD_CS = 1;
    
    RPINR1bits.INT2R = 17;
    TRISFbits.TRISF5 = 1;   // INT2
}

void MCP3901_AdressLoop()
{
    uint8_t word = STATUS;
    word <<= 1;
    word |= WRITE;
    
    AD_CS = 0;
    readwriteSPI1(word);        // Addr     
    readwriteSPI1(0b11000000);  // Address counter loops on entire register map
    AD_CS = 1;
    
}

// The part stays within the same loop
// until CS returns high
int32_t MCP3901_Read()//(uint8_t *buffer)
{
    uint8_t SPIBUFFR[] = {0,0,0};	// SPI buffer for Receiving
    //int16_t ADC_DATA = 0;
    
    uint8_t word = DATA_CH0;
    //uint8_t status = 0;
    
    // time out !!!
    //do{
    //    status = MCP3901_Read_STATUS();
    //}while ((status & 0x01) == 0x01);
    
    word <<= 1;
    word |= READ;
    
    //TRISFbits.TRISF2 = 0;   // MISO - SPI - SDI
    //TRISEbits.TRISE8 = 0;   // MISO

    AD_CS = 0;
    //TRISFbits.TRISF2 = 1;   // MOSI - SPI - SDI
    //TRISEbits.TRISE8 = 1;   // MISO

    readwriteSPI1(word);    // Addr 
    
    SPIBUFFR[0] = readwriteSPI1(0x00);
    //buffer ++;
    
    SPIBUFFR[1] = readwriteSPI1(0x00);
    //buffer ++;
    
    SPIBUFFR[2] = readwriteSPI1(0x00);
    
    AD_CS = 1;
    
    // Use only 16 MSB bits
    return (int32_t)(SPIBUFFR[0] * 256) + (int32_t)SPIBUFFR[1]; 
}    



// The part stays within the same loop
// until CS returns high
uint8_t MCP3901_Read_STATUS()
{
    uint8_t word = STATUS;
    uint8_t buffer;
    
    word <<= 1;
    word |= READ;
    
    //TRISFbits.TRISF2 = 0;   // MISO - SPI - SDI
    //TRISEbits.TRISE8 = 0;   // MISO

    AD_CS = 0;
    //TRISFbits.TRISF2 = 0;   // MOSI - SPI - SDI
    //TRISEbits.TRISE8 = 1;   // MISO

    readwriteSPI1(word);    // Addr 
    
    buffer = readwriteSPI1(0x00);
    
    AD_CS = 1;   
    
    return buffer;
}    


// Génération de l'horloge ADOSC pour l'adc 
// OC2RS = 4 => 3.2 MHz
// OC2RS = 3 => 4 MHz
void init_OC2()
{
    TRISFbits.TRISF4 = 1;       // output
    // OUTPUT COMPARE CONFIG
    // PWM Period = [(PRy) + 1] ? TCY ? (Timer Prescale Value)
    OC2CON1 = 0;
    OC2CON2 = 0;
    
    RPOR5bits.RP10R = 19;        // IO remap Output compare
    
    // 4 meg
    OC2R = 1;                 // valeur de toggle on match  
    //OC1RS = (Fcy/Fpwm)-1 = (16,000,000/10000)-1 = 1599
    OC2RS = 3;              // OC1TMR continues to increment until it matches OC1RS
    
    // 1 meg
    //OC2R = 8;
    //OC2RS = 15;              
    
    OC2CON2bits.OCINV = 0;              // OCx output is not inverted
    OC2CON2bits.SYNCSEL = 0b11111 ;     // synchronized by itself
    //OC1CON2bits.SYNCSEL = 0b01111;     
    OC2CON2bits.OCTRIS = 0;             // Output compare module drives the OCx pin
            
    OC2CON1bits.OCTSEL = 0b111;         //  Peripheral clock (FCY)
    
    //OC1CON1bits.OCTSEL = 0b011;       // Timer 5
    OC2CON1bits.OCM = 0b110;            // Edge-aligned PWM mode on OC
    
    __delay_ms(50);
    TRISFbits.TRISF4 = 0;       // output
    __delay_ms(50);
}


void Multi_Coil()
{
    //OC1R = 67;
    while(1)
    {
    float feedback = 0;
    float quantum_adc = 3.3/1024;
    float quantum_dac = 3.3/1600;
    float error = 0;
    float consigne = 0;
    float data = 0;
    int16_t datad = 0;
    uint16_t dataf = 0;
    uint16_t adc_val = 0;
    
    OC1R = 97;
    HIV = 1; // Shunt de la res 100k
    LOW = 0; // No pull down
    ELEMTERH = 0;
    TRISBbits.TRISB13 = 1; // confirmation en input
    
        adc_val = ADC_Read(13);    
        data = 0;    
        feedback = quantum_adc * adc_val;  //ADC_Read(13)*quantum_adc;  // Valeur de tension de tapmert
        consigne = 0.2;                   // Valeur désirée
        error = consigne - feedback;      // Ecart entre la consigne et la mesure
        data = (float)(error/quantum_dac);


        datad = (int16_t)(data);
        if(error > 0) OC1R += datad;
        else
        {
            dataf = 0xFFFF - (uint16_t)datad;
            OC1R -= dataf;
        }
        Nop();
    }
}



