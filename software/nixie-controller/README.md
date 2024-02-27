This is set up to program an ATTiny84 using an Arduino running the ArduinoISP sketch.

To program, two PIO commands need to be run.

`pio run -t fuses` will burn the correct fuses to disable the 8x clock divider and run the ATTiny at 8MHz.

`pio run -t upload` will upload the actual firmware.

The fuse programming only needs to be run once.

## I2C Testing

I used a Pi Pico running CircuitPython because I wanted a REPL and somebody is borrowing my logic analyzer...

```python
>>> import busio, board
# note that the default 100kHz is too much for our poor ATTiny to handle (or my wires are just a hot mess)
# I also had to add 10K pullups to SDA and SCL because CircuitPython doesn't try to use its internal pullups
>>> i2c = busio.I2C(board.GP3, board.GP2, frequency=10000)
>>> i2c.try_lock()
True
>>> i2c.scan()
[78]
>>> 0x4e
78
# Sweet, that's us
>>> i2c.unlock()
>>> i2c.deinit()
```

After further mucking I got it to talk back very inconsistently with the following `readEvent()`:
```c
volatile uint8_t reg_address = 0;

void readEvent()
{
    TinyWireS.send(reg_address++);
    if (reg_address > REG_SIZE) reg_address = 0;
}
```

```python
>>> i2c.readfrom_into(78, buffer); print(buffer[0])
2
>>> i2c.readfrom_into(78, buffer); print(buffer[0])
3
>>> i2c.readfrom_into(78, buffer); print(buffer[0])
Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
OSError: [Errno 19] No such device
>>> i2c.readfrom_into(78, buffer); print(buffer[0])
Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
OSError: [Errno 19] No such device
>>> i2c.readfrom_into(78, buffer); print(buffer[0])
4
```
Interestingly, this is equally disastrous when running the ATTiny from 5V and 3.3V - so it's possibly something 
stupid that the ATTiny is doing with clock-stretching. Or a pullup problem. Time to find a scope I guess.
