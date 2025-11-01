# Factory Firmware

The ZIP drive in this directory is what I received from HiWonder Support (support@hiwonder.com).  It contains the factory default firmware for the LeArm 6DOF robotic arm kit.

Trying to install via Arduino IDE is downright impossible. It is riddled with errors, and if you tried - you may get stuck in an endless loop of debugging like I did from the way Windows is caching files.  Think you fixed one and another pops up. Example output below: 

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

We can verify though that both of these functions (and many others) are defined in their corresponding files. Window's cache is the problem.

## The Problem Statement(s)

- Windows cached old object files, causing multiple missing definition errors.
- The `kinematics` files are not named correctly.  It should be named `Kinematics.cpp` instead of `kinematics.cpp`. Caused dual definitions in compiler. 
- Arduino Cloud Editor flattens everything to the top level directory. Thus all references pointed at folder structure that didn't exist. 
- The robot slammed into the table after uploading the fixed factory firmware, and power cycling.

## The Fix
 Steps Taken to Fix Compilation & Upload Issues

1. **Move away from Arduino Desktop IDE (Windows caching issues)**  
   - The Desktop IDE on Windows sometimes reuses old compiled object files, causing multiple definition errors.  
   - Switched to **Arduino Cloud Editor**, which forces clean builds and handles file paths more consistently.  

2. **Rename `kinematics.hpp` and `kinematics.cpp` to match case**  
   - Did not need to change `kinematics.hpp` → `Kinematics.hpp`, it was already correct.  
   - Changed `kinematics.cpp` → `Kinematics.cpp`  
   - Arduino Cloud Editor and the ESP32 toolchain are **case-sensitive**, so the mismatch caused linker errors.  

3. **Flatten all file references**  
   - Removed for example `../../` and `../src/` references in `#include` statements.  
   - Arduino Cloud Editor moves all files to the top-level folder, so relative paths needed to be eliminated.  

4. **Zip the folder and import into Arduino Cloud Editor**  
   - Created a **project ZIP** containing all the `.ino`, `.cpp`, `.hpp`, and necessary files.  
   - Imported it to the cloud editor to maintain a consistent file structure and clean build.  

5. **Upload to ESP32 Dev Module**  
   - Selected the ESP32 Dev Module as the board. For the Espressif ESP32 WROOM 32D we can use ESP32 Dev Module as the device type when connecting to the board
   - Verified the code compiled and uploaded successfully.  

6. **Move away from the device before running the arm**  
   - The robot arm moves **quickly to its initial position**, so stepping back is crucial for safety.
   - It will return to it's factory default home position. See Fix #7.

7. **The arm slams or moves to a weird home position**  
   - If the servos horn was moved prior to installation, it could be at an incorrect position and have an incorrect home position.
   - The Servo Horn on this robot is the black round plastic piece side of the servo with a screw in the center. 
   - The silver round metallic piece with a screw in the center is just part of the servo mounting hardware.
   - What I chose to do was 'home' the robot by either reset button or power cycle. Then unscrew both sides of the servo and raise your robot axis to the desired position carefully. 
   - Reinstall the screws to both sides of the servo. Now when you power it on, it should stay in that location. Do so for each axis.
   - Fortunately for me, Axis 5 was 90 degrees off home so this was an easy fix and prevented slamming into table.
