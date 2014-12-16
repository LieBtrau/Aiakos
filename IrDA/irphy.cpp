//Implemenation of SIR (for baudrates of 9600baud up to 115200baud)

#include <util/crc16.h>
#include "irphy.h"
#define DEBUG 1

static byte dataBitsMask;
static byte sendPacket[IrPhy::ASYNC_WRAPPER_SIZE];
static byte packetDataCnt=0;
static byte packetIndex=0;
static byte dataReg;
typedef enum{STARTBIT, DATABITS, STOPBIT} SENDSTATE;
static SENDSTATE sendState;
static byte timer0;

IrPhy::IrPhy(){

}

//Construct ASYNC WRAPPER packet: XBOF | BOF | DATA | FCS | EOF
bool IrPhy::send(byte* sendBuffer, byte byteCount){
    word crc=0xFFFF;
    //XBOF
    for(packetDataCnt=0;packetDataCnt<10;packetDataCnt++){
        sendPacket[packetDataCnt]=XBOF;
    }
    //BOF
    sendPacket[packetDataCnt++]=BOF;
    //DATA
    for(byte i=0;i<byteCount;i++){
        byte c=sendBuffer[i];
        crc=_crc_ccitt_update(crc,c);
        if(c==BOF || c==EOF_FLAG || c==CE){
            sendPacket[packetDataCnt++]=0x7D;
            c ^= 0x20;
        }
        sendPacket[packetDataCnt++]=c;
    }
    //FCS
    sendPacket[packetDataCnt++]= ~lowByte(crc);
    sendPacket[packetDataCnt++]= ~highByte(crc);
    //EOF
    sendPacket[packetDataCnt++]=EOF_FLAG;
#ifdef DEBUG
    for(byte i=0;i<packetDataCnt;i++){
        Serial.print(sendPacket[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
#endif
    //Start TX
    //Disable timer0
    timer0=TCCR0B;
    bitClear(TCCR0B,CS02);
    bitClear(TCCR0B,CS01);
    bitClear(TCCR0B,CS00);
    packetIndex=0;
    dataBitsMask=0x01;
    TCNT2=0;
    bitSet(TIMSK2,OCIE2A);          //Enable interrupt on OCR2B compare match
    return true;
}

void IrPhy::init()
{
    pinMode(3,OUTPUT);
    //debug
    pinMode(4,OUTPUT);
#if defined(__AVR_ATmega328P__)
    bitClear(PRR,PRTIM2);            //Power up Timer2
    bitSet(TCCR2B,WGM22);            //Put timer 2 in fast PWM-mode with top at OCR2A
    bitSet(TCCR2A,WGM21);
    bitSet(TCCR2A,WGM20);
    bitClear(TCCR2A, COM2B1);          //Clear OC2B-pin upon compare match
    bitClear(TCCR2A, COM2B0);
    bitClear(TCCR2B, CS22);          //No prescaler, fTimer = fclk = 16MHz
    bitClear(TCCR2B, CS21);
    bitSet(TCCR2B, CS20);
    //T_pulse = N*(1+OCR2B)/fclk => OCR2B = T_pulse * fclk - 1 = 1.63u * 16e6 - 1 = 25
    OCR2B=25;
    //f = fclk/(N*(1+OCR2A)) => OCR2A = fclk / f - 1 = 16e6 / 115200 - 1 = 138
    OCR2A=138;                        //In fast PWM-mode, timer will be cleared when it reaches OCR2A value.
#else
#error Unsupported MCU
#endif
}

ISR(TIMER2_COMPA_vect){
    switch(sendState){
    case STARTBIT:
        bitSet(TCCR2A, COM2B1);
        dataBitsMask=0x01;
        dataReg=sendPacket[packetIndex];
        sendState=DATABITS;
        break;
    case DATABITS:
        if(dataReg & dataBitsMask){
            bitClear(TCCR2A, COM2B1);
        }else{
            bitSet(TCCR2A, COM2B1);
        }
        dataBitsMask<<=1;
        if(!dataBitsMask)
        {
            sendState=STOPBIT;
        }
        break;
    case STOPBIT:
        bitClear(TCCR2A, COM2B1);
        if(packetIndex < packetDataCnt - 1)
        {
            packetIndex++;
            sendState=STARTBIT;
        }else{
            //stop interrupt routine
            bitClear(TIMSK2,OCIE2A);
            //reset packetBuffer
            packetDataCnt=0;
            TCCR0B=timer0;
        }
        break;
    }
}
