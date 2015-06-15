/*  MRF89XA driver
    Copyright (C) 2014  Christoph Tack

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef RH_MRF89XA_h
#define RH_MRF89XA_h

#include "RHGenericSPI.h"
#include "RHNRFSPIDriver.h"

// This is the maximum number of bytes that can be carried by the MRF89XA.
// We use some for headers, keeping fewer for RadioHead messages
#define RH_MRF89XA_MAX_PAYLOAD_LEN 64

// The length of the headers we add.
// The headers are inside the MRF89XA payload
#define RH_MRF89XA_HEADER_LEN 4

// This is the maximum RadioHead user message length that can be supported by this library. Limited by
// the supported message lengths in the MRF89XA
#define RH_MRF89XA_MAX_MESSAGE_LEN (RH_MRF89XA_MAX_PAYLOAD_LEN-RH_MRF89XA_HEADER_LEN)


// Register names
const byte RH_MRF89XA_REGISTER_MASK=0x1f;
const byte GCONREG=0x00;
const byte DMODREG=0x01;
const byte FDEVREG=0x02;
const byte BRSREG=0x03;
const byte FLTHREG=0x04;
const byte FIFOCREG=0x05;
const byte R1CREG=0x06;
const byte P1CREG=0x07;
const byte S1CREG=0x08;
const byte R2CREG=0x09;
const byte P2CREG=0x0A;
const byte S2CREG=0x0B;
const byte PACREG=0x0C;
const byte FTXRXIREG=0x0D;
const byte FTPRIREG=0x0E;
const byte RSTHIREG=0x0F;
const byte FILCREG=0x10;
const byte PFCREG=0x11;
const byte SYNCREG=0x12;
const byte RSVREG=0x13;
const byte RSTSREG=0x14;
const byte OOKCREG=0x15;
const byte SYNCV31REG=0x16;
const byte SYNCV23REG=0x17;
const byte SYNCV15REG=0x18;
const byte SYNCV07REG=0x19;
const byte TXCONREG=0x1A;
const byte CLKOREG=0x1B;
const byte PLOADREG=0x1C;
const byte NADDSREG=0x1D;
const byte PKTCREG=0x1E;
const byte FCRCREG=0x1F;

// SPI Command names
const byte RH_MRF89XA_COMMAND_R_REGISTER=0x20;
const byte RH_MRF89XA_COMMAND_W_REGISTER=0x00;

//GCONREG
const byte RH_MRF89XA_MASK_CMOD=0xE0;
const byte RH_MRF89XA_CMOD_SLEEP=0x00;
const byte RH_MRF89XA_CMOD_STANDBY=0x20;
const byte RH_MRF89XA_CMOD_FS=0x40;
const byte RH_MRF89XA_CMOD_RX=0x60;
const byte RH_MRF89XA_CMOD_TX=0x80;

class RH_MRF89XA : public RHNRFSPIDriver
{
public:

    //    /// \brief Defines convenient values for setting data rates in setRF()
    //    typedef enum
    //    {
    //    DataRate1Mbps = 0,   ///< 1 Mbps
    //    DataRate2Mbps,       ///< 2 Mbps
    //    DataRate250kbps      ///< 250 kbps
    //    } DataRate;

    //    /// \brief Convenient values for setting transmitter power in setRF()
    //    /// These are designed to agree with the values for RF_PWR in RH_MRF89XA_REG_06_RF_SETUP
    //    /// To be passed to setRF();
    //    typedef enum
    //    {
    //    TransmitPowerm18dBm = 0,        ///< On nRF24, -18 dBm
    //    TransmitPowerm12dBm,            ///< On nRF24, -12 dBm
    //    TransmitPowerm6dBm,             ///< On nRF24, -6 dBm
    //    TransmitPower0dBm,              ///< On nRF24, 0 dBm
    //    // Sigh, different power levels for the same bit patterns on RFM73:
    //    // On RFM73P-S, there is a Tx power amp, so expect higher power levels, up to 20dBm. Alas
    //    // there is no clear documentation on the power for different settings :-(
    //    RFM73TransmitPowerm10dBm = 0,   ///< On RFM73, -10 dBm
    //    RFM73TransmitPowerm5dBm,        ///< On RFM73, -5 dBm
    //    RFM73TransmitPowerm0dBm,        ///< On RFM73, 0 dBm
    //    RFM73TransmitPower5dBm          ///< On RFM73, 5 dBm. 20dBm on RFM73P-S2 ?

    //    } TransmitPower;

    //    /// Constructor. You can have multiple instances, but each instance must have its own
    //    /// chip enable and slave select pin.
    //    /// After constructing, you must call init() to initialise the interface
    //    /// and the radio module
    //    /// \param[in] chipEnablePin the Arduino pin to use to enable the chip for transmit/receive
    //    /// \param[in] slaveSelectPin the Arduino pin number of the output to use to select the NRF24 before
    //    /// accessing it. Defaults to the normal SS pin for your Arduino (D10 for Diecimila, Uno etc, D53 for Mega,
    //    /// D10 for Maple)
    //    /// \param[in] spi Pointer to the SPI interface object to use.
    //    ///                Defaults to the standard Arduino hardware SPI interface
    RH_MRF89XA(uint8_t slaveSelectPinData, uint8_t slaveSelectPinCommand, uint8_t irq0pin, uint8_t irq1pin, RHGenericSPI& spi = hardware_spi);

    //    /// Initialises this instance and the radio module connected to it.
    //    /// The following steps are taken:g
    //    /// - Set the chip enable and chip select pins to output LOW, HIGH respectively.
    //    /// - Initialise the SPI output pins
    //    /// - Initialise the SPI interface library to 8MHz (Hint, if you want to lower
    //    /// the SPI frequency (perhaps where you have other SPI shields, low voltages etc),
    //    /// call SPI.setClockDivider() after init()).
    //    /// -Flush the receiver and transmitter buffers
    //    /// - Set the radio to receive with powerUpRx();
    //    /// \return  true if everything was successful
    bool        init();

    //    /// Reads a single register from the NRF24
    //    /// \param[in] reg Register number, one of NRF24_REG_*
    //    /// \return The value of the register
    uint8_t        spiReadRegister(uint8_t reg);

    //    /// Writes a single byte to the NRF24, and at the ame time reads the current STATUS register
    //    /// \param[in] reg Register number, one of NRF24_REG_*
    //    /// \param[in] val The value to write
    //    /// \return the current STATUS (read while the command is sent)
    uint8_t        spiWriteRegister(uint8_t reg, uint8_t val);

    //    /// Reads a number of consecutive registers from the NRF24 using burst read mode
    //    /// \param[in] reg Register number of the first register, one of NRF24_REG_*
    //    /// \param[in] dest Array to write the register values to. Must be at least len bytes
    //    /// \param[in] len Number of bytes to read
    //    /// \return the current STATUS (read while the command is sent)
    uint8_t           spiBurstReadRegister(uint8_t reg, uint8_t* dest, uint8_t len);

    //    /// Write a number of consecutive registers using burst write mode
    //    /// \param[in] reg Register number of the first register, one of NRF24_REG_*
    //    /// \param[in] src Array of new register values to write. Must be at least len bytes
    //    /// \param[in] len Number of bytes to write
    //    /// \return the current STATUS (read while the command is sent)
    uint8_t        spiBurstWriteRegister(uint8_t reg, uint8_t* src, uint8_t len);

    //    /// Reads and returns the device status register NRF24_REG_02_DEVICE_STATUS
    //    /// \return The value of the device status register
    //    uint8_t        statusRead();

    //    /// Sets the transmit and receive channel number.
    //    /// The frequency used is (2400 + channel) MHz
    //    /// \return true on success
    //    bool setChannel(uint8_t channel);
    //    /// Sets the chip configuration that will be used to set
    //    /// the NRF24 NRF24_REG_00_CONFIG register when in Idle mode. This allows you to change some
    //    /// chip configuration for compatibility with libraries other than this one.
    //    /// You should not normally need to call this.
    //    /// Defaults to NRF24_EN_CRC| RH_MRF89XA_CRCO, which is the standard configuration for this library
    //    /// (2 byte CRC enabled).
    //    /// \param[in] mode The chip configuration to be used whe in Idle mode.
    //    /// \return true on success
    //    bool setOpMode(uint8_t mode);

    //    /// Sets the Network address.
    //    /// Only nodes with the same network address can communicate with each other. You
    //    /// can set different network addresses in different sets of nodes to isolate them from each other.
    //    /// Internally, this sets the nRF24 TX_ADDR and RX_ADDR_P0 to be the given network address.
    //    /// The default network address is 0xE7E7E7E7E7
    //    /// \param[in] address The new network address. Must match the network address of any receiving node(s).
    //    /// \param[in] len Number of bytes of address to set (3 to 5).
    //    /// \return true on success, false if len is not in the range 3-5 inclusive.
    //    bool setNetworkAddress(uint8_t* address, uint8_t len);

    //    /// Sets the data rate and transmitter power to use. Note that the nRF24 and the RFM73 have different
    //    /// available power levels, and for convenience, 2 different sets of values are available in the
    //    /// RH_MRF89XA::TransmitPower enum. The ones with the RFM73 only have meaning on the RFM73 and compatible
    //    /// devces. The others are for the nRF24.
    //    /// \param [in] data_rate The data rate to use for all packets transmitted and received. One of RH_MRF89XA::DataRate.
    //    /// \param [in] power Transmitter power. One of RH_MRF89XA::TransmitPower.
    //    /// \return true on success
    //    bool setRF(DataRate data_rate, TransmitPower power);

    //    /// Sets the radio into Power Down mode.
    //    /// If successful, the radio will stay in Power Down mode until woken by
    //    /// changing mode it idle, transmit or receive (eg by calling send(), recv(), available() etc)
    //    /// Caution: there is a time penalty as the radio takes a finite time to wake from sleep mode.
    //    /// \return true if sleep mode was successfully entered.
    virtual bool    sleep();

    //    /// Sets the radio in power down mode, with the configuration set to the
    //    /// last value from setOpMode().
    //    /// Sets chip enable to LOW.
    //    /// \return true on success
    void setModeIdle();

    //    /// Sets the radio in RX mode.
    //    /// Sets chip enable to HIGH to enable the chip in RX mode.
    //    /// \return true on success
    void setModeRx();

    //    /// Sets the radio in TX mode.
    //    /// Pulses the chip enable LOW then HIGH to enable the chip in TX mode.
    //    /// \return true on success
    void setModeTx();

    //    /// Sends data to the address set by setTransmitAddress()
    //    /// Sets the radio to TX mode
    //    /// \param [in] data Data bytes to send.
    //    /// \param [in] len Number of data bytes to set in teh TX buffer. The actual size of the
    //    /// transmitted data payload is set by setPayloadSize
    //    /// \return true on success (which does not necessarily mean the receiver got the message, only that the message was
    //    /// successfully transmitted).
    bool send(const uint8_t* data, uint8_t len);

    //    /// Blocks until the current message (if any)
    //    /// has been transmitted
    //    /// \return true on success, false if the chip is not in transmit mode or other transmit failure
        virtual bool waitPacketSent();

    //    /// Indicates if the chip is in transmit mode and
    //    /// there is a packet currently being transmitted
    //    /// \return true if the chip is in transmit mode and there is a transmission in progress
    //    bool isSending();

    //    /// Prints the value of all chip registers
    //    /// for debugging purposes
    //    /// \return true on success
    //    bool printRegisters();

    //    /// Checks whether a received message is available.
    //    /// This can be called multiple times in a timeout loop
    //    /// \return true if a complete, valid message has been received and is able to be retrieved by
    //    /// recv()
    bool        available();

    //    /// Turns the receiver on if it not already on.
    //    /// If there is a valid message available, copy it to buf and return true
    //    /// else return false.
    //    /// If a message is copied, *len is set to the length (Caution, 0 length messages are permitted).
    //    /// You should be sure to call this function frequently enough to not miss any messages
    //    /// It is recommended that you call it in your main loop.
    //    /// \param[in] buf Location to copy the received message
    //    /// \param[in,out] len Pointer to available space in buf. Set to the actual number of octets copied.
    //    /// \return true if a valid message was copied to buf
    bool        recv(uint8_t* buf, uint8_t* len);

    //    /// The maximum message length supported by this driver
    //    /// \return The maximum message length supported by this driver
    uint8_t maxMessageLength();


    //protected:
    //    /// Flush the TX FIFOs
    //    /// \return the value of the device status register
    //    uint8_t flushTx();

    //    /// Flush the RX FIFOs
    //    /// \return the value of the device status register
    //    uint8_t flushRx();

    //    /// Examine the revceive buffer to determine whether the message is for this node
        void validateRxBuf();

    //    /// Clear our local receive buffer
        void clearRxBuf();

private:
    void writeFifo(uint8_t data);
    bool setOperatingMode(uint8_t om);
    uint8_t readFifo();
    uint8_t _irq0pin;
    uint8_t _irq1pin;
    bool _rxBufValid;
    uint8_t _opMode;

    //    /// This idle mode chip configuration
    //    uint8_t             _configuration;

    //    /// the number of the chip enable pin
    //    uint8_t             _chipEnablePin;

    //    /// Number of octets in the buffer
    uint8_t             _bufLen;

    //    /// The receiver/transmitter buffer
    uint8_t             _buf[RH_MRF89XA_MAX_PAYLOAD_LEN];

    //    /// True when there is a valid message in the buffer
};
#endif
