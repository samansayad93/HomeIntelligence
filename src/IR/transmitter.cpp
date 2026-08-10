#include <IR/transmitter.h>

bool transmitIRSignal(const IRSignal *signal)
{
    if (signal == nullptr || signal->rawLength == 0)
    {
        return false;
    }

    switch (signal->protocol)
    {
    case decode_type_t::NEC:
        Serial.println("Transmitting NEC signal: ");
        irTransmitter.sendNEC(signal->value, signal->bits);
        break;

    case decode_type_t::SONY:
        Serial.println("Transmitting SONY signal: ");
        irTransmitter.sendSony(signal->value, signal->bits);
        break;

    case decode_type_t::SAMSUNG:
        Serial.println("Transmitting SAMSUNG signal: ");
        irTransmitter.sendSAMSUNG(signal->value, signal->bits);
        break;

    default:
        Serial.println("Transmitting RAW signal: ");
        irTransmitter.sendRaw(signal->rawData, signal->rawLength, 38);
    }
    pulseStatusLED();
    return true;
}
