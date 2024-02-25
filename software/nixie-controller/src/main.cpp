#include<Arduino.h>

void setup()
{
    DDRA |= 1 << 5; //LED on PA5
}

void loop()
{
    PORTA ^= 1<<5;
    delay(1000);
}
