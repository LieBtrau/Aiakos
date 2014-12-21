//Implemenation of SIR (for baudrates of 9600baud up to 115200baud)

#include <util/crc16.h>
#include "irphy.h"
#define DEBUG 1

static byte dataBitsMask;
static byte* packetData;
static byte packetDataCnt=0;
static byte packetIndex=0;
static byte dataReg;
typedef enum{STARTBIT, DATABITS, STOPBIT} SENDSTATE;
static SENDSTATE sendState;
static byte timer0;

const uint8_t BUFFER_SIZE=100;
word dataBuffer[BUFFER_SIZE];
uint8_t dataCtr=0;

word shiftRegister=0;
byte bitCtr=0;

IrPhy::IrPhy(){

}

void processShiftRegister(){
    //full byte including start + stop is read
    if(bitRead(shiftRegister,5)==0 && bitRead(shiftRegister,14)==1){
        //start bit & stop bit detected
#ifdef DEBUG
        Serial.print("\r\ndata: ");
        Serial.print(lowByte(shiftRegister>>6),HEX);
#endif
//        dataBuffer[dataCtr++]=lowByte(shiftRegister>>7);
//        if(dataCtr==BUFFER_SIZE){
//            dataCtr=0;
//        }
    }
    bitCtr=1;
}

void processBit(word icr){
    if(icr<IrPhy::MINIMUM_GAP){
        //Sync lost
        shiftRegister=0;
        bitCtr=0;
#ifdef DEBUG
        Serial.print("sync lost");
#endif
    }else if(icr<IrPhy::ZERO_ONES_MAX){
        //0bit
        shiftRegister>>=1;
        bitCtr++;
#ifdef DEBUG
        Serial.print("0");
#endif
    }else if(icr<IrPhy::ONE_ONES_MAX){
        //0bit+1x1bit
        shiftRegister>>=2;
        shiftRegister|=0x4000;
        bitCtr+=2;
#ifdef DEBUG
        Serial.print("10");
#endif
    }else if(icr<IrPhy::TWO_ONES_MAX){
        //0bit+2x1bit
        shiftRegister>>=3;
        shiftRegister|=0x6000;
        bitCtr+=3;
#ifdef DEBUG
        Serial.print("110");
#endif
    }else if(icr<IrPhy::THREE_ONES_MAX){
        //0bit+3x1bit
        shiftRegister>>=4;
        shiftRegister|=0x7000;
        bitCtr+=4;
#ifdef DEBUG
        Serial.print("1110");
#endif
    }else if(icr<IrPhy::FOUR_ONES_MAX){
        //0bit+4x1bit
        shiftRegister>>=5;
        shiftRegister|=0x7800;
        bitCtr+=5;
#ifdef DEBUG
        Serial.print("11110");
#endif
    }else if(icr<IrPhy::FIVE_ONES_MAX){
        //0bit+5x1bit
        shiftRegister>>=6;
        shiftRegister|=0x7C00;
        bitCtr+=6;
#ifdef DEBUG
        Serial.print("111110");
#endif
    }else if(icr<IrPhy::SIX_ONES_MAX){
        //0bit+6x1bit
        shiftRegister>>=7;
        shiftRegister|=0x7E00;
        bitCtr+=7;
#ifdef DEBUG
        Serial.print("1111110");
#endif
    }else if(icr<IrPhy::SEVEN_ONES_MAX){
        //0bit+7x1bit
        shiftRegister>>=8;
        shiftRegister|=0x7F00;
        bitCtr+=8;
#ifdef DEBUG
        Serial.print("11111110");
#endif
    }else if(icr<IrPhy::EIGHT_ONES_MAX){
        //S+8x1bit
        shiftRegister>>=9;
        shiftRegister|=0x7F80;
        bitCtr+=9;
#ifdef DEBUG
        Serial.print("111111110");
#endif
    }else if(icr<IrPhy::NINE_ONES_MAX){
        //S+9x1bit = reading 0xFF
        shiftRegister>>=10;
        shiftRegister|=0x7FC0;
        bitCtr+=10;
#ifdef DEBUG
        Serial.print("1111111110");
#endif
    }else{
        //startbit of new frame detected
        shiftRegister=0;
        bitCtr=1;
#ifdef DEBUG
        Serial.print("start");
#endif
    }
    if(bitCtr==11){
        processShiftRegister();
    }
    if(bitCtr>11){
        shiftRegister=0;
        bitCtr=0;
    }
#ifdef DEBUG
    Serial.print("\tbitCtr: ");
    Serial.print(bitCtr);
    Serial.print("\tshiftRegister: 0x");
    Serial.println(shiftRegister,HEX);
#endif
}

//ISR for receiving IrDA signals
ISR(TIMER1_CAPT_vect){
    bitSet(PORTD,4);
    int16_t icr=ICR1;
    TCNT1=0;
    dataBuffer[dataCtr++]=icr;
    bitClear(PORTD,4);
}

ISR(TIMER1_OVF_vect){
    /*
    if(bitCtr==9){
        //After the last byte has been sent, its stop bit will not be followed by a new edge.  Hence the stop bit
        //will go undetected.
        //On runout of this timer, an artificial stop bit will be inserted.
        shiftRegister>>=2;
        shiftRegister|=0x4000;
        bitCtr+=2;
        processShiftRegister();
    }else{
        //this is a spurious edge, reset the shiftregister
        shiftRegister=0;
        bitCtr=0;
    }
    */
}

void IrPhy::show(){
    Serial.print("datactr: ");
    Serial.println(dataCtr);
    for(uint16_t i=0;i<dataCtr;i++){
        Serial.print(dataBuffer[i]);
        Serial.print("\t");
        processBit(dataBuffer[i]);
    }
    if(bitCtr==9){
        shiftRegister>>=2;
        shiftRegister|=0x4000;
        bitCtr+=2;
        processShiftRegister();
    }
    Serial.println("--------------");
}

void IrPhy::startTx(byte* buffer, byte size){
#ifdef DEBUG
    for(byte i=0;i<size;i++){
        Serial.print(buffer[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
#endif
    packetDataCnt=size;
    packetData=buffer;
    //Disable timer0
    timer0=TCCR0B;
    bitClear(TCCR0B,CS02);
    bitClear(TCCR0B,CS01);
    bitClear(TCCR0B,CS00);
    packetIndex=0;
    dataBitsMask=0x01;
    TCNT2=0;
    bitSet(TIMSK2,OCIE2A);          //Enable interrupt on OCR2B compare match
}

bool IrPhy::sendRaw(byte* sendBuffer, byte byteCount){
    memcpy(_sendPacket,sendBuffer,byteCount);
    startTx(_sendPacket,byteCount);
    return true;
}

//Construct ASYNC WRAPPER packet: XBOF | BOF | DATA | FCS | EOF
bool IrPhy::send(byte* sendBuffer, byte byteCount){
    word crc=0xFFFF;
    byte size=0;
    //XBOF
    for(size=0;size<10;size++){
        _sendPacket[size]=XBOF;
    }
    //BOF
    _sendPacket[size++]=BOF;
    //DATA
    for(byte i=0;i<byteCount;i++){
        byte c=sendBuffer[i];
        crc=_crc_ccitt_update(crc,c);
        if(c==BOF || c==EOF_FLAG || c==CE){
            _sendPacket[size++]=0x7D;
            c ^= 0x20;
        }
        _sendPacket[size++]=c;
    }
    //FCS
    _sendPacket[size++]= ~lowByte(crc);
    _sendPacket[size++]= ~highByte(crc);
    //EOF
    _sendPacket[size++]=EOF_FLAG;
    startTx(_sendPacket, size);
    return true;
}

bool IrPhy::sendingDone(){
    return bitRead(TIMSK2, OCIE2A);
}

void IrPhy::init()
{
    pinMode(3,OUTPUT);
    pinMode(4,OUTPUT);
#if defined(__AVR_ATmega328P__)
    //Setup timer for sending
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

    //Setup timer for receiving
    //A 16bit timer is needed.
    //It must be fast -> no prescaling
    //It must also be able to detect the XBOF header, which is a train of 11.5kHz pulses.
    //An 8bit timer at this clockspeed rcan only count down to 62.5kHz (without artificial bit increasing by software).
    TCCR1A=0;
    bitSet(TCCR1B, ICNC1);          //Enable noise canceler (uses 4 CLK's)
    bitSet(TCCR1B, ICES1);          //Trigger on rising edge
    bitClear(TCCR1B, WGM13);
    bitClear(TCCR1B, WGM12);        //Timer1 mode 0: Normal, top=0xFFFF
    bitClear(TCCR1B, CS12);         //Timer1 clock: no prescaling
    bitClear(TCCR1B, CS11);
    bitSet(TCCR1B, CS10);
    ICR1=0;
    bitSet(TIMSK1, ICIE1);          //input capture interrupt enable (on ICP-pin = Arduino pin 8)
    bitSet(TIMSK1, TOIE1);          //timer overflow interrupt enable
#else
#error Unsupported MCU
#endif
}



//ISR for generating IrDA signals
ISR(TIMER2_COMPA_vect){
    switch(sendState){
    case STARTBIT:
        bitSet(TCCR2A, COM2B1);
        dataBitsMask=0x01;
        dataReg=packetData[packetIndex];
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
