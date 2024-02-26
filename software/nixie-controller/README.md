This is set up to program an ATTiny84 using an Arduino running the ArduinoISP sketch.

To program, two PIO commands need to be run.

`pio run -t fuses` will burn the correct fuses to disable the 8x clock divider and run the ATTiny at 8MHz.

`pio run -t upload` will upload the actual firmware.

The fuse programming only needs to be run once.
