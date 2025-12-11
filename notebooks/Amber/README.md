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

# 2025-10-7: TA Meeting
- First PCB iteration due, reviewed design and submission
- Breadboard demo 1 - showed lighting up LED with breadboard

# 2025-10-14: TA Meeting
- Design document done

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
- Logic level shifter for 3.3V to 5V step-up for LED data line:
  - Based off of schematic for SparkFun logic-level bidirectional
  - Data sheet: https://cdn.sparkfun.com/datasheets/BreakoutBoards/Logic_Level_Bidirectional.pdf
  - Example Schematic: <img width="356" height="308" alt="image" src="https://github.com/user-attachments/assets/d434adb5-8896-40eb-8a84-61fd00842916" />
  - Implemented Design: <img width="513" height="451" alt="image" src="https://github.com/user-attachments/assets/ca971390-01ad-48ee-8f71-f20d26a920fa" />

# 2025-10-20 - Photosensor Circuit
- Use analog output for ambient lights could use digital for 0 or 1 value
- <img width="982" height="387" alt="image" src="https://github.com/user-attachments/assets/448b1470-4001-40ef-ad34-cdfaf7915663" />

# 2025-10-27 - TA Meeting: Breadboard Demo 2
- Showed actual tracking of buses along green street

# 2025-11-3 - PCB Testing
- soldered PCB and tested
- able to flash microcontroller via USB after trial/error and could light up debugging LEDs with code
- linear regulators don't work with wall supply
- have to redesign power distribution system and microcontroller set up

# 2025-11-10 - Second PCB Design
- New microcontroller set up for EN and Boot correct oritentation:
  -<img width="680" height="623" alt="image" src="https://github.com/user-attachments/assets/7ea9b69b-88df-4167-b60a-7c812b76f0bd" />
- New power distribution circuit using a 5V to 3.3V converter to handle input noise or spikes in current/voltage draw
  - <img width="1082" height="453" alt="image" src="https://github.com/user-attachments/assets/4c226108-d42d-43f7-bac6-084606d7628b" />











