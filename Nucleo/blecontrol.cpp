#include "blecontrol.h"
#include "rn4020.h"

extern SoftwareSerial sw;
rn4020 rn(Serial,8,5,A3,A4,6);
SoftwareSerial* sPortDebug;

bleControl::bleControl()
{
}

bool bleControl::begin(bool bCentral)
{
    if(!rn.begin(2400))
    {
        //Maybe the module is blocked or set to an unknown baudrate.
        if(!rn.doFactoryDefault())
        {
            return false;
        }
        //Factory default baud=115200
        if(!rn.begin(115200))
        {
            return false;
        }
        //Switch to 2400baud
        // + It's more reliable with the ProTrinket 3V.
        // + It also works when the module is in deep sleep mode.
        if(!rn.set(rn4020::BAUDRATE,2400))
        {
            return false;
        }
        //Baudrate only becomes active after resetting the module.
        if(!rn.doReboot(2400))
        {
            return false;
        }
    }
    if(bCentral)
    {
        //Central
        //    ble2_reset_to_factory_default(RESET_SOME);
        //    ble2_set_server_services(0xC0000000);
        //    ble2_set_supported_features(0x82480000);
        //    ble2_device_reboot();
    }else
    {
        //Peripheral
        if(!rn.set(rn4020::SRV_SERVICES, 0xC0000000))
        {
            return false;
        }
        if(!rn.set(rn4020::SUP_FEATURES, 0x02480000))
        {
            return false;
        }
        //        ble2_set_transmission_power(PWR_MIN19_1dBm);
        //
    }
    return true;
}
