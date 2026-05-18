#include <PIR/pir.h>

void setupPIR(){
    pinMode(PIR_PIN,INPUT);
}

int readPIR(){
    int input = digitalRead(PIR_PIN);
    if (input == HIGH){
        return 1;
    } else {
        return 0;
    }
}