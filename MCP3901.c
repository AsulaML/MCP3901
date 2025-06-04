#include "MCP3901.h"
#include "SPI.h"
#include "Adc.h"

// SPI write helper
static void MCP3901_WriteRegister(uint8_t reg, uint8_t data) {
    uint8_t word = (reg << 1) | WRITE;
    AD_CS = 0;
    readwriteSPI1(word);
    readwriteSPI1(data);
    AD_CS = 1;
}

// SPI read helper
static uint8_t MCP3901_ReadRegister(uint8_t reg) {
    uint8_t word = (reg << 1) | READ;
    AD_CS = 0;
    readwriteSPI1(word);
    uint8_t val = readwriteSPI1(0x00);
    AD_CS = 1;
    return val;
}

void MCP3901_Init() {
    MCP3901_WriteRegister(CONFIG2, 0b11000000); // Reset
    MCP3901_WriteRegister(STATUS,  0b11000000); // Loop over all registers

    AD_CS = 0;
    readwriteSPI1((PHASE << 1) | WRITE);
    readwriteSPI1(INITPHASE);
    readwriteSPI1(INITGAIN);
    readwriteSPI1(INITSTAT);
    readwriteSPI1(INITCFG1);
    readwriteSPI1(INITCFG2);
    AD_CS = 1;

    RPINR1bits.INT2R = 17;
    TRISFbits.TRISF5 = 1; // INT2
}

void MCP3901_AdressLoop() {
    MCP3901_WriteRegister(STATUS, 0b11000000);
}

int32_t MCP3901_Read() {
    uint8_t word = (DATA_CH0 << 1) | READ;
    uint8_t SPIBUFFR[3];

    AD_CS = 0;
    readwriteSPI1(word);
    SPIBUFFR[0] = readwriteSPI1(0x00);
    SPIBUFFR[1] = readwriteSPI1(0x00);
    SPIBUFFR[2] = readwriteSPI1(0x00);
    AD_CS = 1;

    return ((int32_t)SPIBUFFR[0] << 8) | SPIBUFFR[1]; // 16 MSBs
}

uint8_t MCP3901_Read_STATUS() {
    return MCP3901_ReadRegister(STATUS);
}