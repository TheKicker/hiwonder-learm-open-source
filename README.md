# HiWonder LeArm Open Source Effort

![LeArm](https://m.media-amazon.com/images/S/al-na-9d5791cf-3faf/1e663318-73fc-4cce-b78a-34f197be3b5d._CR0%2C0%2C1000%2C1000_SX200_.png)

This GitHub repository is my personal attempt to get the **HiWonder LeArm Robotic Arm Kit (6DOF) Open Source** working after purchase.  

**Product link:** [HiWonder LeArm on Amazon](https://www.amazon.com/dp/B0FJFVV6C7?ref=ppx_yo2ov_dt_b_fed_asin_title&th=1)  
**Official Wiki:** [HiWonder LeArm Wiki](https://wiki.hiwonder.com/projects/LeArm_AI/en/latest/docs/1.Geting_Ready.html)  
**Additional Docs (Google Drive):** [Google Repo](https://drive.google.com/drive/folders/1vwAXIoUjviLJAhGvHmiILseAW0N5tj59?usp=drive_link)

---

## Background

I purchased this kit to explore Arduino-based robotics and STEM projects. After carefully assembling it in a little over **4 hours** and triple-checking every step, I plugged it in annnnnnd **nothing worked**.  It was supposed to go to a home position, etc. 

- I tried the iPhone app, it told me “LeArm is currently not in app control mode. Please restart the robot and switch to app control mode before proceeding!”
- I tried the iPad app, it told me “LeArm is currently not in app control mode. Please restart the robot and switch to app control mode before proceeding!”
- I tried the Playstion 2 (PS2) Controller that came in the box, totally ignored anything I pressed.
- I tried the LeArm.exe Windows Desktop app and same message as 1 and 2, the robot was not in a certain control mode. Please restart.
- I tried arduino, and keep getting different errors about the built in open source libraries not being found or using old function/variable references and the robot/Espressif ESP32-WROOM32D board being completely unresponsive.

I'm really trying to make a fun hobby robot project out of a crappy unfinished product. Completely Chinese junk. But it's getting frustrating.

This repo exists to document my efforts to make this kit functional and to gather fixes, improvements, and translations of Chinese resources to English.

---

## Repository Structure

```
hiwonder-learm-open-source/
├── led-lights/             # Code for controlling the LEDs on ESP32
├── libraries/              # Arduino libraries needed for the arm as I add them
├── README.md               # This file
└── reference-materials/    # PDFs, 3D models, and electrical drawings
    ├── 3d-models/          
    ├── electrical-drawings/
    │   ├── CHINESE-SCH-*.pdf
    │   └── ENGLISH-SCH-*.pdf
    ├── A00-build-instructions.pdf
    ├── A01-introduction.pdf
    ├── A02-controller.pdf
    └── A03-deviation.pdf
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

## Notes / Current Status

- Kit assembled successfully, all servos and mechanical parts physically installed according to instructions. 
- Electronics are all connected, but initial tests produce **no response** whatsoever.  
- Documentation and reference material collected for troubleshooting.  
- Open to experimenting with code, firmware, and Arduino examples.  

---

## References

- [HiWonder Official Wiki](https://wiki.hiwonder.com/projects/LeArm_AI/en/latest/docs/1.Geting_Ready.html)  
- [Google Drive Reference Materials](https://drive.google.com/drive/folders/1vwAXIoUjviLJAhGvHmiILseAW0N5tj59?usp=drive_link)  

---

## How to Help

1. Fork this repo.  
2. Test Arduino sketches with your own LeArm kit.  
3. Translate additional Chinese resources if possible.  
4. Submit PRs with fixes, sketches, or documentation improvements.  
5. Report bugs or issues in GitHub Issues.

---

**Disclaimer:** This project is experimental. I am doing my best to make a functional robotic arm out of what was initially a non-working purchase. Use at your own risk.  

---

*PS: I’m determined to make this kit work, one step at a time.*

