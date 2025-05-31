#ifndef MCP3901_h
#define MCP3901_h

    #include "main.h"

    #define ADOSC PORTFbits.RF4
    #define ADOSC_Dir TRISFbits.TRISF4 //0 = output, 1 = input
    
    // Register addresses
    #define DATA_CH0 0x00 
    #define DATA_CH1 0x03
    #define MOD      0x06
    #define PHASE    0x07
    #define GAIN     0x08
    #define STATUS   0x09
    #define CONFIG1  0x0A
    #define CONFIG2  0x0B
    
//    // Default values
//    #define INITPHASE 0x00
//    #define INITGAIN  0x00
//    #define INITSTAT  0xF4  // Clear status -entitre loop et DR ON
//    #define INITCFG1  0x34  // 24 bits mode
//    #define INITCFG2  0x0D  // External Clock


    // Default values
    #define INITPHASE 0x00
    #define INITGAIN  0x00
    #define INITSTAT  0xF4  // Clear status -entitre loop et DR ON
    #define INITCFG1  0xF4  // 24 bits mode
    #define INITCFG2  0x81  // External Clock


    // Read Write bit
    #define READ  0x01
    #define WRITE 0x00

    void MCP3901_Init(void);

    void Multi_Coil();
    
    void init_OC2();
    int32_t MCP3901_Read();//(uint8_t *buffer);
    uint8_t MCP3901_Read_STATUS(void);
    
#endif
