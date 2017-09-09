#include "garagecontroller.h"

namespace
{
KryptoKnightComm* pk;
Configuration* cfg;
const byte PULSE_PIN=5;
bool slowMcu=false;
}
void dataReceived(byte* data, byte length);
void setSlowMcuSpeed(bool bSlow);

extern void setKeyInfo(byte* remoteId, byte length);


GarageController::GarageController(byte ownAddress,
                                   Configuration* config,
                                   RH_RF95 *prhLora, RH_Serial *prhSerial, byte cableDetectPin): LoRaDevice(ownAddress, prhLora, prhSerial, cableDetectPin)
{
    pk=&k;
    cfg=config;
    setPeerAddress(2);
}

bool GarageController::setup()
{
    if(!LoRaDevice::setup())
    {
        return false;
    }
#ifdef ARDUINO_SAM_DUE
    initRng();
#ifndef DEBUG
    //See §9.1 Peripheral identifiers of the SAM3X datasheet
    pmc_disable_periph_clk(2);      // real-time clock
    pmc_disable_periph_clk(3);      // real-time timer
    pmc_disable_periph_clk(4);      // watchdog timer
    pmc_disable_periph_clk(6);      // EEFC0  flash ctrl
    pmc_disable_periph_clk(7);      // EEFC1  flash ctrl
    pmc_disable_periph_clk(9);      // SMC_SDRAMC
    pmc_disable_periph_clk(10);     // SDRAMC
    pmc_disable_periph_clk(11);     // PIO A
    pmc_disable_periph_clk(12);     // PIO B
    pmc_disable_periph_clk(14);     // PIO D
    pmc_disable_periph_clk(15);     // PIO E
    pmc_disable_periph_clk(16);     // PIO F
    pmc_disable_periph_clk(18);     // USART1
    pmc_disable_periph_clk(19);     // USART2
    pmc_disable_periph_clk(20);     // USART3
    pmc_disable_periph_clk(21);     // HSMCI (SD/MMC ctrl, N/C)
    pmc_disable_periph_clk(22);     // TWI/I2C bus 0 (i.MX6 controlling)
    pmc_disable_periph_clk(23);     // TWI/I2C bus 1
    pmc_disable_periph_clk(25);     // SPI1
    pmc_disable_periph_clk(26);     // SSC (I2S digital audio, N/C)
    pmc_disable_periph_clk(27);     // timer/counter 0
    pmc_disable_periph_clk(28);     // timer/counter 1
    pmc_disable_periph_clk(29);     // timer/counter 2
    pmc_disable_periph_clk(30);     // timer/counter 3
    pmc_disable_periph_clk(31);     // timer/counter 4
    pmc_disable_periph_clk(32);     // timer/counter 5
    pmc_disable_periph_clk(33);     // timer/counter 6
    pmc_disable_periph_clk(34);     // timer/counter 7
    pmc_disable_periph_clk(35);     // timer/counter 8
    pmc_disable_periph_clk(36);     // PWM
    pmc_disable_periph_clk(37);     // ADC
    pmc_disable_periph_clk(38);     // DAC ctrl
    pmc_disable_periph_clk(39);     // DMA ctrl
    pmc_disable_periph_clk(40);     // USB OTG high-speed ctrl
    pmc_disable_periph_clk(42);     // ethernet MAC - N/C
    pmc_disable_periph_clk(43);     // CAN controller 0
    pmc_disable_periph_clk(44);     // CAN controller 1
#else
    debug_println("Debug: No peripherals disabled to save power.");
#endif    
    pinMode(13, OUTPUT);
    digitalWrite(13, LOW);
    setSlowMcuSpeed(true);
#endif
    k.setMessageReceivedHandler(dataReceived);
    k.setKeyRequestHandler(setKeyInfo);
    pinMode(PULSE_PIN, OUTPUT);
    return true;
}

void GarageController::loop()
{
    cableDetect.update();
    if(!cableDetect.read())
    {
        if(cableDetect.fell())
        {
            //on falling edge of cable detect, all keys must be cleared.
            debug_println("Entering pairing mode");
            setSlowMcuSpeed(false);
            k.reset();
            cfg->removeAllKeys();
        }
        else
        {
            //Secure pairing mode
            if (ecdh.loop() == EcdhComm::AUTHENTICATION_OK)
            {
                debug_println("Securely paired");
                cfg->addKey(ecdh.getRemoteId(), ecdh.getMasterKey());
            }
        }
    }else
    {
        if(cableDetect.rose())
        {
            debug_println("Leaving pairing mode");
        }
        switch(k.loop())
        {
        case KryptoKnightComm::AUTHENTICATION_AS_PEER_OK:
            debug_println("Message received by remote initiator");
            break;
        case KryptoKnightComm::NO_AUTHENTICATION:
            setSlowMcuSpeed(true);
            break;
        case KryptoKnightComm::AUTHENTICATION_BUSY:
            setSlowMcuSpeed(false);
            break;
        }
    }
}

void dataReceived(byte* data, byte length)
{
    debug_println("Event received with the following data:");
    debug_printArray(data, length);
    const byte PORTPULSE[4]={0xFE, 0xDC, 0xBA, 0x98};
    if(!memcmp(data,PORTPULSE,sizeof(PORTPULSE)))
    {
        debug_println("Generating port pulse.");
        digitalWrite(PULSE_PIN, HIGH);
        delay(500);
        digitalWrite(PULSE_PIN, LOW);
    }
}

void setSlowMcuSpeed(bool bSlow)
{
#ifdef ARDUINO_SAM_DUE
#ifndef DEBUG
    if(slowMcu!=bSlow)
    {
        slowMcu=bSlow;
        pmc_set_writeprotect(false);
        pmc_mck_set_prescaler(bSlow ? 96 : 16);   // 2.6 MHz or 84MHz
    }
#else
    debug_print("DEBUG: MCU speed throttling disabled: ");debug_println(bSlow ? "down" : "up");
#endif
#endif
}

void setKeyInfo(byte* remoteId, byte length)
{
    debug_println("Remote ID Event received with the following data:");
    debug_printArray(remoteId, length);
    byte* key = cfg->findKey(remoteId, length);
    if(key)
    {
        pk->setRemoteParty(remoteId, length, key);
    }
    else
    {
        debug_println("Key not found in database.");
    }
}


