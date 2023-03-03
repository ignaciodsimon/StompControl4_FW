# StompControl4_FW
Replacement firmware for the Harley Benton StompControl-4 pedal loop switcher

![alt text](https://github.com/ignaciodsimon/StompControl4_FW/raw/main/reverse%20engineering%20the%20board/switcher%20working%20with%20new%20firmware.jpg)

## Description

This is a (fully written from scratch) replacement firmware for the pedal switcher sold by Thomann under the brand Harley Benton (this is a OEM product manufactured by Vitoos, so it's probably found under other brand names). The source code is completely original (not obtained from the manufacturer in any way), thus there are no intellectual property rights protecting it. You are welcome to use it, modify it, etc, as the license document specifies.

The original product description can be found here: 
https://harleybenton.com/product/stompcontrol-4-iso/

# Why you would want to install this firmware on your board

This firmware was developed after using the switcher unmodified for a little while and becoming rather unsatisfied with it. The hardware is of reasonably good quality, with a robust all-metal case, okay quality footswitches and a most of the basic features. *The stock firmware just isn't good enough in my opinion.*

## Original firmware features

- 5 banks with 4 presets each
- dedicated footswitch for "tuner"
- dedicated footswitch for "next bank" *(do you really need to switch banks in the middle of a song?)*
- ***bug?: repeatedly pushing any preset footswitch will cycle between that preset and "all off". This can make live performance a lot less reliable.***
- ***bug: coming out of "tuner" mode will display the last used preset as engaged, but will actually be in full bypass. This is just super bad.***

## New firmware features

- 10 banks with 5 presets each (the original "tuner" footswitch is now preset E and the original "bank+" is now "tuner").
- dedicated footswitch for "tuner"
- momentary mode (engaged when holding any footswitch down for over 1.5 seconds), meaning that after releasing it, it will go back to the last preset. This is very useful for short passages where a brief change is required, such as a short solo
- "simple mode", where each footswitch controls one relay, and that's all. This mode can be accessed by holding down the buttons "edit" and "store" while powering up. In this mode, the "Preset-E" footswitch doesn't do anything.
- firmware version displayed when booting up
- all loops are disengaged when in "tuner" mode, for the quietest output while tuning up
- current preset indicator blinks while in tuner mode, to indicate which preset will be recalled after pressing "tuner" again
- the "tuner" mode can be exited also by pressing a preset footswitch

# New firmware design

![alt text](https://raw.githubusercontent.com/ignaciodsimon/StompControl4_FW/main/new%20UI%20specification/new%20UI.jpg)

![alt text](https://github.com/ignaciodsimon/StompControl4_FW/raw/main/new%20UI%20specification/state_machine_v0.3.jpg)


# Technical specifications

## CPU

This switcher is governed by a cheap 8-bit microcontroller made by "STC", a chinese 8051-compatible chip, model STC11F08XE. It runs on its internal oscillator (no external crystal) and is designed for 3.3-5.5 Volts operation. It has 8kB of flash, 512 bytes of ram and 5kB of "EEPROM" (actually flash memory). This board uses the LQFP-44 package.

## Peripherals

There are a total of five relays (four for the actual loop switching and one for the tuner output / mute). The loop LEDs are hardwired to their corresponding relays, thus cannot be driven independently. The LEDs associated with the preset footswitches are independent and can be driven as desired. The footswitches are all of "momentary" operation, meaning that they return to their default position when released. The same goes for the two small push-buttons.

The i/o of the MCU is mapped as follows:

    PIN     i/o       Description
    -----------------------------

    P2.3    input     Relay 0 engages the loop 0.
    P2.2    input     Relay 1 engages the loop 1.
    P2.1    input     Relay 2 engages the loop 2.
    P2.0    input     Relay 3 engages the loop 3.
    P4.4    input     Relay 4 engages the "MUTE" for the tuner output.

    P1.2    input     FS tuner
    P4.6    input     FS preset-0
    P4.1    input     FS preset-1
    P4.5    input     FS preset-2
    P1.0    input     FS preset-3
    P1.1    input     FS preset-4

    P1.7    input     Edit
    P1.6    input     Store

    n/c     output    LED FS tuner
    P3.3    output    LED FS preset-0
    P3.4    output    LED FS preset-1
    P3.5    output    LED FS preset-2
    P3.6    output    LED FS preset-3
    P3.7    output    LED FS preset-4

    P0.0    output    7-segment
    P0.1    output    7-segment
    P0.2    output    7-segment
    P0.3    output    7-segment
    P0.4    output    7-segment
    P0.5    output    7-segment
    P0.6    output    7-segment
    P0.7    output    7-segment

The logic on the relays and the 7-segment display is reversed (a zero turns it on).
The logic on the footswitch LEDs is not reversed.
The logic on the "Edit" and "Store" buttons is reversed.

# How to install this firmware

You will need:
- A computer with Windows (it can be done using Linux and other flashing tools, but I haven't tried it).
- The HEX file of the new firmware, which can be found in this repository, for example "newFW_v0.5.hex".
- A USB-to-serial adapter, which can usually be found online for less than 5 dollars.
- To open your switcher and connect (or solder) four wires.

Steps:
- Open up your switcher and remove the main board (the one with the jacks and relays).
- Locate the programming connector, placed between the microcontroller and the transformer, unlabelled.
- The pinout of the programming connector, from left to right (left being the closest to the microcontroller) are:
  - GND
  - TxD
  - RxD
  - VCC
- Connect this pins to your usb-to-serial adapter, as follows:
  - (switcher) GND --> GND (adapter)
  - (switcher) TxD --> Rx (adapter)
  - (switcher) RxD --> Tx (adapter)
  - (switcher) VCC --> VCC / +5V (adapter) ***(leave this wire unconnected for now)***
- Do not connect your switcher to a power supply
- Launch the flashing tool "stc-isp" and select the right serial port (the one corresponding to your external usb-serial adapter).
- **IMPORTANT: check "Select system clock source" under "HW Option". If you forget to check this, the MCU will be flashed with the fuses to run on external clock, and since the board doesn't have any, it will not run at all. Furthermore, you won't be able to re-flash it until you physically install a crystal directly on the chip.**
- Click the button "Open Code File" and select the HEX file with the new firmware.
- Click the button "Download/Program" and immediately connect the VCC wire to power up your board from the usb-serial adapter. It needs to be powered up after clicking the "Download/Program" because the programming is done by a built-in bootloader that comes in from the factory. If you need to re-flash, just remove the VCC wire, hit the button again, and reconnect the VCC wire.
- Programming should take less than 10 seconds. The "stc-isp" tool will inform you of whether it failed or not.
- If it worked, the board will quickly boot up with the new firmware, displaying its version on the 7-segment display and then going directly to "tuner" mode.

### Additional note

The switcher comes from the factory (at least mine did) already with an indicator LED for the "Preset E" (the right-most footswitch which was the "tuner" before). Perhaps it was going to be a 5-preset board, and later this idea got discarded but the PCB design was not updated. Luckily for us this means that "Preset E" can have an indicator LED just like the other four ones, but you'll need to drill a hole on the top chassis to be able to see the LED. Additionally I used a bit of hot glue to improvise a simple "light-guide" for better visualization.

## Disclaimer

**I take absolutely no responsability for anything either good or bad done with the information / software contained in this repository. If you break your pedal switcher, it's not my responsability.**

**All the information / software contained in this respository has been found freely available on the internet or simply found by looking at the board of the switcher.**

# Modify and build this firmware

This firmware was developed and built using the free version of Keil's uVision IDE, which can be downloaded for free for non-commercial uses here: https://www.keil.com/demo/eval/c51.htm. There are some limitations in the free version, but it's more than enough for this project.

A tool needed to install the CPU definitions, as well as for flashing can be downloaded from the manufacturer's site (which at least for me worked extremely bad). I used version "stc-isp-v6.91B.exe". You may be able to download this and/or newer versions here: https://www.stcmicro.com/rjxz.html

### Simplified steps:

- Download and install Keil uVision (freely available from their site)
- Download and launch "stc-isp"
  - Select the right MCU type on the combobox in the top-left corner (STC11F08XE)
  - Go to the tab "Keil ICE Settings"
  - Click the button "Add MCU type to Keil"
  - Select the path to your Keil library, for example: C:\Keil_v5\UV4
- Launch Keil uVision and load the project file (the entire project can be found in this repository)
- Build it (press F7)
- If no errors are found, the new HEX file will be produced to \Objects\newFW.hex
- Follow the steps previously described in this document to flash your newly built firmware onto your board.
