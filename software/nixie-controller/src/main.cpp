#include<Arduino.h>

// PB0 is bit 2
// PB1 is bit 1
// PB2 is bit 0
#define ENABLE_ANODE(num) (PORTB = (PORTB & ~0x07) | ((num & 0b01) << 2) | (num & 0b010) | ((num & 0b0100) >> 2))

// PA0 is bit 0
// PA1 is bit 3
// PA2 is bit 1
// PA3 is bit 2
#define ENABLE_CATHODE(num) (PORTA = (PORTA & ~0xF0) | ((num & 0b0100) << 1) | ((num & 0b010) << 1) | ((num & 0b1000) >> 2) | (num & 0b01))

# define DISABLE_CATHODES() (PORTA &= 0x50) 

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
    DDRA |= 0xEF; // PA7, PA6, PA5, PA3:0

    DDRB |= 0x07; // PB2:0

    DISABLE_CATHODES(); // To keep anything from turning on we make sure all the cathodes are off

    ENABLE_ANODE(4); // This is a little jank. Ideally we'd be able to set this to output 8 or 9
                     // since that doesn't energize anything (or set it to 10+ for output blanking)
                     // but we don't have the high-bit input line wired to the microcontroller.
                     // Since this board only has four digits soldered on, enabling digit 4 is perfectly safe
}

void loop()
{
    uint8_t numbers[] = {7, 10, 9, 10};
    for (int i=0; i<4; ++i)
    {
        if (numbers[i] < 10) ENABLE_ANODE(i); // we get weird anode ghosting across digits otherwise - may just be the breadboard
        
        ENABLE_CATHODE(numbers[i]);
        set_dp(0);

        delay(1); //still has anode ghosting; probably need to mess with anode resistors

        //prevent ghosting onto the next number
        DISABLE_CATHODES(); 
    }
}
