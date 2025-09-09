# ECE445-MTD-Tracker

## Champaign MTD Bus Tracker Map
Team Members:
Amber Wilt (anwilt2)
Daniel Vlassov (dvlas2)
Ziad Aldohaim (ziada2)
Problem
Champaign has a very large and complex bus system through the MTD. It can be hard for students to know when the buses are coming when they are in buildings such as the ECEB, since the bus times are only displayed at the stops. Furthermore, these buses can be late or early, causing students to miss their bus or not arrive at their destination on time.

### Solution
To fix this, we will come up with the design for a large display that shows real-time locations of all buses (color-coded using RGB) in the surrounding campus area. This can be used by students in buildings to easily visualize where the bus they want to take is currently located, making it easier for students to time when to leave classrooms and when to expect their ride. The display will update the locations approximately every 30 seconds and will light up every LED along a bus route every few minutes to make it easier for students to visualize which bus route they need to take. Furthermore, the system will include various light settings (theme/brightness).

### Solution Components
This system will mainly include the subsystems of the LED matrix, the controller, and the power supply.

#### Subsystem 1 - LED Matrix
The LED matrix will be located on a large PCB or 3D printed map of the city (cost dependent). This subsystem will be made of addressable LEDs, photoresistors to automatically modify the intensity of LEDs, and will be controlled by the microcontroller (to indicate positions).

#### Subsystem 2 - Microcontroller
The microcontroller will utilize wifi to access the MTD API to gather real-time bus data as well as provide control to individually address each LED within the matrix. Furthermore, it will control/communicate with other modules/displays in the system, such as a real-time clock or menu. The microcontroller will be an ESP32

#### Subsystem 3 - Power Supply
The power supply will provide ample power to a large number of LEDs (and the entire system). We will need to include a buck converter to step down the power supply to be usable by the LEDs.

## Criterion for Success
To demonstrate the success of our project, we will need to prove the accuracy of the data we are displaying (how accurate are bus timings/locations). Additionally, we will need to show that the data is easy to interpret for a user and can be utilized for easier bus system use.
