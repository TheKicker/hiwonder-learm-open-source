# Factory Firmware

The ZIP drive in this directory is what I received from HiWonder.  It contains the factory firmware for the LeArm 6DOF robotic arm kit.

Trying to install via Arduino IDE is downright impossible. It is riddled with errors, and if you tried - you may get stuck in an endless loop from the way Windows is caching files. 

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

## The Problem Statement(s)

- Windows cached old object files, causing multiple errors.
- The issue is that the `kinematics` folder is not named correctly.  It should be named `Kinematics` instead of `kinematics`. Caused dual definitions in compiler. 
- Arduino Cloude Editor flattens everything to the top level directory. Thus all references pointed at folder structure that didn't exist.

## The Fix
 Steps Taken to Fix Compilation & Upload Issues

1. **Move away from Arduino Desktop IDE (Windows caching issues)**  
   - The Desktop IDE on Windows sometimes reuses old compiled object files, causing multiple definition errors.  
   - Switched to **Arduino Cloud Editor**, which forces clean builds and handles file paths more consistently.  

2. **Rename `kinematics.hpp` and `kinematics.cpp` to match case**  
   - Did not need to changed `kinematics.hpp` → `Kinematics.hpp`, it was already correct.  
   - Changed `kinematics.cpp` → `Kinematics.cpp`  
   - Arduino Cloud Editor and the ESP32 toolchain are **case-sensitive**, so the mismatch caused linker errors.  

3. **Flatten all file references**  
   - Removed `../../` and `../src/` references in `#include` statements.  
   - Arduino Cloud Editor moves all files to the top-level folder, so relative paths needed to be eliminated.  

4. **Zip the folder and import into Arduino Cloud Editor**  
   - Created a **project ZIP** containing all the `.ino`, `.cpp`, `.hpp`, and necessary files.  
   - Imported it to the cloud editor to maintain a consistent file structure and clean build.  

5. **Upload to ESP32 Dev Module**  
   - Selected the ESP32 Dev Module as the board. 
   - Verified the code compiled and uploaded successfully.  

6. **Move away from the device before running the arm**  
   - The robot arm moves **quickly to its initial position**, so stepping back is crucial for safety.  
