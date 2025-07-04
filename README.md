![Cover](https://github.com/KonradWohlfahrt/FocusWheel/blob/main/images/Cover.jpg)

# Focus Wheel
The Focus Wheel is a customizable desk companion designed to help you stay focused, manage study sessions, and take effective breaks. Powered by an ESP32-C3, it features a rotary encoder, buzzer, LED ring, OLED display, and optional slider — all neatly packed into a 3D-printed enclosure.
This project combines hardware and software to provide:
- Multiple focus and break timers
- Visual and audible notifications
- Customizable controls via knob, button, and slider
- Expandability for future features (e.g., media control or BLE functionality) \

Whether you're a student, maker, or just looking to build a functional productivity gadget, the Focus Wheel is a great weekend project — with lots of room for upgrades! Check out the in-depth [guide]() at instructables! \

**Sponsored by PCBWay!**
This project was made possible with the help of [PCBWay](https://www.pcbway.com/) — a reliable PCB manufacturing service offering high-quality boards at great prices. If you’re looking to bring your electronics project to life with professionally fabricated PCBs, check them out!
![Focus Wheel](https://github.com/KonradWohlfahrt/FocusWheel/blob/main/images/FocusWheel_3.jpg)

***
# Materials:
| Component | Amount | Silkscreen label |
|:----------|:------:|-----------------:|
| custom pcb | 1 | - |
| MMBT2222A | 1 | Q1 |
| 10uF 0805 | 3 | C1,C3,C9 |
| 100nF 0805 | 4 | C2,C4,C8,C11 |
| 1uF 0805 | 2 | C5,C10 |
| 47pF 0805 | 2 | C6,C7 |
| 100uF 1206 | 1 | C12 |
| 10k 0805 | 5 | R1,R2,R5,R6,R11 |
| 0r 0805 | 3 | R3,R4,R7 |
| 330r 0805 | 1 | R8 |
| 33r 0805 | 1 | R9 |
| 1k 0805 | 1 | R10 |
| 220k 0805 | 1 | R12 |
| 100k 0805 | 1 | R13 |
| 45mm fader | 1 | RS1 |
| Rotary encoder | 1 | SW1 |
| Tactile switch 12x12mm | 1 | SW2 |
| SMD switch 3x6x5mm | 1 | KEY1 |
| Buzzer 12x9mm | 1 | BUZZER1 |
| USB3.0 A socket | 1 | USB1 |
| ESP32-C3-WROOM-02 | 1 | U1 |
| AMS1117-3.3V | 1 | U2 |
| 0.91" OLED | 1 | U3 |
| threaded heat insert m3 x 4.5mm | 6 | - |
| M3x14mm screw | 4 | - |
| M3x4mm screw | 2 | - |
| USB-A cable (male to male) | 1 | - |

***
# Schematic:
![Schematic](https://github.com/KonradWohlfahrt/FocusWheel/blob/main/images/Schematic_FocusWheel_V1.png)
_Schematic of the Focus Wheel_
![Soldering Lables](https://github.com/KonradWohlfahrt/FocusWheel/blob/main/images/SMD_Soldering.png)
_Lables for soldering components_

***
# Programming
Programmed with the Arduino IDE and ESP32 board manager. 
Install the following libraries:
- [DonutStudioTimer](https://github.com/KonradWohlfahrt/Arduino-Timer-Library)
- [RotaryEncoder](https://github.com/mathertel/RotaryEncoder)
- [FastLED](https://github.com/FastLED/FastLED)
- [U8g2](https://github.com/olikraus/u8g2)

**Uploading:**
Board: `ESP32C3 Dev Module`
Settings: default
Connection via USB cable.
Press the knob button (Flash) when powering up the device, it also seems uploading works without doing so.

***
# 3D Printing
Print `Housing Bottom` and one of the `Housing Top` options - depending on what components you would like to use. Finally, print both `Fading Knob` and `Knob`. Put in the six threaded heat inserts and screw everything together. The knobs slide on top.
![Focus Wheel](https://github.com/KonradWohlfahrt/FocusWheel/blob/main/images/FocusWheel_1.jpg)


***
# Functions
The following button functions are used in the examples:
- middle button: start timer, pause timer, change selection
- rotating middle button: change timer values or timers
- right button: edit timers, skip timers
- long press right button (only `Timer.ino`): save timer to eeprom

![Focus Wheel](https://github.com/KonradWohlfahrt/FocusWheel/blob/main/images/FocusWheel_2.jpg)