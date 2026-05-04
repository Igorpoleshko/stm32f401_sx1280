/*
 * sx1280.h
 *
 *  Created on: May 2, 2026
 *      Author: Igor
 */

#ifndef SRC_SX1280_SX1280_H_
#define SRC_SX1280_SX1280_H_

#define XTAL_FREQ                                   52000000
//#define FREQ_STEP                                   ( ( double )( XTAL_FREQ / pow( 2.0, 18.0 ) ) )
#define FREQ_STEP                                   ((double)(XTAL_FREQ / 262144))


// opcode
#define RADIO_GET_IRQSTATUS         (0x15)
#define RADIO_CLR_IRQSTATUS         (0x97)

#define RADIO_WRITE_BUFFER    		(0x1A)
#define RADIO_READ_BUFFER           (0x1B)
#define RADIO_READ_REGISTER         (0x19)
#define RADIO_SET_STANDBY           (0x80)
#define RADIO_SET_RFFREQUENCY       (0x86)
#define RADIO_SET_PACKETTYPE		(0x8A)
#define RADIO_SET_MODULATIONPARAMS  (0x8B)
#define RADIO_SET_PACKETPARAMS  	(0x8C)
#define RADIO_SET_DIOIRQPARAMS      (0x8D)
#define RADIO_SET_TXPARAMS          (0x8E)

#define RADIO_SET_BUFFERBASEADDRESS (0x8F)


#define PACKET_TYPE_LORA		(0x01)

#define TX_OUTPUT_POWER         (13)
#define RADIO_RAMP_02_US        (0x00)

// регістри
#define REG_LR_FIRMWARE_VERSION_MSB                 0x0153

// IRQ bits
#define  IRQ_RADIO_NONE             (0x0000)
#define IRQ_TX_DONE 				(0x0001)
#define IRQ_RX_DONE                 (0x0002)
#define IRQ_RADIO_ALL            	(0xFFFF)


void SX128_reset( void );
void SX1280_waitOnBusy( void );

void SX1280_writeCommand(uint8_t command, uint8_t *buffer, uint16_t size );
void SX1280_readCommand(uint8_t command, uint8_t *buffer, uint16_t size);

void SX1280_setPacketType(uint8_t packetType);
void SX1280_setPacketParams(uint8_t payload_len);
void SX1280_setModulationParams();
void SX1280_setRfFrequency(uint32_t frequency);
void SX1280_setTxParams(int8_t power, uint8_t rampTime);
void SX1280_setBufferBaseAddresses(uint8_t txBaseAddress, uint8_t rxBaseAddress);

uint8_t SX1280_readRegister(uint16_t address);
void SX1280_readRegisters(uint16_t address, uint8_t *buffer, uint16_t size);
uint16_t SX1280_getFirmwareVersion(void);

void SX1280_setStandby(uint8_t standbyConfig);
void SX1280_setPayload(uint8_t *buffer, uint8_t size);
void SX1280_writeBuffer(uint8_t offset, uint8_t *buffer, uint8_t size);

void SX1280_setTx(uint16_t timeout_ms);
void SX1280_setRx(uint16_t timeout_ms);

void SX1280_clearIrq(uint16_t mask);
uint16_t SX1280_getIrqStatus(void);
void SX1280_setDioIrqParams(uint16_t irqMask, uint16_t dio1Mask, uint16_t dio2Mask, uint16_t dio3Mask);

void SX1280_ReadBuffer(uint8_t offset, uint8_t *buffer, uint8_t size);
void SX1280_GetRxBufferStatus(uint8_t *payloadLength, uint8_t *rxOffset);

#endif /* SRC_SX1280_SX1280_H_ */
