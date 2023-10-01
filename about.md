## Theory of Operation

A Nixie tube is a special case of a cold-cathode neon tube. Rather than using the typical high-frequency plasma that you see in CCFLs, a Nixie relies upon individual shaped "filaments" (these aren't actually incandescent filaments that glow due to the current being passed through them, but it is convenient to use the "filament" shorthand).
The anode, or grid, of the Nixie is brought to a high potential (around 170). When one of the "filament" cathodes is grounded, it causes the gas around that filament to ionize, causing the characteristic blue-orange glow. Note that the grid surrounds the entire front face of the tube, with the cathodes being stacked atop each other.

One concern with Nixies, besides their obvious physical fragility due to their glass-metal construction, is that prolonged use of a given cathode can lead it to deposit metal onto other cathodes. This produces an uneven, blotchy appearance and is known as cathode poisoning. Consequently, many Nixie driver boards cycle through all the cathodes periodically to burn off any deposits.  
A fun thing that you will notice is that the digits' cathodes are not arranged in sequential order but rather in a geometric order such that the simplest (and thus least-obscuring) elements are toward the rear of the tube. This is typically the "1" digit.

## Driver Topologies

### Direct-drive
The simplest way to drive a set of Nixies is to energize all of the anodes and then connect one cathode from each tube to ground. Due to the voltage drop across the tube itself, this requires drive circuitry capable of sinking about 60V. One can do this with individual transistors or MOSFETS with the proper voltage rating, but life is too short to solder one component for each cathode of each tube...

What appears to be the standard in the casual hobbyist community is to use VFD driver chips, which conveniently are designed to handle these voltage ranges and often come with a nice shift-register package. THe HV5622 is a popular choice but is a relatively fine-pitch 0.5mm QFP device and difficult for novices to solder. (Trivia fact - the HV5622 datasheet says it expects 12V input logic, but 5V or even 3V logic appears to work just fine. Go figure.)  
There are other Microchip devices in the same category, but I only have the HV5622's model number memorized.

Back in the late 50s and early 60s when Nixies were in vogue, a different cathode driver chip was used: the 7441/74141 high-voltage BCD-to-Decimal decoder. It has a four-bit input that selects which of the ten output pins to allow to sink current. Inputs greater than 9 lead to no output on the 74141 but wrap around modulo ten on the 7441. Old designs will use one of these chips per Nixie tube, meaning that each tube only needs four drive lines rather than ten.  
The 74141, with its over-range output blanking, was cloned by the Russians as the K155ID1, and this clone is much more widely available today.


### Multiplexing

Using these Soviet-era chips is expensive both in money and board space constraints, however. The modern solution is to use one of the aforementioned VFD drivers, but those are also expensive and can be annoying to solder. The direct-drive approach also uses a lot of power, which isn't great.

The solution is to multiplex the Nixies: we will tie all the cathodes together (i.e. all the 1's go to a single pin, all the 2's go to a single pin, etc.) and only activate one anode at a time.  
Thus, we will ground all the cathodes for a given numeric digit, but only the cathode whose anode grid is energized will light. By cycling through the anodes quickly enough, this results in a display that appears to the human eye to have all its Nixies lit.

For these designs, it is common to use one of the 74141 drive chips for the cathodes. The anode drivers see much greater freedom of expression. While dual-transistor drive circuits can work, we are going to use optocouplers because they are conceptually easier to understand (and I happened to have a quad-optocoupler unit on hand for testing).

## Schematic

### Standard Nixie board pinout

| Pin | Name | Value        |
|----|----|-----------------|
| 1  | C0 | Cathode for 0's |
| 2  | C1 | Cathode for 1's |
| 3  | C2 | Cathode for 2's |
| 4  | C3 | Cathode for 3's |
| 5  | C4 | Cathode for 4's |
| 6  | C5 | Cathode for 5's |
| 7  | C6 | Cathode for 6's |
| 8  | C7 | Cathode for 7's |
| 9  | C8 | Cathode for 8's |
| 10 | C9 | Cathode for 9's |
| 11 | CA | Extra cathode (left decimal point) |
| 12 | CB | Extra cathode (right decimal point) |
| 13 | A0 | Anode for digit 0 |
| 14 | A1 | Anode for digit 1 |
| 15 | A2 | Anode for digit 2 |
| 16 | A3 | Anode for digit 3 |
| 17 | A4 | Anode for digit 4 |
| 18 | A5 | Anode for digit 5 |
| 19 | A6 | Anode for digit 6 |
| 20 | A7 | Anode for digit 7 |

For my design, I split the board that holds the individual Nixies apart from the board with the drive logic. This allows for some additional voltage separation and also makes PCB routing possible. Since I want to be able to drive different sets of Nixie tubes with the same controller board, I've made this very informal 0.1" header standard.

The first ten pins are the bussed cathodes for each digit. The two pins after that are for alternate items like INS-1 neon bulbs or the left/right side decimal points present in some Nixie tubes (the IN-16 has both, while IN-12As and IN_12Bs differ based on whether they include a left or a right side decimal point).

After that, we have up to eight different drive lines for the anode grids of each Nixie. I realized that I had excess GPIO capacity, so I extended the board design to handle eight digits with the marginal drive circuitry for the additional four digits being easily snapped off the PCB with no ill effects.

### Controller implementation notes

From a systems-level perspective, we have an ATTiny84 (U1) microcontroller that is responsible for the multiplexing of the Nixies. The idea is that it will present a register-based interface over I2C wherein the individual Nixies' values can be specified and then alter its timings to produce that effect.

For anode selection, a low-voltage 7445 BCD-to-Decimal decoder (U3) has been selected in order to save on pin count - using only three pins, we can address up to eight Nixies' anodes.  
This decoder then drives one or two CNY74-4H optocouplers (U4, U5) depending on how many Nixies are being used. Because this is an active-low decoder (similarly to its 7441/74141 buddy from high-voltage land), we use it to sink current through a given optocoupler's LED to ground.  
This then activates the phototransistor on the other side of the optocoupler, which connects the high-voltage rail to that Nixie digit.

For cathode selection, we are using a classic 74141 decoder (U2), as well as a pair of high-voltage MPSA42 NPN transistors for the extra/decimal-point cathodes.

Since I'm a wuss, high voltage (170V) is provided by an off-the-shelf 5V-to-170V boost converter from Aliexpress. Unfortunately this is no longer produced, but the pinout is simple enough that it will be easy to retrofit anything into its place with a few bodge wires.

Note that pin assignments on the ATTiny84 are almost entirely based on what made the PCB routing easy rather than what would make for easy code. Routing a purely through-hole PCB is made both easier and harder by the eponymous through-holes: while they do provide automatic connection points on every layer, this means they must also be avoided on every layer.  
The exception to this rule is that the PA4 and PA6 pins were assigned to SCL and SDA respectively since that is where the native I2C/TWI interface is connected and that's way better than bit-banging it.

### A priori code example

I'm writing this on the airplane without any datasheets or syntax checking, so it will probably be horrifically buggy. Hopefully the comments and general flow provide enough pseudocode to be helpful.

```c

#define ENABLE_ANODE(num) (PORTB = (PORTB & ~0x07) | ((num & 0b01) << 2) | (num & 0b010) | ((num & 0b0100) >> 2))

#define ENABLE_CATHODE(num) (PORTA = (PORTA & ~0xF0) | ((num & 0b0100) << 1) | ((num & 0b010) << 1) | ((num & 0b1000) >> 2) | (num & 0b01))

# define DISABLE_CATHODES() (PORTA |= 0x0F; PORTA &= ~((1<<PA5) | (1<<PA7)))

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
            PORTA |= (1 << PA5 | (1 << PA7);
            break;
        default:
            PORTA &= ~((1 << PA5) | (1 << PA7));
    }
}

void setup()
{

    // 1 is output, 0 is input for the DDR* registers
    DDRA |= 0xAF; // PA7, PA5, PA3:0

    DDRB |= 0x07; // PB2:0

    ENABLE_ANODE(0); // This is a little jank. Ideally we'd be able to set this to output 8 or 9
                     // since that doesn't energize anything (or set it to 10+ for output blanking)
                     // but we don't have the high-bit input line wired to the microcontroller.

    DISABLE_CATHODES(); // To keep anything from turning on we make sure all the cathodes are off
}

void loop()
{
    uint8_t numbers[] = {1, 3, 3, 7};
    for (int i=0; i<4; ++i)
    {
        ENABLE_ANODE(i);
        ENABLE_CATHODE(numbers[i]);
        set_dp(i%2 + 1);
        delay(1); // total guesswork here; may be unnecessary

        //prevent ghosting onto the next number
        DISABLE_CATHODES(); 
    }
}
```

Basic conventions here - `DDR*` sets the port direction and `PORT*` sets the output value for the port. We aren't reading any inputs here so life is nice and easy. For now!  
Note that my bit-addressing practices on the PORT accesses are a bit lazy, especially when it comes to the hard-coded bitswapping in `ENABLE_ANODE` and `ENABLE_CATHODE`. Usually I wind up doing something like `#define LHDP (1<<PA7)` so that I can just do `PORTA |= LHDP` later.
