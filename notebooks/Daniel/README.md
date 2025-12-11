# Daniel's Notebooks

This directory contains Daniel's notebooks and work for the ECE445 MTD Tracker project.

- 2025-09-03 - RFA Creation
- 2025-09-10 - Proposal Creation
- 2025-09-14
# 2025-09-3 - Brainstorming & RFA
First meeting to come up with potential ideas for senior design project.
- Initial Ideas:
  - 3D Printed Battle Bots - join the battle bot competition with professor gruev
  - Pill Sorter - automatically sort pills by days needed
  - Automatic Frozen Fish Feeder - keeps fish food frozen and automatically feeds the fish
  -nd  MTD Bus Visual - Live feed of the buses on campus ashows them on PCB/3D printer*

- MTD Bus Tracker Project Idea:
  - live tracking every 30 sec
  - two-way LED roads along 3d printed champaign
  - maybe include brightness, themes
  - 7-segment display for time or date?
  
# 2025-09-10 - Proposal Creation
Meeting to start creating proposal documentation
- Daniel: Visual aid and a block diagram.
- Brainstormed what the block diagram needs and what components we should consider using.

# 2025-09-14 - Block Diagram and Visual Aid
- Visual Aid
<img width="1963" height="1085" alt="Visual Aid" src="https://github.com/user-attachments/assets/c3b8065c-8ae2-4509-9d80-4cafc5b8311a" />
  
- Block Diagram
<img width="1825" height="990" alt="blockDiagram" src="https://github.com/user-attachments/assets/52fb0858-e097-4e3f-a9e4-11ddf0b8d9f6" />

# 2025-09-20
- Basic design of the 3d model, making it simpler and figuring out the size


# 2025-10-6 
- Lab meeting, built out a simple breadboard with the LEDs, LED chase, button control, and photoresistor analog input.
- Added the Arduino code

# 2025-10-7
-Started the first 3d prints to get options for scale 
-<img width="1410" height="886" alt="3dmap" src="https://github.com/user-attachments/assets/ea35188f-664e-4546-998d-71e125d634e6" />

# 2025-10-14
-Design Doc Done
  
# 2025-10-14 - 11-20
-3d printing 25 tiles for the map

# 20205-10-20 - PCB Layout Rev1
- Routed all the traces on a PCB for the first iteration passes all PCB way tests
<img width="1522" height="1408" alt="pcbrev1" src="https://github.com/user-attachments/assets/e9088376-9019-4278-861b-b07c68d2fb51" />

# 2025-10-27 - TA Meeting: Breadboard Demo 2
-We showed tracking buses on a mock 3d map with a functioning PCB

# 2025-11-2 - PCB Soldering
-Soldered PCB
-tested all of the traces to make sure they work
-The linear regulator kept burning out
-Using bench power, able to flash simple test code to light up RGB LED

# 2025-11-10 - PCB Layout Rev 2
-Routed all the traces on the new iteration of the board, changed most of the layout to feature better placement of components, and new power system
<img width="1876" height="1208" alt="pcbrev2" src="https://github.com/user-attachments/assets/7ebfd849-e81b-4bde-a092-d14b7420dc01" />

# 2025-11-11 - TA Meeting
-Still 3D printing the model 

# 2025-11-13 - Soldering PCB
- Soldered the new PCB rev 2 board and did the same test to make sure power and ESP32 flashing works, not tested on wall power
- ![PCBrev3](https://github.com/user-attachments/assets/c5b21fcc-9fb6-4249-89a1-9db57a3fd0fe)

# 2025-11-18 - TA Meeting
-Demonstrated new PCB functioning on bench power supply and additional mapping of buses on LED strip

# 2025-11-29 - Physical Assembly Done
-Physically glued all 25 plates to the backboard and started designing a custom 3d printed border 
<img width="828" height="995" alt="assebmled" src="https://github.com/user-attachments/assets/c6f94008-f0e7-443e-9216-7a3ee6eebfe0" />

# 2025-12-30 - Routing Led Strips
-Spent most of the day routing led strips through all roads buses go
-Worked on code for color matching to buses

# 2025-12-10 - Final Demo
-Final project with a custom menu for bus route selection
![demo](https://github.com/user-attachments/assets/b6816e08-eeb3-4f85-bc5d-44fd98392afe)


