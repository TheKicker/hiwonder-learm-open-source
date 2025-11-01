# HiWonder LeArm Open Source Effort

![LeArm](https://www.amazon.com/stores/page/F831EB5A-A2F2-45C6-9D1C-5EA8EB33136D?ingress=2&lp_context_asin=B0FJFVV6C7&visitId=2a151ad0-5a80-4abc-8b23-ecf35a2b0c5e&store_ref=bl_ast_dp_brandLogo_sto&ref_=ast_bln)

This GitHub repository is my personal attempt to get the **HiWonder LeArm Robotic Arm Kit (6DOF)** working after purchase.  

**Product link:** [HiWonder LeArm on Amazon](https://www.amazon.com/dp/B0FJFVV6C7?ref=ppx_yo2ov_dt_b_fed_asin_title&th=1)  
**Official Wiki:** [HiWonder LeArm Wiki](https://wiki.hiwonder.com/projects/LeArm_AI/en/latest/docs/1.Geting_Ready.html)  
**Additional Docs (Google Drive):** [Google Repo](https://drive.google.com/drive/folders/1vwAXIoUjviLJAhGvHmiILseAW0N5tj59?usp=drive_link)

---

## Background

I purchased this kit to explore Arduino-based robotics and STEM projects. After carefully assembling it over **4 hours** and triple-checking every step, I plugged it in—**nothing worked**.  

This repo exists to document my efforts to make this kit functional and to gather fixes, improvements, and translations of Chinese resources to English.

---

## Repository Structure

```
hiwonder-learm-open-source/
├── led-lights/             # Code for controlling the LEDs
├── libraries/              # Arduino libraries needed for the arm
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

- Kit assembled successfully, all servos and mechanical parts physically installed.  
- Electronics are all connected, but initial tests produce **no response**.  
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

