#include <LDR/ldr.h>

int readLDR()
{
    int input = analogRead(LDR_PIN);
    return input;
}