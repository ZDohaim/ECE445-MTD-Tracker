# Amber's Notebooks
This directory contains Amber's notebooks and work for the ECE445 MTD Tracker project.

- 2025-09-03 - RFA Creation
- 2025-09-10 - Proposal Creation
- 2025-09-28 - Initial PCB Design
- 2025-10-2 - PCB Design and Components
- 2025-10-6 - More PCB Design and Components

# 2025-09-3 - Brainstorming & RFA
First meeting to come up with potential ideas for senior design project.
- Initial Ideas:
  - 3D Printed Battle Bots - join the battle bot competition with professor gruev
  - Pill Sorter - automatically sort pills by days needed
  - Automatic Frozen Fish Feeder - keeps fish food frozen and automatically feeds the fish
  - MTD Bus Visual - Live feed of the buses on campus and shows them on PCB/3D printer*

- MTD Bus Tracker Project Idea:
  - live tracking every 30 sec
  - two-way LED roads along 3d printed champaign
  - maybe include brightness, themes
  - 7-segment display?
  
# 2025-09-10 - Proposal Creation
Meeting to start creating proposal documentation
- Amber: make the introduction (problem and solution)
- Problem: students find bus routes too confusing, miss bus
- Solution: high-level design, why this is better than just a screen

# 2025-09-29 - PCB Design
<img width="237" height="295" alt="image" src="https://github.com/user-attachments/assets/cfb3fc56-cdc9-4846-9f4f-caf801beae50" />
- use the ESP32 s3 wroom 1 (wifi)

# 2025-10-2 - PCB Design and Components
- CL21A226MAQNNNE - 22uF
- CC0805KRX7R9BB104 - 100nF (2)
- CL21B105KBFNNNE - 1uF

- <img width="1792" height="2484" alt="image" src="https://github.com/user-attachments/assets/b140b522-7185-4191-9431-312bd4baf9a5" />

- Working File Uploaded

# 2025-10-6 - More PCB Components
-Capacitors:
  - [CL21A226MAQNNNE](https://www.digikey.com/en/products/detail/samsung-electro-mechanics/CL21A226MAYNNNE/10479857) - 22uF (decouple capacitor)
  - [CC0805KRX7R9BB104](https://www.digikey.com/en/products/detail/yageo/CC0805KRX7R9BB104/302874) - 0.1uF (4)   (decouple capacitor, enable capacitor, photoresistor capacitor)
  - [CL21B105KBFNNNE](https://www.digikey.com/en/products/detail/samsung-electro-mechanics/CL21B105KBFNNNE/3886687) - 1uF    (boot capacitor)
  - [TAJA106K016RNJ](https://www.digikey.com/en/products/detail/kyocera-avx/TAJA106K016RNJ/563762) - 10uF   (step down polarized capacitor)
  - [TCJB226M025R0150E](https://www.digikey.com/en/products/detail/kyocera-avx/TCJB226M025R0150E/10062672)](https://www.digikey.com/en/products/detail/kemet/T529P226M010AAE200/4246277) - 22uF   (step down   polarized capacitor)
  - barrel jack - https://www.digikey.com/en/products/detail/same-sky-formerly-cui-devices/PJ-080BH/3477153
  - USB - https://www.mouser.com/ProductDetail/GCT/USB4216-03-A?qs=Tc%252BHE9vUsnv1G2UWLFIexA%3D%3D
  - fuse - https://www.digikey.com/en/products/detail/bel-fuse-inc/0ZCF0500FF2A/10445775
    



