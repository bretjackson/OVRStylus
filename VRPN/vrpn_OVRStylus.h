#ifndef VRPN_OVRSTYLUS_H
#define VRPN_OVRSTYLUS_H

#include "vrpn_Analog.h"                // for vrpn_Serial_Analog
#include "vrpn_Button.h"                // for vrpn_Button_Filter
#include "vrpn_Configure.h"             // for VRPN_API
#include "vrpn_Connection.h"            // for vrpn_CONNECTION_LOW_LATENCY, etc
#include "vrpn_Shared.h"                // for timeval
#include "vrpn_Types.h"                 // for vrpn_uint32

// Device drivers for the OVR Stylus

class VRPN_API vrpn_OVRStylus: public vrpn_Serial_Analog,
             public vrpn_Button_Filter
{
public:
	vrpn_OVRStylus(const char * name, 
		       vrpn_Connection * c,
		       const char * port, 
		       int baud = 115200,
		       const int numbuttons = 2);

	virtual ~vrpn_OVRStylus();

	virtual void mainloop();
    
    //Tells the DRV2605 LRA haptic driver on the stylus to play
    // a specific haptic waveform effect. Id's range from 1-123
    // with descriptions: https://cdn-shop.adafruit.com/datasheets/DRV2605.pdf
    virtual void sendHapticCommand(uint8_t hapticEffectID);

protected:
	int _num_read;
	int _expectedNumChars;	// How many characters to expect in the report
	struct timeval _timestamp;	// Time of the last report from the device
	unsigned char _messageStartByte;
    virtual void clear_values(void);    // Set all buttons, analogs back to 0
	 virtual void get_report(void);    // Try to read a report from the device

	// send report iff changed
    virtual void report_changes (vrpn_uint32 class_of_service = vrpn_CONNECTION_LOW_LATENCY);
    // send report whether or not changed
    virtual void report (vrpn_uint32 class_of_service = vrpn_CONNECTION_LOW_LATENCY);
    // NOTE:  class_of_service is only applied to vrpn_Analog
    //  values, not vrpn_Button or vrpn_Dial
    
};
// end of VRPN_OVRSTYLUS_H
#endif

