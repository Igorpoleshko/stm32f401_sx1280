/*
 * sx1280.c
 *
 *  Created on: May 2, 2026
 *      Author: Igor
 */

#include "main.h"
#include <string.h>
#include "sx1280.h"

extern SPI_HandleTypeDef hspi1;

static uint8_t _txBuffer[128] = {0x00};
static uint8_t _rxBuffer[128] = {0x00};

void SX128_reset( void ){
    HAL_Delay( 20 );
    HAL_GPIO_WritePin(NRESET_GPIO_Port, NRESET_Pin, RESET);
    HAL_Delay( 50 );
    HAL_GPIO_WritePin(NRESET_GPIO_Port, NRESET_Pin, SET);
    HAL_Delay( 20 );
}

void SX1280_waitOnBusy(){
    while( HAL_GPIO_ReadPin( BUSY_GPIO_Port, BUSY_Pin ) == 1 );
}

void SX1280_writeCommand(uint8_t command, uint8_t *buffer, uint16_t size){

    SX1280_waitOnBusy();

    HAL_GPIO_WritePin(NSS_GPIO_Port, NSS_Pin, RESET);

    _txBuffer[0] = command;
    memcpy( _txBuffer + 1, buffer, size );

    HAL_SPI_Transmit( &hspi1, _txBuffer, size + 1, HAL_MAX_DELAY );

    HAL_GPIO_WritePin(NSS_GPIO_Port, NSS_Pin, SET);

    SX1280_waitOnBusy();
}

void SX1280_readCommand(uint8_t command, uint8_t *buffer, uint16_t size){
    uint16_t total = 2 + size; // [cmd] + [NOP] + [data × size]

    _txBuffer[0] = command;
    for (uint16_t i = 1; i < total; i++)
        _txBuffer[i] = 0x00;

    SX1280_waitOnBusy();

    HAL_GPIO_WritePin(NSS_GPIO_Port, NSS_Pin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(&hspi1, _txBuffer, _rxBuffer, total, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(NSS_GPIO_Port, NSS_Pin, GPIO_PIN_SET);

    SX1280_waitOnBusy();

    // rx[0] = status, rx[1] = status, rx[2..] = дані
    memcpy(buffer, _rxBuffer + 2, size);
}


void SX1280_setPacketType(uint8_t packetType){
    SX1280_writeCommand(RADIO_SET_PACKETTYPE, (uint8_t*)&packetType, 1);
}


void SX1280_setPacketParams(uint8_t payload_len){
    uint8_t buf[7];

    buf[0] = 12;			// PreambleLength
    buf[1] = 0;				// LORA_PACKET_VARIABLE_LENGTH
    buf[2] = payload_len;	// PayloadLength
    buf[3] = 0; 			// LORA_CRC_OFF
    buf[4] = 0x40;			// LORA_IQ_NORMAL
    buf[5] = 0;
    buf[6] = 0;

    SX1280_writeCommand( RADIO_SET_PACKETPARAMS, buf, 7 );

}

void SX1280_setModulationParams(){
    uint8_t buf[3];

    buf[0] = 0x60;		// LORA_SF6
    buf[1] = 0x18;		// LORA_BW_0800
    buf[3] = 0x01;		// LORA_CR_4_5

    SX1280_writeCommand( RADIO_SET_MODULATIONPARAMS, buf, 3 );

}

void SX1280_setRfFrequency(uint32_t frequency){
    uint8_t buf[3];
    uint32_t freq = 0;

    freq = ( uint32_t )( ( double )frequency / FREQ_STEP );
    buf[0] = ( uint8_t )( ( freq >> 16 ) & 0xFF );
    buf[1] = ( uint8_t )( ( freq >> 8 ) & 0xFF );
    buf[2] = ( uint8_t )( freq & 0xFF );

    SX1280_writeCommand( RADIO_SET_RFFREQUENCY, buf, 3 );
}

void SX1280_setTxParams(int8_t power, uint8_t rampTime){
    uint8_t buf[2];

    buf[0] = power + 18;
    buf[1] = ( uint8_t )rampTime;

    SX1280_writeCommand( RADIO_SET_TXPARAMS, buf, 2 );
}


void SX1280_setBufferBaseAddresses(uint8_t txBaseAddress, uint8_t rxBaseAddress){
    uint8_t buf[2];

    buf[0] = txBaseAddress;
    buf[1] = rxBaseAddress;

    SX1280_writeCommand( RADIO_SET_BUFFERBASEADDRESS, buf, 2 );
}

uint8_t SX1280_readRegister(uint16_t address){
    uint8_t data;

    SX1280_readRegisters(address, &data, 1);

    return data;
}

void SX1280_readRegisters(uint16_t address, uint8_t *buffer, uint16_t size){
    uint16_t _size = 4 + size;
    _txBuffer[0] = RADIO_READ_REGISTER;
    _txBuffer[1] = ( address & 0xFF00 ) >> 8;
    _txBuffer[2] = address & 0x00FF;
    _txBuffer[3] = 0x00;
    for( uint16_t i = 0; i < size; i++ )
    {
    	_txBuffer[4 + i] = 0x00;
    }

    SX1280_waitOnBusy();

    HAL_GPIO_WritePin(NSS_GPIO_Port, NSS_Pin, RESET);

    HAL_SPI_TransmitReceive( &hspi1, _txBuffer, _rxBuffer, _size, HAL_MAX_DELAY );

    memcpy(buffer, _rxBuffer + 4, size );

    HAL_GPIO_WritePin(NSS_GPIO_Port, NSS_Pin, SET);

    SX1280_waitOnBusy();
}

uint16_t SX1280_getFirmwareVersion(void){
    return( ( ( SX1280_readRegister( REG_LR_FIRMWARE_VERSION_MSB ) )) | ( SX1280_readRegister( REG_LR_FIRMWARE_VERSION_MSB + 1 ) )  << 8  );
}

uint16_t SX1280_getIrqStatus(void) {
    uint8_t raw[2] = {0};
    SX1280_readCommand(RADIO_GET_IRQSTATUS, raw, 2);
    return ((uint16_t)raw[0] << 8) | raw[1];
}

void SX1280_clearIrq(uint16_t mask) {
    uint8_t p[2] = { (mask >> 8) & 0xFF, mask & 0xFF };
    SX1280_writeCommand(RADIO_CLR_IRQSTATUS, p, 2); // ClearIrqStatus
}

void SX1280_setStandby(uint8_t standbyConfig){
    SX1280_writeCommand(RADIO_SET_STANDBY, ( uint8_t* )&standbyConfig, 1);
}

void SX1280_writeBuffer(uint8_t offset, uint8_t *buffer, uint8_t size){
    uint16_t _size = size + 2;
    _txBuffer[0] = RADIO_WRITE_BUFFER;
    _txBuffer[1] = offset;

    memcpy(_txBuffer + 2, buffer, size);

    SX1280_waitOnBusy( );

    HAL_GPIO_WritePin(NSS_GPIO_Port, NSS_Pin, RESET);

    HAL_SPI_Transmit( &hspi1, _txBuffer, _size + 1, HAL_MAX_DELAY );

    HAL_GPIO_WritePin(NSS_GPIO_Port, NSS_Pin, SET);

    SX1280_waitOnBusy( );
}

void SX1280_setPayload(uint8_t *buffer, uint8_t size){
    SX1280_writeBuffer(0x00, buffer, size);
}


void SX1280_setTx(uint16_t timeout_ms) {
    uint8_t p[3] = {
        0x02,                          // periodBase = 1 мс
        (timeout_ms >> 8) & 0xFF,      // periodBaseCount[15:8]
         timeout_ms        & 0xFF      // periodBaseCount[7:0]
    };
    SX1280_writeCommand(0x83, p, 3);
}

void SX1280_setRx(uint16_t timeout_ms) {
    // timeout_ms == 0    → Single (прийняти один пакет та перейти у STDBY_RC)
    // timeout_ms == 0xFFFF → Continuous (залишатись у RX)
    uint8_t p[3] = {
        0x02,                          // periodBase = 1 мс
        (timeout_ms >> 8) & 0xFF,
         timeout_ms        & 0xFF
    };
    SX1280_writeCommand(0x82, p, 3);
}

void SX1280_setDioIrqParams(uint16_t irqMask, uint16_t dio1Mask, uint16_t dio2Mask, uint16_t dio3Mask){
    uint8_t buf[8];

    buf[0] = ( uint8_t )( ( irqMask >> 8 ) & 0x00FF );
    buf[1] = ( uint8_t )( irqMask & 0x00FF );
    buf[2] = ( uint8_t )( ( dio1Mask >> 8 ) & 0x00FF );
    buf[3] = ( uint8_t )( dio1Mask & 0x00FF );
    buf[4] = ( uint8_t )( ( dio2Mask >> 8 ) & 0x00FF );
    buf[5] = ( uint8_t )( dio2Mask & 0x00FF );
    buf[6] = ( uint8_t )( ( dio3Mask >> 8 ) & 0x00FF );
    buf[7] = ( uint8_t )( dio3Mask & 0x00FF );
    SX1280_writeCommand( RADIO_SET_DIOIRQPARAMS, buf, 8 );
}

void SX1280_GetRxBufferStatus(uint8_t *payloadLength, uint8_t *rxOffset) {
    uint8_t raw[2] = {0};
    SX1280_readCommand(0x17, raw, 2);
    *payloadLength = raw[0]; // rxPayloadLength
    *rxOffset      = raw[1]; // rxStartBufferPointer
}

void SX1280_ReadBuffer(uint8_t offset, uint8_t *buffer, uint8_t size) {
    // Структура: [0x1B][offset][NOP][NOP×size] → [st][st][st][data...]
    uint16_t total = 3 + size;

    uint8_t tx[total];
    uint8_t rx[total];

    tx[0] = 0x1B;    // ReadBuffer opcode
    tx[1] = offset;  // з якого байту читати
    for (uint16_t i = 2; i < total; i++)
        tx[i] = 0x00;

    SX1280_waitOnBusy();
    HAL_GPIO_WritePin(NSS_GPIO_Port, NSS_Pin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(&hspi1, tx, rx, total, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(NSS_GPIO_Port, NSS_Pin, GPIO_PIN_SET);
    SX1280_waitOnBusy();

    // дані починаються з байту 3 (не 2!)
    memcpy(buffer, rx + 3, size);
}
