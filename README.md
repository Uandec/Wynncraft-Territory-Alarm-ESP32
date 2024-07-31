# Wynncraft Territory lost Alarm
This is an ESP32 project using the Arduino framework, that makes Wynncraft API calls to detect if a territory has been lost for a specific guild.

## Used Libraries
[Arduino TickTwo](https://github.com/sstaub/TickTwo)
[Arduino_GFX](https://github.com/moononournation/Arduino_GFX)

## Used Materials
- Breadboard + Jumper wires
- ESP32 NodeMCU-32S
- SSD1331 OLED SPI 0.95" 96x64
- 3v Active Buzzer
- Normally Open Button

## Connection Diagram
| NodeMCU-32S | SSD1331 |
| --- | --- |
| GND | GND |
| 3v3 | VCC |
| 18 | SCL |
| 23 | SDA |
| 12 | RES |
| 14 | DC |
| 5 | CS |

| NodeMCU-32S | Buzzer |
| --- | --- |
| GND | GND |
| 33 | + |

\+ Normaly Open Button between Pin 32 and GND

### Features
- Flexible display support, if it is supported by the GFX library, it will most likely run here:tm:
- Display the territory that was lost in the screen and which guild took it
- Sound alert when a territory is lost
- Button to wake display or keep it alive for longer
- WIFI connection/reconnection feature
- Alert when WIFI connection has been lost or an API failure occured

## How to Build
### Using Visual Studio Code
- Download the **Official Arduino** extension for **VSCode** and all the recuired extensions
- Download and install the specific board drivers that you need (should be available in the Board Manager)
- Download and install the Libraries mentioned above (download their release from the GitHub and extract them in \Documents\Arduino\libraries DO NOT USE THE Library Manager)
- Clone the Repository
- Open the repository as a folder in VSCode and wait for it to compile
- Open the ```.ino``` file and wait for it to compile
- Open ```Config.h```, modify the settings and save
- Plug your board and make sure it is ready to be flashed
- Make sure the correct board and Port is selected (you don't need to select a Programmer)
- Upload the ```.ino``` to the board

### Using The Arduino IDE
- Download and install the specific board drivers that you need (should be available in the Board Manager)
- Download and install the Libraries mentioned above (download their release from the GitHub and extract them in \Documents\Arduino\libraries DO NOT USE THE Library Manager)
- Clone the Repository
- Open ```Config.h``` in a text editor, modify the settings and save
- Open the ```.ino``` file and wait for it to compile
- Plug your board and make sure it is ready to be flashed
- Make sure the correct board and Port is selected (you don't need to select a Programmer)
- Upload the sketch

## FAQ
### Why is there a min update time enforced?
The code does not check the remaining request available, it is used as a protection to not get rate-limited. Also every iteration of the code takes a few seconds so the update will not happen that much sooner.
### How do I change the code to use another board or display?
- Make sure the new board or any extra library is properly installed for your IDE
- Open ```UI.hpp```
- Follow the instructions in the GFX library documentation and update these 2 lines of code according to your hardware:
```
Arduino_DataBus *m_bus = new Arduino_ESP32SPI(14, 5, 18, 23, GFX_NOT_DEFINED, VSPI); // DC, CS, SCK, MOSI, MISO, spi_num
Arduino_GFX *m_gfx = new Arduino_SSD1331(m_bus, 12, 0);                              // RST, rotation
```
### How do I change the Wake Button Pin?
Open ```esp32_wynn_terr_alarm.ino``` and change the number in the line ```#define WAKE_BUTTON_PIN 32``` to any number suitable
> [!IMPORTANT]
> Make sure the selected Pin can be used as an Interrupt Input. Also if the board can't set that Pin as a PullUp make sure you use a resistor externally to make it a PullUp.
### How do I change the Buzzer Pin?
Open ```UI.hpp``` and change the number in the line ```#define BUZZER_PIN 33``` to any number suitable
> [!IMPORTANT]
> Make sure the selected Pin can be used as an Output.
### I don't want to use a feature how do I disable it?
Honestly the code is structured in a way that, bu just not using the hardware the rest would work (I haven't tested this DO AT YOUR OWN RISK)