/* This class will execute the BLE-pairing based on the exchange of 6 digit PINs.
 *
 * !! Be warned that BLE pairing by design is known to have serious weaknesses !!
 *
 * If you want to know more, you can read:
 * https://lacklustre.net/bluetooth/Ryan_Bluetooth_Low_Energy_USENIX_WOOT.pdf
 * https://security.stackexchange.com/questions/100443/security-of-bluetooth-low-energy-ble-link-layer-encryption
 * https://eprint.iacr.org/2013/309.pdf
 *
 */

#include "blepairing.h"

BlePairing::BlePairing(RHReliableDatagram *mgrSer, byte peer):
    pmgrSer(mgrSer),
    peerAddress(peer)
{
}

bool BlePairing::setRemoteBleAddress(byte* address)
{
    return writeDataSer(address, 12);
}

bool BlePairing::getRemoteBleAddress(byte** address)
{
    byte length;
    return readDataSer(address, length) && length==12;
}

bool BlePairing::setPinCode(uint32_t pinCode)
{
    byte array[4];
    memcpy(array, &pinCode, 4);
    return writeDataSer(array, 4);
}

bool BlePairing::getPinCode(uint32_t& pinCode)
{
    byte length;
    byte array[4];
    byte* array2=array;
    if(readDataSer(&array2, length) && length==4)
    {
        memcpy(&pinCode, array2, 4);
    }
}

bool BlePairing::writeDataSer(byte* data, byte length)
{
    Serial.print("Sending serial data...");
#ifdef DEBUG
    print(data, length);
#endif
    return pmgrSer->sendtoWait(data, length, peerAddress);
}

bool BlePairing::readDataSer(byte** data, byte& length)
{
    byte from;
    if (!pmgrSer->available())
    {
        return false;
    }
    if(!pmgrSer->recvfromAck(*data, &length, &from))
    {
        return false;
    }
    if(from != peerAddress)
    {
#ifdef DEBUG
        Serial.println("Sender doesn't match");
#endif
        return false;
    }
#ifdef DEBUG
    Serial.println("Received data: ");print(*data, length);
#endif
    return true;
}


