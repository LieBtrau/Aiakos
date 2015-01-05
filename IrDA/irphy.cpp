//Implemenation of SIR (for baudrate of 115200baud)

#include <util/crc16.h>
#include "irphy.h"

#define DEBUG2 1

static byte dataBitsMask;
static byte* packetData;
static byte packetDataCnt=0;
static byte packetIndex=0;
static byte dataReg;
typedef enum{STARTBIT, DATABITS, STOPBIT} SENDSTATE;
static SENDSTATE sendState;
static byte timer0;
static word shiftRegister=0;
static byte bitCtr=0;
static word buffer[IrPhy::ASYNC_WRAPPER_SIZE];
static byte start;
static byte end;
static byte cnt;

IrPhy::IrPhy(){

}

bool IrPhy::pop(word& data)
{
    if(!cnt)
    {
        return false;
    }
    data = buffer[start];
    --cnt;
    if (++start > IrPhy::ASYNC_WRAPPER_SIZE)
    {
        start = 0;
    }
    return true;
}


void IrPhy::show(){
    word sr;
    byte data;
    if(!pop(sr)){
        return;
    }
    Serial.print(sr, HEX);
    Serial.print("\t");
    Serial.print(cnt);
    Serial.print("\t");
    Serial.print(start);
    Serial.print("\t");
    Serial.print(end);
    Serial.print("\t");
    if(processShiftRegister(sr, data)){
        Serial.print(data, HEX);
    }
    Serial.println();
}

bool IrPhy::processShiftRegister(word sr, byte& data){
    data=0;
     //startbit=bit6 of shiftRegister, stopBit = bit15 of shiftregister
    bool bRetVal= (bitRead(sr,6)==0 && bitRead(sr,15)==1) ? true : false;
    if(!bRetVal)
    {
        return false;
    }
    data=lowByte(sr>>7);
    return true;
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
    setTimerMode(TX_MODE);
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
    return bitRead(TIMSK1, OCIE1A);
}

void IrPhy::init()
{
    end=0;
    start=0;
    cnt=0;

    pinMode(3, OUTPUT);
    pinMode(4,OUTPUT);
#if defined(__AVR_ATmega328P__)
    //For receiving data, a timer with at least 16bit resolution is needed.
    //It must be fast -> no prescaling allowed
    //It must also be able to detect the XBOF header, which is a train of 11.5kHz pulses.
    //(An 8bit timer at this clockspeed can only count down to 62.5kHz).
    //Power Up Timer1
    bitClear(PRR, PRTIM1);
    //Disconnect OC1B for the time being
    bitClear(TCCR1A, COM1B0);
    bitClear(TCCR1A, COM1B1);
    //Timer1 clock: no prescaling
    bitClear(TCCR1B, CS12);
    bitClear(TCCR1B, CS11);
    bitSet(TCCR1B, CS10);
    //t_pulse = N*(1+OCR1B)/fclk => OCR1B = T_pulse * fclk - 1 = 1.63u * 16e6 - 1 = 25
    OCR1B=25;
    //f = fclk/(N*(1+OCR1A)) => OCR1A = fclk / f - 1 = 16e6 / 115200 - 1 = 138
    OCR1A=138;
    //OC1B-pin is PB2 = Arduino pin 10
    pinMode(10,OUTPUT);
    bitSet(TCCR1B, ICNC1);          //Enable noise canceler (uses 4 CLK's)
    bitSet(TCCR1B, ICES1);          //Trigger on rising edge
    setTimerMode(RX_MODE);
#else
#error Unsupported MCU
#endif
}

void setTimerMode(TIMER_MODE tm){
    switch (tm) {
    case RX_MODE:
        //Set Timer1 in mode 0: Normal, top=0xFFFF
        bitClear(TCCR1A, WGM10);
        bitClear(TCCR1A, WGM11);
        bitClear(TCCR1B, WGM13);
        bitClear(TCCR1B, WGM12);
        bitSet(TIFR1, ICF1);            //Clear pending interrupt in case was one
        bitSet(TIMSK1, ICIE1);          //input capture interrupt enable (on ICP-pin = Arduino pin 8)
        bitSet(TIMSK1, TOIE1);          //timer overflow interrupt enable
        bitClear(TIMSK1, OCIE1A);       //output compare interrupt disable
        break;
    case TX_MODE:
        //Set Timer1 in mode 15: Fast PWM, top at OCR1A
        bitSet(TCCR1A, WGM10);
        bitSet(TCCR1A, WGM11);
        bitSet(TCCR1B, WGM12);
        bitSet(TCCR1B, WGM13);
        //Disable interrupts that are used for receiving data
        bitClear(TIMSK1, ICIE1);
        bitClear(TIMSK1, TOIE1);
        //Reset timer value
        TCNT1=0;
        //Reset sending state machine
        sendState=STARTBIT;
        //Enable interrupt for sending data
        bitSet(TIMSK1, OCIE1A);
        break;
    default:
        break;
    }
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

//ISR for generating IrDA signals
ISR(TIMER1_COMPA_vect){
    switch(sendState){
    case STARTBIT:
        bitSet(TCCR1A, COM1B1);
        dataBitsMask=0x01;
        dataReg=packetData[packetIndex];
        sendState=DATABITS;
        break;
    case DATABITS:
        if(dataReg & dataBitsMask){
            bitClear(TCCR1A, COM1B1);
        }else{
            bitSet(TCCR1A, COM1B1);
        }
        dataBitsMask<<=1;
        if(!dataBitsMask)
        {
            sendState=STOPBIT;
        }
        break;
    case STOPBIT:
        bitClear(TCCR1A, COM1B1);
        if(packetIndex < packetDataCnt - 1)
        {
            packetIndex++;
            sendState=STARTBIT;
        }else{
            //reset packetBuffer
            packetDataCnt=0;
            TCCR0B=timer0;
            setTimerMode(RX_MODE);
        }
        break;
    }
}

//ISR for receiving IrDA signals
//Reading data is achieved by triggering an interrupt on the rising edges of data (i.e. when a 0-bit is being received).
//By measuring the time interval between two consecutive rising edges, the software calculates the number of 1-bits has
//been received in the mean time.  That number of 1-bits will be shifted in in a shiftregister.
//The disadvantage of using this method is that it's quite vulnerable to spurious edges.  It's adviced to leave on the
//edge filtering of the timer capture unit.
//The other option would be to poll the level of the RX-pin at fixed intervals.  The high time of a 0-bit however is only
//1.4us.  To get reliable bit detection, you would have to poll at least every 1.4us/3 = 4us.   On a 16MHz MCU, this leaves
//no time to do other things in the mean time.
ISR(TIMER1_CAPT_vect){
    bitSet(PORTD,4);
    word icr=ICR1;
    TCNT1=0;
    //STEP 1: Fill shift register
    if(icr<IrPhy::ZERO_ONES_MAX){
        //0bit
        shiftRegister>>=1;
        bitCtr++;
    }else if(icr<IrPhy::ONE_ONES_MAX){
        //0bit+1x1bit
        shiftRegister>>=2;
        shiftRegister|=0x4000;
        bitCtr+=2;
    }else if(icr<IrPhy::TWO_ONES_MAX){
        //0bit+2x1bit
        shiftRegister>>=3;
        shiftRegister|=0x6000;
        bitCtr+=3;
    }else if(icr<IrPhy::THREE_ONES_MAX){
        //0bit+3x1bit
        shiftRegister>>=4;
        shiftRegister|=0x7000;
        bitCtr+=4;
    }else if(icr<IrPhy::FOUR_ONES_MAX){
        //0bit+4x1bit
        shiftRegister>>=5;
        shiftRegister|=0x7800;
        bitCtr+=5;
    }else if(icr<IrPhy::FIVE_ONES_MAX){
        //0bit+5x1bit
        shiftRegister>>=6;
        shiftRegister|=0x7C00;
        bitCtr+=6;
    }else if(icr<IrPhy::SIX_ONES_MAX){
        //0bit+6x1bit
        shiftRegister>>=7;
        shiftRegister|=0x7E00;
        bitCtr+=7;
    }else if(icr<IrPhy::SEVEN_ONES_MAX){
        //0bit+7x1bit (reading 0x7F or reading 0xFC)
        shiftRegister>>=8;
        shiftRegister|=0x7F00;
        bitCtr+=8;
    }else if(icr<IrPhy::EIGHT_ONES_MAX){
        //0bit+8x1bit = reading 0xFE (reading 0x7F is only 7 consecutive ones)
        shiftRegister=0x7F80;
        bitCtr+=9;
    }else if(icr<IrPhy::NINE_ONES_MAX){
        //S+9x1bit = reading 0xFF
        shiftRegister=0x7FC0;
        bitCtr+=10;
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
    //STEP 2: Check if shift register is full
    if(bitCtr==10){
        push(shiftRegister);
        bitCtr=0;
    }else if(bitCtr==11){
        push(shiftRegister<<1);
        bitCtr=1;
    }else if(bitCtr>11){
        bitCtr=0;
    }
    bitClear(PORTD,4);
}

ISR(TIMER1_OVF_vect){
    //After the last byte in a packet has been sent, its stop bit will not be followed by a new edge.  Hence the stop bit
    //will go undetected.  The last byte would never be read completely and the state machine would hang.
    //On runout of this timer, we assume that the stop bit has been sent.
    //The data byte currently being read will finish.  The number of ones needed to finish the current byte will be shifted in.
    if(bitCtr){
        //finish the current byte if it was busy
        while(bitCtr<10){
            shiftRegister>>=1;
            shiftRegister|=0x8000;
            bitCtr++;
        }
        push(shiftRegister);
        bitCtr=0;
    }
}
