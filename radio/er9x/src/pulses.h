#ifndef PULSES_H
#define PULSES_H


typedef struct t_PXXData {
    uint8_t   header1;
    uint8_t   rxnum;
    uint16_t  flags;
    int16_t   ch1:12;
    int16_t   ch2:12;
    int16_t   ch3:12;
    int16_t   ch4:12;
    int16_t   ch5:12;
    int16_t   ch6:12;
    int16_t   ch7:12;
    int16_t   ch8:12;
    uint8_t   crc;
} __attribute__((packed)) PXXData;

extern uint8_t PausePulses ;

void startPulses( void ) ;
void setupPulsesPPM( uint8_t proto ) ;
//void setupPulsesPXX();

void setupPulsesSerial( void ) ;

#endif // PULSES_H
