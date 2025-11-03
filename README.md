# HiWonder LeArm Open Source Issues & Troubleshooting

![LeArm 6DOF Robot Arm](reference-materials/servos.png)

> <span style="color:green">SUCCESS: </span><br>
> This project has been a success, see [LeArm-ESP32-Factory-Firmware/README.md](LeArm-ESP32-Factory-Firmware/README.md). Had to overcome some hardware and software hurdles. 

This GitHub repository is my personal attempt to get the **HiWonder LeArm Robotic Arm Kit (6DOF) Open Source** working directly after purchase.  

- **Product link:** [HiWonder LeArm on Amazon](https://www.amazon.com/dp/B0FJFVV6C7?ref=ppx_yo2ov_dt_b_fed_asin_title&th=1)  
- **Official Wiki:** [HiWonder LeArm Wiki](https://wiki.hiwonder.com/projects/LeArm_AI/en/latest/docs/1.Geting_Ready.html)  
- **Additional Docs (Google Drive):** [Google Repo](https://drive.google.com/drive/folders/1vwAXIoUjviLJAhGvHmiILseAW0N5tj59)
- **Factory Firmware (Google Drive):** [Google Repo](https://drive.google.com/drive/folders/1JufiqiCjYjH1hNJEh8xa6O496BbO982L)
- **Espressif ESP32 Board Manager URL:** [ESP32 Dev Module](https://espressif.github.io/arduino-esp32/package_esp32_index.json)

```
## Versions

Arduino IDE: 2.3.6 # USE ARDUINO CLOUD EDITOR INSTEAD (Install board manager from link above)
ESP32 by Espressif Systems: 3.3.2

```

### Servo Information

| Servo        | Axis | Rotation Speed        | Stall Torque                | Rotation Range | Servo Accuracy |
|---------------|------|----------------------|-----------------------------|----------------|----------------|
| LDX-218       | 4, 5    | 0.16sec / 60° / 7.4v | 15kg.cm / 6v <br> 17kg.cm / 7.4v | 0-180°         | 0.3°           |
| LD-1501 MG    | 6    | 0.16sec / 60° / 7.4v | 15kg.cm / 6v <br> 17kg.cm / 7.4v | 0-180°         | 0.3°           |
| LDX-335 MG    | 1    | 0.18sec / 60° / 7.4v | 20kg.cm / 7.4v               | 0-180°         | 0.3°           |
| LFD-06 MG     | 2, 3    | 0.25sec / 60° / 7.4v | 6kg.cm / 7.4v                | 0-180°         | 0.3°           |

Source: [A02-Controller.pdf](reference-materials/A02-Controller.pdf)

---

## Background

I purchased this kit to explore Arduino-based robotics and STEM projects. After carefully assembling it in a little over **3.5-4 hours** and triple-checking every step, I plugged it in annnnnnd **nothing worked**.  Immediately after power up it was supposed to go to a home position, etc. 

![LeArm iPhone Error: The robot is not currently in app control mode](reference-materials/iphone-error.PNG)

- I tried the iPhone app, it told me “LeArm is currently not in app control mode. Please restart the robot and switch to app control mode before proceeding!”
- I tried the iPad app, it told me “LeArm is currently not in app control mode. Please restart the robot and switch to app control mode before proceeding!”
- I tried the Playstion 2 (PS2) Controller that came in the box, confirmed connection with solid green lights (according to documentation), but it totally ignored anything I pressed.
- I tried the LeArm.exe Windows Desktop app and same message as 1 and 2, the robot was not in a certain control mode. Please restart.
- I tried Arduino IDE, and keep getting different errors about the built in open source libraries not being found or using old function/variable references and the robot/Espressif ESP32-WROOM32D board being completely unresponsive.

I'm really trying to make a fun hobby robot project out of an imperfect product.
But it's getting frustrating.

This repo exists to document my efforts to make this kit functional and to gather fixes, improvements, and translations of Chinese resources to English.

---

## Logs and Compiler Fails on Arduino IDE

Through an enormous amount of trial and error, found that something is happening within the Windows Cache and Arduino IDE.
- verified I had the right board & port selected
- verified I had the right libraries included in the script

Despite clearing all files, uninstalling, and reinstalling the IDE, the following error kept showing up.

```
C:\Users\TheKicker\AppData\Local\Temp\.arduinoIDE-unsaved2025101-26284-elnxva.8axvd\sketch\sketch.ino: In function 'void setup()': C:\Users\TheKicker\AppData\Local\Temp\.arduinoIDE-unsaved2025101-26284-elnxva.8axvd\sketch\sketch.ino:4:3: error: 'ledcSetup' was not declared in this scope     
        4 | ledcSetup(0, 1000, 8); 
          | ^~~~~~~~~ 
C:\Users\TheKicker\AppData\Local\Temp\.arduinoIDE-unsaved2025101-26284-elnxva.8axvd\sketch\sketch.ino:5:3: error: 'ledcAttachPin' was not declared in this scope; did you mean 'ledcAttach'? 
        5 | ledcAttachPin(2, 0); 
          | ^~~~~~~~~~~~~ 
          | ledcAttach exit status 1 

Compilation error: 'ledcSetup' was not declared in this scope
```

Since downloading [Arduino Cloud Agent](https://cloud.arduino.cc/download-agent/) and using the Arduino Cloud Editor, I've been able to push the same exact snippet of code to the board allowing a successful compile and upload of the LED SOS INO script.  Additionally, to test functionality I was able to play a simple melody using the controller board buzzer.

---

## Repository Structure (explanation of files)

```
hiwonder-learm-open-source/
├── LeArm-ESP32-Factory-Firmware/   # Factory Firmware via Customer Support
├── led-lights/                     # Code for controlling the LEDs on ESP32
├── libraries/                      # Arduino libraries needed for the arm as I add them
├── README.md                       # This file
└── reference-materials/            # PDFs, 3D models, and electrical drawings
    ├── 3d-models/                  # Not all models included (Github size limit)
    ├── electrical-drawings/
    │   ├── CHINESE-SCH-*.pdf
    │   └── ENGLISH-SCH-*.pdf       # Did my best to translate
    ├── A00-build-instructions.pdf  # Original build instructions 
    ├── A01-introduction.pdf
    ├── A02-controller.pdf
    ├── A03-deviation.pdf
    ├── controller-board.png    # Hi-Definition image For referencing pins
    ├── ESP32.png               # Hi-Definition image For referencing ESP32 Board
    └── servos.png              # Hi-Definition image For referencing servos
```

**Electrical Drawings:**  
I’ve done my best to translate the original Chinese schematics into English to make troubleshooting possible.  

- `CHINESE-SCH-*.pdf` — Original schematics in Chinese  
- `ENGLISH-SCH-*.pdf` — My translated versions  

---

## Goal

The primary goal of this repo is:

- To **get the HiWonder LeArm kit working** by any means necessary.  
- To **translate Chinese documentation** for better accessibility.  
- To create a **community-driven open source effort** for fixes and improvements.  

Contributions are welcome! If you have experience with Arduino, robotics, or can improve translations, please submit a PR.

---

## Notes / Current Status / Learnings

- Kit assembled successfully, all servos and mechanical parts physically installed according to instructions. 
- The kit is mechanically sound, well crafted aluminum parts and heavy base.
- Electronics are all connected according to documentation, but initial tests produce **no response** whatsoever.  
- Documentation and reference material collected for troubleshooting, unfortunately a lot of it is in Chinese which I don't speak.  
- Open to experimenting with code, firmware, and Arduino examples.
- There is some funny business between the Windows caching system and Arduino IDE. Do not use it. Download the Arduino Cloud Agent and use the Arduino Cloud Editor.
- I have found that GPIO Port 27 will make the buzzer work. (found through trial and error, but if you visit [LeArm-ESP32-Factory-Firmware/Arduino-Cloud-Editor/LeArm_ESP32_Arduino_factory250512/Config.h](LeArm-ESP32-Factory-Firmware/Arduino-Cloud-Editor/LeArm_ESP32_Arduino_factory250512/Config.h) you can find all of the associated ports.)
- I have found I can make the LED on the ESP32 board work (LED SOS code included above with a reference to the RMS Titanic)  

---

## Outcome

There were multiple actually. I contacted official support channels (support@hiwonder.com) and received a ZIP folder with the factory default firmware for the LeArm 6DOF robot arm. My gut feeling is that the unit came with completely missing or corrupt firmware, and trying to fix it with the Arduino Desktop IDE was a nightmare. Constant 'multiple definition' errors from Windows file caching and typos/file naming mismatches. After switching to the Arduino Cloud Editor, flattening the factory firmware into a single folder, cleaning up referenced file paths, and a few other things - the firmware finally compiled and uploaded. 

Full details and troubleshooting steps in the link below:<br>
[LeArm-ESP32-Factory-Firmware/README.md](LeArm-ESP32-Factory-Firmware/README.md)

---

**Disclaimer:** This project is experimental. I am doing my best to make a functional robotic arm out of what was initially a non-working purchase. Use at your own risk.  

This experience is mine, and yours may be much better - I fully encourage any and all to grab a desktop robot arm. 

---

"There are an endless number of things to discover about robotics. A lot of it is just too fantastic for people to believe.""

<strong>Daniel H. Wilson</strong>



"Hands-on experience is the best way to learn about all the interdisciplinary aspects of robotics."

<strong>Rodney Brooks</strong>
