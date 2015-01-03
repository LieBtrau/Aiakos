//Implemenation of SIR (for baudrate of 115200baud)

#include <util/crc16.h>
#include "irphy.h"
#include "../RingBuffer.h"

#define DEBUG2 1

static byte dataBitsMask;
static byte* packetData;
static byte packetDataCnt=0;
static byte packetIndex=0;
static byte dataReg;
typedef enum{STARTBIT, DATABITS, STOPBIT} SENDSTATE;
static SENDSTATE sendState;
static byte timer0;

word shiftRegister=0;
byte bitCtr=0;
//RingBuffer rb(IrPhy::ASYNC_WRAPPER_SIZE);
word buffer[IrPhy::ASYNC_WRAPPER_SIZE];
byte start;
byte end;
byte cnt;

IrPhy::IrPhy(){

}

void push(word value)
{
    buffer[end] = value;
    if (++end > IrPhy::ASYNC_WRAPPER_SIZE) end = 0;
    if (cnt == IrPhy::ASYNC_WRAPPER_SIZE) {
        if (++start > IrPhy::ASYNC_WRAPPER_SIZE) start = 0;
    } else {
        ++cnt;
    }
}


void IrPhy::show(){
    Serial.print("bitctr: ");
    Serial.println(bitCtr);
    Serial.print("shiftRegister: ");
    Serial.println(shiftRegister,HEX);
    Serial.println("shift");
    //    while(rb.count())
    //    {
    //        processShiftRegister(rb.pop());
    //    }
    for(byte i=0;i<cnt;i++){
        processShiftRegister(buffer[i]);
    }
    Serial.println("--------------");
}

void IrPhy::processShiftRegister(word sr){
    //full byte including start bit of next byte: bitCtr=11 -> startbit = bit5 of shiftRegister, stopbit = bit14 of shiftregister
    //full byte, no start bit of next byte: bitCtr=10 -> startbit=bit6 of shiftRegister, stopBit = bit15 of shiftregister
    if(bitRead(sr,6)==0 && bitRead(sr,15)==1){
        //start bit & stop bit detected
#ifdef DEBUG2
        Serial.print("data: ");
        Serial.println(lowByte(sr>>7),HEX);
#endif
    }else{
        //wrong data
#ifdef DEBUG2
        Serial.println("\treset");
#endif
    }
}


//ISR for receiving IrDA signals
ISR(TIMER1_CAPT_vect){
    bitSet(PORTD,4);
    word icr=ICR1;
    TCNT1=0;

    if(icr<IrPhy::ZERO_ONES_MAX){
        //0bit
        shiftRegister>>=1;
        bitCtr++;
#ifdef DEBUG
        Serial.print("\t0");
#endif
    }else if(icr<IrPhy::ONE_ONES_MAX){
        //0bit+1x1bit
        shiftRegister>>=2;
        shiftRegister|=0x4000;
        bitCtr+=2;
#ifdef DEBUG
        Serial.print("\t10");
#endif
    }else if(icr<IrPhy::TWO_ONES_MAX){
        //0bit+2x1bit
        shiftRegister>>=3;
        shiftRegister|=0x6000;
        bitCtr+=3;
#ifdef DEBUG
        Serial.print("\t110");
#endif
    }else if(icr<IrPhy::THREE_ONES_MAX){
        //0bit+3x1bit
        shiftRegister>>=4;
        shiftRegister|=0x7000;
        bitCtr+=4;
#ifdef DEBUG
        Serial.print("\t1110");
#endif
    }else if(icr<IrPhy::FOUR_ONES_MAX){
        //0bit+4x1bit
        shiftRegister>>=5;
        shiftRegister|=0x7800;
        bitCtr+=5;
#ifdef DEBUG
        Serial.print("\t11110");
#endif
    }else if(icr<IrPhy::FIVE_ONES_MAX){
        //0bit+5x1bit
        shiftRegister>>=6;
        shiftRegister|=0x7C00;
        bitCtr+=6;
#ifdef DEBUG
        Serial.print("\t111110");
#endif
    }else if(icr<IrPhy::SIX_ONES_MAX){
        //0bit+6x1bit
        shiftRegister>>=7;
        shiftRegister|=0x7E00;
        bitCtr+=7;
#ifdef DEBUG
        Serial.print("\t1111110");
#endif
    }else if(icr<IrPhy::SEVEN_ONES_MAX){
        //0bit+7x1bit (reading 0x7F or reading 0xFC)
        shiftRegister>>=8;
        shiftRegister|=0x7F00;
        bitCtr+=8;
#ifdef DEBUG
        Serial.print("\t11111110");
#endif
    }else if(icr<IrPhy::EIGHT_ONES_MAX){
        //0bit+8x1bit = reading 0xFE (reading 0x7F is only 7 consecutive ones)
        shiftRegister=0x7F80;
        bitCtr+=9;
#ifdef DEBUG
        Serial.print("\t111111110");
#endif
    }else if(icr<IrPhy::NINE_ONES_MAX){
        //S+9x1bit = reading 0xFF
        shiftRegister=0x7FC0;
        bitCtr+=10;
#ifdef DEBUG
        Serial.print("\t1111111110");
#endif
    }else{
        //very large gap, but smaller than timer overflow
        if(bitCtr){
            //finish the current byte if it was busy
            while(bitCtr<10){
                shiftRegister>>=1;
                shiftRegister|=0x8000;
                bitCtr++;
            }
        }
        //There's a received edge.  It must be starting edge of the next package
        shiftRegister>>=1;
        bitCtr++;
    }
#ifdef DEBUG
    Serial.print("\tbitCtr: ");
    Serial.print(bitCtr);
#endif
    if(bitCtr==10){
        //rb.push(shiftRegister);
        push(shiftRegister);
        //dataBuf[dataCtr++]=shiftRegister;
#ifdef DEBUG
        Serial.print("\r\nDecoded: ");
        Serial.println(shiftRegister, HEX);
#endif
        bitCtr=0;
    }else if(bitCtr==11){
        //rb.push(shiftRegister<<1);
        push(shiftRegister<<1);
        //dataBuf[dataCtr++]=shiftRegister<<1;
#ifdef DEBUG
        Serial.print("\r\nDecoded: ");
        Serial.println(shiftRegister<<1, HEX);
#endif
        bitCtr=1;
    }else if(bitCtr>11){
        bitCtr=0;
    }
#ifdef DEBUG
    Serial.print("\tbitCtr: ");
    Serial.print(bitCtr);
    Serial.print("\tshiftRegister: 0x");
    Serial.println(shiftRegister,HEX);
#endif

    bitClear(PORTD,4);
}

ISR(TIMER1_OVF_vect){
    //    //After the last byte has been sent, its stop bit will not be followed by a new edge.  Hence the stop bit
    //    //will go undetected.  The last byte would never be read completely and the state machine would hang.
    //    //On runout of this timer, the data byte currently being read will finish.  The number of
    //    //ones needed to finish the current byte will be shifted in.
    if(bitCtr){
        //finish the current byte if it was busy
        while(bitCtr<10){
            shiftRegister>>=1;
            shiftRegister|=0x8000;
            bitCtr++;
        }
        //rb.push(shiftRegister);
        push(shiftRegister);
        //dataBuf[dataCtr++]=shiftRegister;
        bitCtr=0;
    }

    //    if(dataCtr>0 && dataBuffer[dataCtr-1]!=IrPhy::MAXIMUM_GAP){
    //        dataBuffer[dataCtr++]=IrPhy::MAXIMUM_GAP;
    //    }
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
    end=0;
    start=0;
    cnt=0;

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
