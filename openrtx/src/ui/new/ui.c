#include "ui/ui_new.h"

/***

Current menu structure:

- Banks
  - All channels
  - (banks...)
- Channels
  - (channels...)
- Contacts
  - (contacts...)
- GPS
  - GPS UI Screen
- Settings
  - Display
    - Brightness                 (0-100,5)
    - Timer                      (Enum)
    - Battery Icon               (Bool)
  - GPS
    - GPS Enabled                (Bool)
    - GPS Set Time               (Bool)
    - UTC Timezone               (-Inf-Inf,0.5)
  - Radio
    - Offset                     (Input)
    - Direction                  (Enum)
    - Step                       (Enum)
  - M17
    - Callsign                   (Input)
    - CAN                        (0-15,1)
    - CAN RX Check               (Bool)
  - FM
    - CTCSS Tone                 (Enum)
    - CTCSS En.                  (Enum)
  - Accessibility
    - Macro Latch                (Bool)
    - Voice                      (Bool)
    - Phonetic                   (Bool)
  - Default Settings
    - Are you sure UI
- Info
    - (commit ver string)
    - Bat. Voltage
    - Bat. Charge
    - RSSI
    - Used heap
    - Band
    - VHF
    - UHF
    - Hw Version
- About
    - About Screen UI
 */