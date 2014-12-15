#include "irphy.h"

static const byte TX_BUFFER_SIZE=20;
static byte dataBitsMask;
static byte txBuffer[TX_BUFFER_SIZE];
static byte txIndex=0;
static byte txCtr=0;
static byte dataReg;
typedef enum{STARTBIT, DATABITS, STOPBIT} SENDSTATE;
static SENDSTATE sendState;

IrPhy::IrPhy(){

}

bool IrPhy::doTransmission(){
    TCNT2=0;
    if(txIndex==0){
        return false;
    }
    txCtr=0;
    bitSet(TIMSK2,OCIE2A);          //Enable interrupt on OCR2B compare match
    dataBitsMask=0x01;
    return true;
}

bool IrPhy::write(byte c){
    if(txIndex<TX_BUFFER_SIZE){
        txBuffer[txIndex++]=c;
        return true;
    }else{
        return false;
    }
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
        dataReg=txBuffer[txCtr];
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
        if(txCtr<txIndex-1)
        {
            txCtr++;
            sendState=STARTBIT;
        }else{
            //stop interrupt routine
            bitClear(TIMSK2,OCIE2A);
            //reset sendBuffer
            txIndex=0;
        }
        break;
    }
}
