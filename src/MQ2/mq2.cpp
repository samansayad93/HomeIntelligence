#include <MQ2/mq2.h>

void setupMQ2()
{
    pinMode(MQ2_PIN, INPUT);
}

int readMQ2()
{
    return analogRead(MQ2_PIN);
}

bool checkThreshold(int value)
{
    if (value > MQ2_THRESHOLD)
    {
        return true;
    }
    return false;
}