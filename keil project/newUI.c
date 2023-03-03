/*
    Features
    ========


        [x] minimum features
            [x] basic persistency
            [x] full UI usability with one bank

        [x] display version string at boot

        [-] display "edit" when editing a preset
						Tried this but didn't look very good

        [ ] improve persistency (rotating EEPROM)

          [x] refactor to include multiple banks
                    The button "Store" advances banks when
                    on play mode. There are 10 banks, so
                    a total of 50 presets can be stored.

          [x] "simple" mode (each FS drives a loop)

          [-] try disabling restoreLastUsedBankAndPresetFromEEPROM()
                    to see if the old method of "non-volatile ram"
                    can be used.
                    Tried this and it doesn't seem to work, they
                    are still zeroed-out at boot.

          [x] implement the "hold for 1 second" for
                    momentary operation, which rolls back to the
                    last preset after releasing the footswitch.
          
          [ ] implement up to 20 banks (use the dot on the 7-segment
                    display to indicate whether it's in banks 0-9
                    or 10-19.
                    
          [ ] implement "exclusive simple" mode, where the loops are
               toggled as if it was a "solo" mode (when a loop is engaged
               the rest as automatically turned off). This is useful
               for comparing effects, for example.
               NOTE: this can actually be done with programming as well.

          [x] implement "blinking of 7-segment" while on "momentary"
               mode, to let the user know that as soon as he releases
               the footswitch, the pedalboard is going back to the previous
               preset. Note: done using the "dot" in the 7-segment display,
               instead of on the number, which didn't seem to make as
               much sense (since it has nothing to do with the bank).

          [x] turn off all relays while in tuner mode.

          [x] reverse the logic of the EEPROM so that empty banks are by
               default set to all-relays-off, instead of all-relays-on.



    i/o definitions
    ===============

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
*/


#include "mcu_definitions.h"
//#include "uart.h"
#include "eeprom.h"

/*
    The logic on the Relays and the 7-segment display is reversed (a zero turns it on).
    The logic on the footswitch LEDs is not reversed.
    The logic on the Edit and Store buttons is reversed.
*/

#define PIN_RELAY_0 P23
#define PIN_RELAY_1 P22
#define PIN_RELAY_2 P21
#define PIN_RELAY_3 P20
#define PIN_RELAY_4 P44

#define PIN_LED_FS_1 P33
#define PIN_LED_FS_2 P34
#define PIN_LED_FS_3 P35
#define PIN_LED_FS_4 P36
#define PIN_LED_FS_5 P37

#define PIN_BUTTON_EDIT P17
#define PIN_BUTTON_STORE P16

#define PIN_BUTTON_FS0 P12
#define PIN_BUTTON_FS1 P46
#define PIN_BUTTON_FS2 P41
#define PIN_BUTTON_FS3 P45
#define PIN_BUTTON_FS4 P10
#define PIN_BUTTON_FS5 P11

// --

#define BANKS_COUNT 10
#define PRESETS_PER_BANK 5
#define LOOP_RELAYS_COUNT 4

#define STATE_TUNER 0
#define STATE_EDIT  1
#define STATE_PLAY  2

#define EVENT_INVALID -1
#define EVENT_FS_0     0
#define EVENT_FS_1     1
#define EVENT_FS_2     2
#define EVENT_FS_3     3
#define EVENT_FS_4     4
#define EVENT_EDIT     5
#define EVENT_STORE    6
#define EVENT_TUNER    7

#define RELAY_INDEX_LOOP_0 0
#define RELAY_INDEX_LOOP_1 1
#define RELAY_INDEX_LOOP_2 2
#define RELAY_INDEX_LOOP_3 3
#define RELAY_INDEX_MUTE 4

typedef unsigned char UBYTE;

#define VERSION_STRING "0.5"
const unsigned char SEGMENT_DISPLAY_DIGITS[] = {127, 192, 249, 164, 176, 153, 146, 130, 248, 128, 144};


// ---- Global variables -----------------------------------------

// Using this, all loops are turned off during tuning mode. The disadvantage is that
// the user won't be able to see the loops that belong to that preset before going
// out of the mute state
#define TURN_OFF_LOOPS_DURING_TUNING

volatile UBYTE _shouldBlinkSegmentDisplay = 0;

// Either EDIT, PLAY or TUNER
volatile UBYTE _currentState;

// An index between 0 and PRESETS_PER_BANK-1
volatile UBYTE _currentPresetIndex;
volatile UBYTE _currentBankIndex;
volatile UBYTE _previousPresetIndex;

// The actual array containing all the presets
volatile UBYTE _bankPresets[BANKS_COUNT][PRESETS_PER_BANK];


// ---- Functions ------------------------------------------------


void delay100ms() // Approximate only
{
    unsigned char i, j, k;
    i = 2;
    j = 43;
    k = 207;
    do
    {
        do
        {
            while (--k);
        } while (--j);
    } while (--i);
}

void set7SegmentDisplay(UBYTE segments)
{
    P0 = segments;
}

void clearSegmentDisplay(void)
{
	P0 = 0xFF;
}

void set7SegmentDisplayDot(UBYTE dotState)
{
     P0 = (P0 & 0x7F) | ((!dotState) << 7);
}

void displayVersion(void)
{
	UBYTE i, j;
	for (i = 0; i < sizeof(VERSION_STRING / sizeof(char)); i++)
	{
		if (VERSION_STRING[i] != '.' && (VERSION_STRING[i] < 48 || VERSION_STRING[i] > 57))
		{
			continue;
		}

		if (VERSION_STRING[i] == '.')
		{
			set7SegmentDisplay(SEGMENT_DISPLAY_DIGITS[0]);
		}
		else
		{
			set7SegmentDisplay(SEGMENT_DISPLAY_DIGITS[VERSION_STRING[i] - 48 + 1]);
		}
		
		for (j = 0; j < 5; j++)
		{
			delay100ms();
		}
		clearSegmentDisplay();
		delay100ms();
	}
}

// Save the presets in RAM to the non-volatile storage
void storePresetsInRAMtoEEPROM(void)
{
    UBYTE bank, preset;

    // Naive solution: clear page and write all values as new
    IapEraseSector(IAP_ADDRESS);
    delay100ms();
    for (bank = 0; bank < BANKS_COUNT; bank++)
		{
			for (preset = 0; preset < PRESETS_PER_BANK; preset++)
			{
					IapProgramByte(IAP_ADDRESS + preset + (bank * PRESETS_PER_BANK), ~_bankPresets[bank][preset]);
			}
		}
}

// Read the contents of the non-voltatile storage and set them to RAM
void restorePresetsFromEEPROMtoRAM(void)
{
    // NOTE: Each preset is a byte, where the lower 4 bits are the 
    // setting of the relays (which loops are turned on / off)

    UBYTE preset, bank;
    for (bank = 0; bank < BANKS_COUNT; bank++)
		{
			for (preset = 0; preset < PRESETS_PER_BANK; preset++)
			{
					_bankPresets[bank][preset] = ~IapReadByte(IAP_ADDRESS + preset + (bank * PRESETS_PER_BANK));
			}
		}
}

void restoreLastUsedBankAndPresetFromEEPROM(void)
{
    // TODO: what is the best way to recall the bank/preset without writting to 
		// the eeprom every single time ... ? is there a brown-out detection?
    _currentPresetIndex = 0;
    _currentBankIndex = 0;
		_previousPresetIndex = 0;
}

void storeLastUsedBankAndPresetToEEPROM(void)
{
    // TODO: complete this ...
    // EEPROM <--- _currentPresetIndex
}

void setRelay(UBYTE relayIndex, UBYTE relayState)
{
    // The logic of the relays is reversed
    relayState = relayState == 0 ? 1 : 0;
    if (relayIndex == 0)
    {
        PIN_RELAY_0 = relayState;
    }
    else if (relayIndex == 1)
    {
        PIN_RELAY_1 = relayState;
    }
    else if (relayIndex == 2)
    {
        PIN_RELAY_2 = relayState;
    }
    else if (relayIndex == 3)
    {
        PIN_RELAY_3 = relayState;
    }
    else if (relayIndex == 4)
    {
        PIN_RELAY_4 = relayState;
    }
}

void setFS_LED(UBYTE LEDIndex, UBYTE LEDstate)
{
    if (LEDstate < 0 || LEDstate >= PRESETS_PER_BANK)
    {
        return;
    }

    if (LEDIndex == 0)
    {
        PIN_LED_FS_1 = LEDstate;
    }
    else if (LEDIndex == 1)
    {
        PIN_LED_FS_2 = LEDstate;
    }
    else if (LEDIndex == 2)
    {
        PIN_LED_FS_3 = LEDstate;
    }
    else if (LEDIndex == 3)
    {
        PIN_LED_FS_4 = LEDstate;
    }
    else if (LEDIndex == 4)
    {
        PIN_LED_FS_5 = LEDstate;
    }
}

void recallPreset(UBYTE presetIndex)
{
    UBYTE i;

    if (presetIndex < 0)
    {
        presetIndex = 0;
    }
    else if (presetIndex >= PRESETS_PER_BANK)
    {
        presetIndex = PRESETS_PER_BANK - 1;
    }

    _currentPresetIndex = presetIndex;

    for (i = 0; i < LOOP_RELAYS_COUNT; i++)
    {
        setRelay(RELAY_INDEX_LOOP_0 + i, (_bankPresets[_currentBankIndex][presetIndex] & (1 << i)) >> i);
    }
}

void setMuteRelay(UBYTE state)
{
    setRelay(RELAY_INDEX_MUTE, state);
}

UBYTE areAnyButtonsPushed(void)
{
	if      (!PIN_BUTTON_EDIT)  { return EVENT_EDIT;  }
	else if (!PIN_BUTTON_STORE) { return EVENT_STORE; }
	else if (!PIN_BUTTON_FS0)   { return EVENT_TUNER;  }
	else if (!PIN_BUTTON_FS1)   { return EVENT_FS_0;  }
	else if (!PIN_BUTTON_FS2)   { return EVENT_FS_1;  }
	else if (!PIN_BUTTON_FS3)   { return EVENT_FS_2;  }
	else if (!PIN_BUTTON_FS4)   { return EVENT_FS_3;  }
	else if (!PIN_BUTTON_FS5)   { return EVENT_FS_4;  }

	return -1;
}

UBYTE waitForEvent(void)
{
    UBYTE event = EVENT_INVALID;

   while(1)
   {
     event = areAnyButtonsPushed();
     if (event != -1)
     {
          return event;
     }
   }
}

void waitForAllButtonsRelease(void)
{
    // Initial delay to make sure the pins are stable before checking
    delay100ms();

    // While any button is pressed, wait for a about 100 ms
    while (areAnyButtonsPushed() != -1)
    {
        delay100ms();
    }
}

void setBank(UBYTE bankIndex)
{
	if (bankIndex < 0 || bankIndex >= BANKS_COUNT)
	{
		bankIndex = 0;
	}
	_currentBankIndex = bankIndex;
	set7SegmentDisplay(SEGMENT_DISPLAY_DIGITS[_currentBankIndex + 1]);
}

void mainLoop(void)
{
    UBYTE event, i;

    // Should only break out from this loop if an unrecoverable error
    // happens. Returning from this function will trigger a software reboot.
    while(1)
    {
        waitForAllButtonsRelease();
        event = waitForEvent();

        if (_currentState == STATE_TUNER)
        {
               // ------- START "TUNER" ----------------------------------------------------------------
            if (event == EVENT_TUNER)
            {
                // Recall the current preset (no changes), un-mute and leave tuner mode
                recallPreset(_currentPresetIndex);
                setMuteRelay(0);
                _currentState = STATE_PLAY;
            }
            else if (event >= EVENT_FS_0 && event <= EVENT_FS_4)
            {
                // Go to the selected preset, un-mute and leave tuner mode
                recallPreset(event);
                setMuteRelay(0);
                _currentState = STATE_PLAY;
            }
                // ------- END "TUNER" ----------------------------------------------------------------
        }
        else if (_currentState == STATE_EDIT)
        {
            // ------- START "EDIT" ----------------------------------------------------------------
            if (event == EVENT_STORE)
            {
                // Store current preset to EEPROM and go back to play mode
                storePresetsInRAMtoEEPROM();
                _currentState = STATE_PLAY;
                recallPreset(_currentPresetIndex);
            }
            else if (event == EVENT_EDIT)
            {
                // Recall the current preset from EEPROM, so "discard changes"
                restorePresetsFromEEPROMtoRAM();
            }
            else if (event >= EVENT_FS_0 && event <= EVENT_FS_4)
            {
                // Edit this preset with the corresponding "loop relay" change
                // (toggle the "n-th" bit according to the event)
                _bankPresets[_currentBankIndex][_currentPresetIndex] ^= 1UL << event;
                recallPreset(_currentPresetIndex);
            }
                // ------- END "EDIT" ------------------------------------------------------------------
        }
        else if (_currentState == STATE_PLAY)
        {
          // ------- START "PLAY" ----------------------------------------------------------------
            if (event == EVENT_TUNER)
            {
               // Enter tuner mode
               setMuteRelay(1);

#ifdef TURN_OFF_LOOPS_DURING_TUNING
               // Turn off all loops for quieter output during mute state
               for(i = 0; i < LOOP_RELAYS_COUNT; i++)
               {
                    setRelay(i, 0);
               }
#endif
               _currentState = STATE_TUNER;

            }
            else if (event >= EVENT_FS_0 && event <= EVENT_FS_4)
            {
               if (_currentPresetIndex == event)
               {
                    // It's the same preset, nothing else to do
               }
               else
               {
                    // Keep a copy of the preset we're about to leave, in case we need to return to it
                    _previousPresetIndex = _currentPresetIndex;
                    _currentPresetIndex = event;

                    // Go to the new preset
                    recallPreset(_currentPresetIndex);
                    storeLastUsedBankAndPresetToEEPROM();

                    // Is the FS being held down?
                    for(i = 0; i < 10; i++)
                    {
                         delay100ms();
                         delay100ms();
                         if(areAnyButtonsPushed() != _currentPresetIndex)
                         {
                              break;
                         }
                    }
                    if (i == 10)
                    {
                         // Footswitch is being held down, wait for its release and switch back
                         // to the last preset
                         _shouldBlinkSegmentDisplay = 1;
                         waitForAllButtonsRelease();
                         _currentPresetIndex = _previousPresetIndex;
                         recallPreset(_currentPresetIndex);
                         storeLastUsedBankAndPresetToEEPROM();
                         _shouldBlinkSegmentDisplay = 0;
                    }
               }
            }
            else if (event == EVENT_EDIT)
            {
                _currentState = STATE_EDIT;
            }
						else if (event == EVENT_STORE)
						{
							_currentBankIndex++;
							if (_currentBankIndex >= BANKS_COUNT)
							{
								_currentBankIndex = 0;
							}
							setBank(_currentBankIndex);
							recallPreset(_currentPresetIndex);
						}
        }
    }
}

volatile UBYTE _blinkingState = 0;

UBYTE checkSimpleMode(void)
{
    // If "Edit" and "Store" are held down during boot, it boots to "simple" mode, without programs
    // where each FS controls a loop
    delay100ms();
    if (!PIN_BUTTON_EDIT && !PIN_BUTTON_STORE)
    {
        return 1;
    }
    return 0;
}

void enterSimpleMode(void)
{
    UBYTE event;
    UBYTE i;
    volatile UBYTE loops[LOOP_RELAYS_COUNT];
    volatile UBYTE tunerStatus = 1;

    // Display "S" on the 7-segment
    // set7SegmentDisplay(0x92); // 0b10010010

    // Set all loops OFF
    for (i = 0; i < LOOP_RELAYS_COUNT; i++)
    {
        loops[i] = 0;
        setRelay(i, 0);
				setFS_LED(i, 0);
    }
		setFS_LED(i, 0);

    // Simple mode
    while(1)
    {
        waitForAllButtonsRelease();
        event = waitForEvent();
        switch(event)
        {
        case EVENT_FS_0:
        case EVENT_FS_1:
        case EVENT_FS_2:
        case EVENT_FS_3:
            loops[event] = !loops[event];
            setRelay(event, loops[event]);
						setFS_LED(event, loops[event]);
            break;

        case EVENT_TUNER:
						tunerStatus = tunerStatus == 0 ? 1 : 0;
            setMuteRelay(tunerStatus);
            break;

        default:
            break;

        }
    }
}

volatile unsigned int _timerCounter = 0;
void tm0_isr() interrupt 1
{
     UBYTE i;
     TL0 = 0xFFFF;
     TH0 = 0xFFFF;

     _timerCounter++;
     if (_timerCounter == 1000)
     {
          _timerCounter = 0;
          _blinkingState = _blinkingState == 0 ? 1 : 0;

          // The dot in the 7-segment display can be blinking (during "momentary mode") or off
          set7SegmentDisplayDot(_shouldBlinkSegmentDisplay && _blinkingState);

          // If it's in EDIT or TUNER mode, blink the selected preset
          for(i = 0; i < PRESETS_PER_BANK; i++)
          {
               if (_currentState == STATE_TUNER || _currentState == STATE_EDIT)
               {
                    setFS_LED(i, _blinkingState && (i == _currentPresetIndex));
               }
               else if (_currentState == STATE_PLAY)
               {
                    setFS_LED(i, (i == _currentPresetIndex));
               }
          }
     }
}

void setupTimer(void)
{
		// Set up the timer	
    TMOD = 0x01; // set timer0 as mode1 (16-bit)
    TL0 = 0xFF;  // initial timer0 low byte
    TH0 = 0xFF;  // initial timer0 high byte
    TR0 = 1;     // timer0 start running
    ET0 = 1;     // enable timer0 interrupt
    EA = 1;      // open global interrupt switch
}

// This is called when the CPU is ready to run, to set up all necessary
// i/o stuff and other things
void startup(void)
{
#ifdef TURN_OFF_LOOPS_DURING_TUNING
     UBYTE i;
#endif
     
     setMuteRelay(1);
     displayVersion();
     if (checkSimpleMode())
     {
          enterSimpleMode();
     }
     setupTimer();
     restorePresetsFromEEPROMtoRAM();
     restoreLastUsedBankAndPresetFromEEPROM();
     setBank(_currentBankIndex);
     recallPreset(_currentPresetIndex);

#ifdef TURN_OFF_LOOPS_DURING_TUNING
     // Turn off all loops for quieter output during mute state
     for(i = 0; i < LOOP_RELAYS_COUNT; i++)
     {
          setRelay(i, 0);
     }
#endif

     _currentState = STATE_TUNER;
}

// ------------------------------------------------------------------

void mainUI(void)
{
    while(1)
    {
        startup();
        mainLoop();

        // It should not reach here, and if it happens, restart
        // by running the startup sequence again and hope it all
        // goes back to work.
    }
}
