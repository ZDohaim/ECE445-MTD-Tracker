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
  - Cap - https://www.digikey.com/en/products/detail/vishay-sfernice/T73YP103KT20/1587813
  - 3pin led connector - https://www.digikey.com/en/products/detail/te-connectivity-amp-connectors/282837-3/2187974?gclsrc=aw.ds&gad_source=1&gad_campaignid=20234014242&gbraid=0AAAAADrbLlhPA2MnfUI_BIzZuzQG9WJ-m&gclid=EAIaIQobChMIkLTjoJOTkAMVnUr_AR2CVDs0EAQYASABEgJeRPD_BwE
    
# 2025-10-16 - More PCB Design Work
- ESP32-S3 set up: (reference: https://documentation.espressif.com/esp32-s3-wroom-1_wroom-1u_datasheet_en.pdf)
  - <img width="527" height="686" alt="image" src="https://github.com/user-attachments/assets/28056e40-3391-4647-88ee-64e469730e14" />
  - <img width="832" height="837" alt="image" src="https://github.com/user-attachments/assets/644222f0-5182-4d3c-8a93-af31b95e1c2c" />
  - https://www.youtube.com/watch?v=EdEwRXiQsfc  (microcontroller schematic design example)
  - https://www.youtube.com/watch?v=xK14eVJCoqU   (microcontroller schematic design example)
  - <img width="746" height="701" alt="image" src="https://github.com/user-attachments/assets/d2327905-c5ec-4503-98db-e698660a1ccd" />

  - set up microcontroller based on datasheet and video references, as well as usb, boot/enable buttons, and input
 
# 2025-10-18 - Power Distribution Design
- Choose to use linear regulator for 5V to 3.3V step-down voltage from barrel jack input using
- Datasheet: http://www.advanced-monolithic.com/pdf/ds1117.pdf
- Preliminary Design:
  - <img width="712" height="465" alt="image" src="https://github.com/user-attachments/assets/5ebda353-ec5f-4a4c-aca8-71256edd1a97" />







