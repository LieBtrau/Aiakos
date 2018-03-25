#include "loradevice.h"

namespace
{
RHReliableDatagram* pmgrLoRa;
RHReliableDatagram* pmgrSer;
byte peerAddress;
}
bool writeDataLoRa(byte* data, byte length);
bool readDataLoRa(byte* data, byte& length);
bool readDataSer(byte *data, byte& length);
bool writeDataSer(byte* data, byte length);

LoRaDevice::LoRaDevice(byte ownAddress, RH_RF95 *prhLora, RH_Serial *prhSerial, byte cableDetectPin):
    rhLoRa(prhLora),
    rhSerial(prhSerial),
    ecdh(&RNG, writeDataSer, readDataSer),
    mgrLoRa(*rhLoRa, ownAddress),
    mgrSer(*rhSerial, ownAddress),
    k(&RNG, writeDataLoRa, readDataLoRa),
    CABLE_DETECT_PIN(cableDetectPin)
{
    pmgrLoRa=&mgrLoRa;
    pmgrSer=&mgrSer;
}

bool LoRaDevice::setup()
{
    rhSerial->serial().begin(2400);
    if ((!mgrLoRa.init()))
    {
        debug_println("LoRa init failed");
        return false;
    }
    rhLoRa->setTxPower(5);//minimum TX power for SX1276
    if ((!mgrSer.init()))
    {
        debug_println("Serial init failed");
        return false;
    }
    byte buf[10];
    return (getSerialNumber(buf, Configuration::IDLENGTH)
            && k.init(buf,Configuration::IDLENGTH)
            && ecdh.init(buf, Configuration::IDLENGTH));
}

void LoRaDevice::getInitialPinStates()
{
    pinMode(CABLE_DETECT_PIN, INPUT_PULLUP);
    cableDetect.attach(CABLE_DETECT_PIN);
    cableDetect.interval(100); // interval in ms
}

void LoRaDevice::setPeerAddress(byte address)
{
    peerAddress=address;
}

//http://www.stm32duino.com/viewtopic.php?t=707
word LoRaDevice::readVcc()
{
#ifdef ARDUINO_STM_NUCLEO_F103RB || defined(ARDUINO_GENERIC_STM32F103C)                           //Blue Pill
    adc_reg_map *regs = ADC1->regs;
    regs->CR2 |= ADC_CR2_TSVREFE;    // enable VREFINT and temp sensor
    regs->SMPR1 =  ADC_SMPR1_SMP17;  // sample rate for VREFINT ADC channel
    return 1200 * 4096 / adc_read(ADC1, 17);  // ADC sample to millivolts
#elif ARDUINO_SAM_DUE
    return 0;
#endif
}

bool writeDataSer(byte* data, byte length)
{
    debug_print("Sending serial data...");
    debug_printArray(data, length);
    return pmgrSer->sendtoWait(data, length, peerAddress);
}

bool writeDataLoRa(byte* data, byte length)
{
    debug_print("Sending LoRa data: ");debug_printArray(data, length);
    return pmgrLoRa->sendtoWait(data, length, peerAddress);
}

bool readDataSer(byte* data, byte& length)
{
    byte from;
    if (!pmgrSer->available())
    {
        return false;
    }
    if(!pmgrSer->recvfromAck(data, &length, &from))
    {
        return false;
    }
    if(from != peerAddress)
    {
        debug_print("Sender doesn't match");
        return false;
    }
    debug_print("Received data: ");debug_printArray(data, length);
    return true;
}

bool readDataLoRa(byte *data, byte& length)
{
    byte from;
    if (!pmgrLoRa->available())
    {
        return false;
    }
    if(!pmgrLoRa->recvfromAck(data, &length, &from))
    {
        return false;
    }
    if(from != peerAddress)
    {
        debug_print("Sender doesn't match");
        return false;
    }
    return true;
}

