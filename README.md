# OVR Stylus

![Picture of a user holding the OVR Stylus](Documentation/OVRStylus.png)

The OVR Stylus is an open-source software/hardware design for a 3D input stylus used in virtual and augmented reality applications. With a touch pad, two input buttons, LRA-based vibrotactile haptic feedback, and Bluetooth communication, the 3D-printed prop’s light-weight design (35g) enables creative and precise interaction.

A paper describing the implementation was published at the [Workshop on Novel Input Devices and Interaction Techniques](https://sites.google.com/view/nidit) at the IEEE Virtual Reality 2020 conference. 

This repository archives the files needed to create your own OVR Stylus or to create a modified design.

## Repository Contents

* **/Case** - 3D printing files for the stylus case (.stl, Fusion 360 .f3z)
* **/Documentation** - A copy of the NIDIT paper (.pdf)
* **/Firmware** - Arduino  code used in the stylus and central device
* **/Hardware** - Eagle design files, BOM, Gerber files (.brd, .sch, etc.)
* **/VRPN** - Patch file and stylus classes to integrate with the [VRPN library](https://github.com/vrpn/vrpn)

### Hardware Manufacture

![Stylus Components](Documentation/stylus-pcb.png)

#### PCB

The hardware directory contains the design files needed to print the custom circuit board. The gerber files can be uploaded directly to manufactures (e.g. [Seeed Fusion Studio](https://www.seeedstudio.com/fusion.html)). Ordering specifications including board thickness and other properties are listed in the `PCB Specs - Ordering Info` file. The component placement list and assembly drawing can be used for pick-n-place machines if using professional assembly.

To modify the design the Autodesk Eagle design files for the schematic and board layout are also included.

#### Haptic Controller

The design uses the Fyber Labs [LRA Haptic Flex Module](https://www.tindie.com/products/fyberlabs/lra-haptic-flex-module/). A Samsung 8mm LRA motor can be purchased from the same website. Solder the motor leads to the +0 and -0 through-hole pads. Connect the PWM pad to the stylus's P3 pad. The VIN, GND, SDA, and SCL pads can be connected to the respective pads on the stylus. I recommend using 28 gauge stranded wire and making the leads very short. The haptic flex module is placed vertically at the button end of the stylus. 

#### Sliding Touch Sensor

To capture sliding touch input, a [Spectra Symbol 50mm Linear ThinPot poteniometer](https://www.amazon.com/SPECTRA-SYMBOLLINEAR-THINPOT-50-MM/dp/B005T844FU) is mounted in the groove in the top of the case. Solder leads to the potentiometer pins as described [here](https://learn.sparkfun.com/tutorials/softpot-hookup-guide/all). The 3V3 and GND leads can be kept shorter and be soldered to their respective pads on the stylus pcb near the RX and TX pads. The analog output tab's lead should be kept longer and soldered to the P4 pad at the front of the stylus (It will need to route behind the battery's JST connector). This P4 pad already contains a buildin 10kΩ resistor between the pad and ground that pulls the SoftPot's analog output signal down.

Once the soldering is complete, the potentimeter can be slide sideways into the groove in the top of the stylus case and the adhesive backing can be removed to secure it in place.

#### Battery

The case is designed to fit a [Sparkfun 110mAh Lithium ion battery](https://www.sparkfun.com/products/13853). The battery is mounted in the case underneath the PCB. The wires can either be routed to the front of the PCB and around the buttons to connect to the JST connector, or through the second large hole in the PCB.
    
### 3D Printed Case

The Case folder contains the STL files for the case. Multiple optical tracking end caps are included if multiple styluses will be made for use with the same optical tracking system.

For the prototype, the files were printed with a Makerbot Replicator 5th gen, although more professional results can be obtained with a higher quality resin printer. The prints were made with the standard settings in Makerbot Print, using support material. The top and bottom are designed to fit oriented vertically on the build platform. The support material will need to be removed with a razor blade. The button file will need to be printed twice for the two buttons.

The threading for both endcaps is ISO metric M14x2 profile. Depending on the quality of your printer, the threads may need to be cleaned up using a corresponding bolt and nut. Run the bolt a few times into the endcap to remove any extra plastic from the 3D printing.

The bottom component contains an ANSI 4-40 UNC threaded hole in the center support. This hole may also need the threads to be cleaned up using a steel bolt.

To assemble the components, mount the battery in the slot in the stylus bottom. Route the wires around the center support to the front of the stylus and the top of the board. The haptic module should sit vertically in the  space at the front. The LRA can be hot glued in the smaller slot at the front (The tabs may need to be bent slightly to clear the top of the case). The PCB can be screwed to the center support with a 1/4 inch 4-40 bolt. Place the 3D printed buttons so that they protrude from the top of the stylus, and snap the top and bottom together, making sure the buttons remain in place.  Screw on the end caps.

If the case needs to be modified, the Autodesk Fusion 360 design files are included in the Case directory.

### Software

The following section outlines the software requirements for both the stylus and the central device connected to the host computer.

#### Stylus Software

The stylus code is written for the Arduino environment. Download and install the [Arduino IDE](https://www.arduino.cc/en/Main/Software). The main stylus code is found in Firmware/Stylus.ino. To get started, open the file in the Arduino IDE.

To begin controling the LRA Haptic flex chip, you will need to install the Adafruit_DRV2605 Library. You can do that by going to the Arduino library manager under Sketch -> Include Library -> Manage Libraries... Then search for DRV2605 and find the Adafruit DRV2605 Library and click Install.

The stylus code uses the Pfod low-power Arduino Library by Mathew Ford. A tutorial for installing this software can be found [here](https://www.forward.com.au/pfod/BLE/LowPower/index.html). Below, we copy the most pertinent steps for archival purposes. 

Install [Sandeep Minstry's nRF5 library](https://github.com/sandeepmistry/arduino-nRF5):
1. Go into File → Preferences
2. Add `https://sandeepmistry.github.io/arduino-nRF5/package_nRF5_boards_index.json` as an "Additional Board Manager URL"
3. Open the Boards Manager from the Tools -> Board menu and install "Nordic Semiconductor nRF5 Boards".

Add the nRF5 Flash Softdevice tool:
1. In a terminal, cd <SKETCHBOOK>, where <SKETCHBOOK> is your Arduino Sketch folder:
OS X: ~/Documents/Arduino
Linux: ~/Arduino
Windows: ~/Documents/Arduino
2. Create the following directories: tools/nRF5FlashSoftDevice/tool/
3. Download nRF5FlashSoftDevice.jar to <SKETCHBOOK>/tools/nRF5FlashSoftDevice/tool/ (a local copy of nRF5FlashSoftDevice.jar is here)
4. Restart the Arduino IDE

Install the pfod_lp_nrf52 hardware support.
1. Download the [pfod_lp_nrf52.zip](https://www.forward.com.au/pfod/BLE/LowPower/pfod_lp_nrf52.zip) file. (A local copy can also be found in the Firmware directory.)
2. Start the Arduino IDE, Open the File → Preferences window an at the bottom find the directory where the preferences.txt file is stored. In Window's 7 you can click on that path to open the directory in the Explorer.
3. From the preferences.txt directory, open the packages sub-directory and then the sandeepmistry sub-directory which contains the hardware and tools directories.
4. Delete the hardware directory.
5. Unzip pfod_lp_nrf52.zip to the sandeepmistry directory to install the pfod low power support. This will install the modified hardware directory.
6. Close and restart the Arduino IDE.
7. Open the Tools → Board and scroll down to find the pfod low power nRF52832 boards.
8. Select the `*Redbear BLE Nano 2` board.
9. Under the Tools menu, make sure the programmer is set to `CMSIS-DAP`.

To Program the board, we use a [Particle Debugger](https://store.particle.io/products/particle-debugger). Connect the debugger's GND, SCLK, and SDIO pins to the corresponding pins at the front of the stylus PCB. If you intend to debug frequently, placing Pogo pins in a breadboard allow you to temporarly create connections with the stylus pcb easily without soldering.

Power the stylus pcb and then plug the particle debugger into a usb port on the programming computer. In the tools menu, make sure the usb is selected as the port.

Before the program can be loaded a Nordic Softdevice must be uploaded to the board. Select the `nRF52 Flash SoftDevice` option half way down the Tools menu. The project uses the S132 version 2.0.1 soft device. It is already included with the Sandeep Minstry package.

Once the flash is complete, upload the Stylus sketch onto the stylus pcb.

#### Host Central Device

Multiple BLE-supporting hardware can be used to connect with the stylus. Our implementation uses a stock [Arduino Nano 33 BLE](https://store.arduino.cc/usa/nano-33-ble) module to serve as a BLE to serial repeater. 

Load the Software:
1. Open the Boards Manager from the Tools -> Board menu and install "Arduino nRF528x Boards (Mbed OS)".
2. Select  `Arduino Nano 33 BLE` as the current board and make sure the port corresponds with the USB connection.
3. Upload the nano33_central.ino from the Firmware Directory to the Nano board.

#### VRPN

The VRPN directory contains a patch file to add the OVR Stylus files to the [VRPN library](https://github.com/vrpn/vrpn). The patch was made against commit: 3863b1e089b3070ae37e25fa4a85ab3bb138ee12

After cloning the vrpn library, apply the patch:
```
git apply vrpn_ovrstylus_changes.patch
```

The Stylus device files are also included in the VRPN directory if you would like to manually apply them.
