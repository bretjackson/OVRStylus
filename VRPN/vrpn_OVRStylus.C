// vrpn_OVRStylus.C: VRPN driver for OVR Stylus


#include <stdio.h>                      // for fprintf, stderr, printf

#include "vrpn_BaseClass.h"             // for ::vrpn_TEXT_WARNING, etc
#include "vrpn_Serial.h"
#include "vrpn_Shared.h"                // for timeval, vrpn_gettimeofday
#include "vrpn_OVRStylus.h"
//#include <fcntl.h>
//#include <stdlib.h>
//#if !defined(_WIN32)
//#include <sys/ioctl.h>
//#include <unistd.h>
//#endif
//#include <string.h>

#define VRPN_TIMESTAMP_MEMBER _timestamp // Configuration required for vrpn_MessageMacros in this class.
#include "vrpn_MessageMacros.h"         // for VRPN_MSG_INFO, VRPN_MSG_WARNING, VRPN_MSG_ERROR

#define MAX_OVRSTYLUS_BUTTONS  2


// analog status flags
//vrpn_ANALOG_SYNCING = (2); Looking for the first character of report
//vrpn_ANALOG_REPORT_READY = (1); New message is ready to report
//vrpn_ANALOG_PARTIAL = (0); Still reading report
//vrpn_ANALOG_FAIL = (-2); Error received

vrpn_OVRStylus::vrpn_OVRStylus(const char * name,
                               vrpn_Connection * c,
                               const char * port,
                               int baud/* = 115200*/,
                               const int numbuttons/* = 2*/) :
vrpn_Serial_Analog(name, c, port, baud),
vrpn_Button_Filter(name, c)
{
    if (numbuttons > MAX_OVRSTYLUS_BUTTONS) {
        fprintf(stderr,"vrpn_OVRSTYLUS: Can only support %d buttons, not %d\n", (int)MAX_OVRSTYLUS_BUTTONS, (int)numbuttons);
        vrpn_Button::num_buttons = MAX_OVRSTYLUS_BUTTONS;
    }
    else {
        vrpn_Button::num_buttons = numbuttons;
    }
    
    vrpn_Analog::num_channel = 1;
    
    clear_values();
    
    _num_read = 0;
    _expectedNumChars = 1;
    _messageStartByte = '!';
    
    status = vrpn_ANALOG_SYNCING;
}

vrpn_OVRStylus::~vrpn_OVRStylus()
{
}

void vrpn_OVRStylus::sendHapticCommand(uint8_t hapticEffectID)
{
    if (hapticEffectID >= 1 && hapticEffectID <= 123){
        unsigned char msg[2];
        sprintf((char*)msg, "%u\n", hapticEffectID);
        vrpn_write_characters(serial_fd, &msg[0], 2);
    }
    else {
        VRPN_MSG_WARNING("Error sending haptic command. Effect id must be between 1-123 inclusive");
    }
}

void vrpn_OVRStylus::clear_values()
{
    // Initialize the state of all the buttons
    memset(vrpn_Button::buttons, 0, sizeof(vrpn_Button::buttons));
    memset(vrpn_Button::lastbuttons, 0, sizeof(vrpn_Button::lastbuttons));
    
    vrpn_Analog::channel[0] = vrpn_Analog::last[0] = 0;
}

void vrpn_OVRStylus::mainloop()
{
    get_report();
    server_mainloop();
    vrpn_gettimeofday(&_timestamp, NULL);
}

void vrpn_OVRStylus::report_changes(vrpn_uint32 class_of_service)
{
    vrpn_Analog::timestamp = _timestamp;
    vrpn_Button::timestamp = _timestamp;
    
    vrpn_Analog::report_changes(class_of_service);
    vrpn_Button::report_changes();
}

void    vrpn_OVRStylus::report(vrpn_uint32 class_of_service)
{
    vrpn_Analog::timestamp = _timestamp;
    vrpn_Button::timestamp = _timestamp;
    
    vrpn_Analog::report(class_of_service);
    vrpn_Button::report_changes();
}

void vrpn_OVRStylus::get_report()
{
    int i;
    
    if (status == vrpn_ANALOG_SYNCING) {
        if (1 == vrpn_read_available_characters(serial_fd, buffer, 1)) {
            // if not a record start, we have an error
            if (buffer[0] != _messageStartByte) {
                VRPN_MSG_WARNING("Error reading OVRStylus data stream, finding start byte");
                return;
            }
            
            // we got a good start byte... we're reading now
            _num_read = 0;   //< Ignore the status byte for the following record
            status = vrpn_ANALOG_PARTIAL;
        }
    }
    if (status == vrpn_ANALOG_PARTIAL){
        // we broke out.. if we're not reading, then we have nothing to do
        
        // we're reading now, get the report
        // get the expected number of data record bytes
        int result = vrpn_read_available_characters(serial_fd,
                                                    &buffer[_num_read], _expectedNumChars-_num_read);
        
        if (result < 0) {
            VRPN_MSG_WARNING("Error reading OVRStylus data stream");
            status = vrpn_ANALOG_SYNCING;
            return;
        }
        _num_read += result;
        // If we don't have a full record, go back again.
        if (_num_read < _expectedNumChars) {
            return;
        }
        
        
        
        for (i = 0; i < num_buttons; i++) {
            lastbuttons[i] = buttons[i];
        }
        
        buttons[0] = static_cast<unsigned char>((buffer[0] & (1 << 7)) ? VRPN_BUTTON_ON : VRPN_BUTTON_OFF);
        buttons[1] = static_cast<unsigned char>((buffer[0] & (1 << 6)) ? VRPN_BUTTON_ON : VRPN_BUTTON_OFF);
        
        
        // here is where we decode the analog stuff
        vrpn_Analog::last[0] = vrpn_Analog::channel[0];
        vrpn_Analog::channel[0] = buffer[0] & ((1 << 6) - 1); // create a mask for the lowest 6 bits to hold the softpot value
        
        
        status = vrpn_ANALOG_REPORT_READY;
        
        report_changes();
        
        status = vrpn_ANALOG_SYNCING;
    }
}

