#include <LDR/ldr.h>

void setupLDR()
{
    pinMode(LDR_PIN,INPUT);
}

int readLDR()
{
    int input = analogRead(LDR_PIN);
    return input;
}