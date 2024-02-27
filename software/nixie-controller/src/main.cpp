#include <Arduino.h>
#include <TinyWireS.h>

// PB0 is bit 2
// PB1 is bit 1
// PB2 is bit 0
#define ENABLE_ANODE(num) (PORTB = (PORTB & ~0x07) | ((num & 0b01) << 2) | (num & 0b010) | ((num & 0b0100) >> 2))

// PA0 is bit 0
// PA1 is bit 3
// PA2 is bit 1
// PA3 is bit 2
#define ENABLE_CATHODE(num) (PORTA = (PORTA & ~0xF0) | ((num & 0b0100) << 1) | ((num & 0b010) << 1) | ((num & 0b1000) >> 2) | (num & 0b01))

#define DISABLE_CATHODES() (PORTA &= 0x50) 

// 7-bit I2C address
#define I2C_SLAVE_ADDRESS 0x4E

struct I2C_Regs_s {
    uint8_t digits[8];
    uint8_t decimals[8];
};

#define REG_SIZE (sizeof(I2C_Regs_s))

volatile union {
    struct I2C_Regs_s regs;
    uint8_t bytes[REG_SIZE];
} i2c_regs = { .regs = {{0xde, 0xad, 0xbe, 0xef, 0xd0, 0xd0, 0xca, 0xfe},
                        {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08}}
};


//Borrowing from `examples/attiny85_i2c_slave/attiny85_i2c_slave.ino`
#ifndef TWI_RX_BUFFER_SIZE
#define TWI_RX_BUFFER_SIZE (4*REG_SIZE)
#endif

volatile uint8_t reg_address = 0;

void readEvent()
{
    TinyWireS.send(reg_address++);
    if (reg_address > REG_SIZE) reg_address = 0;
}

void writeEvent(uint8_t count)
{
    if ((count < 1) || (count > TWI_RX_BUFFER_SIZE)) return;

    reg_address = TinyWireS.receive();
    --count;
    if (count == 0) return; // this write was setting the address for a read

    while (count--)
    {
        i2c_regs.bytes[reg_address++] = TinyWireS.receive();
        if (reg_address >= REG_SIZE) reg_address = 0;
    }
}


// decimal points are active-high since we're using NPN transistors
void set_dp(uint8_t val)
{
    switch(val)
    {
        case 1:
            PORTA |= (1 << PA5); // RHDP
            break;
    	case 2:
            PORTA |= (1 << PA7); // LHDP
            break;
        case 3:
            PORTA |= (1 << PA5) | (1 << PA7);
            break;
        default:
            PORTA &= ~((1 << PA5) | (1 << PA7));
    }
}

void setup()
{
    // 1 is output, 0 is input for the DDR* registers
//    DDRA |= 0xAF; // PA7, PA5, PA3:0
//
//    DDRB |= 0x07; // PB2:0
//
//    DISABLE_CATHODES(); // To keep anything from turning on we make sure all the cathodes are off
//
//    ENABLE_ANODE(4); // This is a little jank. Ideally we'd be able to set this to output 8 or 9
//                     // since that doesn't energize anything (or set it to 10+ for output blanking)
//                     // but we don't have the high-bit input line wired to the microcontroller.
//                     // Since this board only has four digits soldered on, enabling digit 4 is perfectly safe

    TinyWireS.begin(I2C_SLAVE_ADDRESS);
    TinyWireS.onRequest(readEvent);
    //TinyWireS.onReceive(writeEvent);
}

void loop()
{
    TinyWireS_stop_check();

//    uint8_t numbers[] = {7, 10, 9, 10};
//    for (int i=0; i<4; ++i)
//    {
//        if (numbers[i] < 10) ENABLE_ANODE(i); // we get weird anode ghosting across digits otherwise - may just be the breadboard
//        
//        ENABLE_CATHODE(numbers[i]);
//        set_dp(0);
//
//        delay(1); //still has anode ghosting; probably need to mess with anode resistors
//
//        //prevent ghosting onto the next number
//        DISABLE_CATHODES(); 
//    }
}
