There is a lot of jank going on with the breadboard.

## Actual bug - miswired optocoupler

The optocoupler is currently wired assuming that the pinout of a quad optocoupler is just that of four single optocouplers taped together. This is incorrect and it needs to be rewired to take that into account.  
It is the same as two dual optocouplers taped together, though...

Opening the PCB in KiCad can be helpful for seeing where things are supposed to be routed.

## Programming

We're using an ATTiny84 microcontroller because it's cool and DIP compatible and I had a couple lying around. To program one of these, we need to be able to speak the AVR SPI-like programming protocol. Luckily, there is a pre-existing tool that does this for us, `ArduinoISP`: flashing this sketch to an Arduino allows you to use it to program other AVRs using the standard protocol.  
There is an Arduino Nano wired to the ZIF socket by the Nixies. The ZIF socket is 16-pin while the ATTiny84 is 14-pin; always put the ATTiny84 on the side closest to the lever.

The Arduino Nano will need to be programmed with the `ArduinoISP` sketch and you'll need to set up the ATTiny84 environment in the Arduino IDE. This Instructable does a good enough job of explaining it all:
https://www.instructables.com/Compact-Attiny-138584-Programmer-Using-Arduino-Nan/

Note that the ZIF socket for the programmer likes to fly off the breadboard. If this happens, just stick it back aligned against the edge of the Nixie carrier PCB.

The intended programming flow is to program the ATTiny84 in the ZIF socket connected to the Arduino Nano and then swap it over to the other ZIF socket. This one is also wired so that pin 1 of the ATTiny84 goes in pin 1 of the ZIF socket (and row 1 of the breadboard in this case).

## Power
The board is currently wired to run everything off the VIN pin of the Arduino Nano. The Arduino Nano only gets 5V/100mA from any computer to which it is connected, and all our old TTL logic (not to mention the high-voltage power supply) draw enough current to drop this down pretty far. I included a USB cable with the end cut off and a 0.1" header soldered on; I recommend disconnecting the VIN jumper on the Arduino Nano and using that USB cable instead.

## Anode current limiting

At the moment there are no current-limiting resistors installed for the anodes of the Nixies on the breakout board. This is because my original value was empirically incorrect (too low a value causes the other Nixies on the board to light up when you have only one anode energized - spooky!). I appear to have corrected this to 22kOhms in the schematic.  
You can test by just soldering one on and seeing how it does, or you could try cowboying it and sticking wires in everywhere to run the Nixies by hand. Do be aware that 170V will hurt a bit if you zap yourself. It hurts way less than 120VAC, but it's still not exactly fun.

## I2C Implementation

The yellow and blue jumper wires coming off the ATTiny ZIF socket are connected to PA4 and PA6 for SDA and SCL respectively. Using the TinyWireS library, you should be able to implement an I2C interface that will allow an external microcontroller to configure which cathodes are activated for each digit. Since we have a reasonable amount of RAM, I'd probably have one set of eight registers for each Nixie's numeric cathodes and then another set of eight for its optional/decimal cathodes.

One useful implementation feature would be pre-computing the PORTA values for each configuration. In the case of the 4-value optional-cathode code I have in the pseudocode, you could make a 2D array:

```c
const unsigned uint8_t cathode_lookup[4][10] PROGMEM = {
   { 0x00, 0x01, 0x04, 0x05, 0x08, 0x09, 0x0C, 0x0D, 0x02, 0x06 },
   { 0x20, 0x21, 0x24, 0x25, 0x28, 0x29, 0x2C, 0x2D, 0x22, 0x26 },
   { 0x80, 0x81, 0x84, 0x85, 0x88, 0x89, 0x8C, 0x8D, 0x82, 0x86 },
   { 0xA0, 0xA1, 0xA4, 0xA5, 0xA8, 0xA9, 0xAC, 0xAD, 0xA2, 0xA6 }
};
...

PORTA = pgm_read_byte(&cathode_lookup[optionals[i]][numbers[i]]) | (PORTA & 0x50);
```

There are a couple tricky things going on here. First, we're doing all the bitshuffling for the 74141 driver pins (the digit cathodes) in the low nybble. Recall from the schematic that the order is `<bit 2><bit 1><bit 3><bit 0>`, so selecting the cathode for numeral 3, for example, would give `0b0101` or 0x5. 
The next tricky thing is that we've encoded the 4-value decimal point lookup value (0-off, 1-`CA`, 2-`CB`, 3-`CA+CB`) into the table itself as the upper nybble (bits 7 and 5 for `CB` and `CA` respectively).  
The last thing is that we can't just write this wholesale to PORTA since the I2C pins also live there - so we start off with those values cleared in our precomputed array and then bitwise-OR them back in if they were set.

Oh I guess that PROGMEM is also tricky - rather than keeping this table in RAM we're storing it in program flash since there is more of it. It's probably an unnecessary optimization since we have 512 bytes of RAM and I don't think this program is that complex, but lookup tables love to live in program flash.
