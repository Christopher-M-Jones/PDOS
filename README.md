# PDOS
Power-Distribution Optimization System (PDOS) is a feature that expands upon existing residential building management systems by providing full energy automation and optimization for systems that incorporate solar, battery, and grid power generation sources.

![System Abstraction](https://imgur.com/boRK7LY)

The system uses intelligent power source-switching through the calculation of solar irradiance data, instantaneous battery current, and power grid pricing to optimize solar, battery, and power grid usage to meet the energy demand of residential applica-tions. The following document expounds on PDOS’s design by detailing the hardware and software components that comprise its subsystems. 

The figure below shows the system overview for the hardware subsystems and their inputs in PDOS.

![Figure 2.1](https://imgur.com/yYpduJb) 

# Hardware

PDOS uses two microcontrollers, an Arduino Mega 2560 (Arduino) and an ESP-12E NodeMCU
(NodeMCU), to gather data and control the connecting subsystems. A battery management system comprised of an input power supply, buck converter, current sensor, and hardware relay is controlled by the Arduino and charges the connected system battery, monitors current, and prevents current overdraw and
power surges. A liquid crystal display (LCD) is used to monitor the battery management system by displaying the battery’s state of charge and instantaneous current draw. Lastly, a secure digital (SD) card reader is used to store load demand data and solar panel wattage-per-hour data for a residential home model and is read by the Arduino microcontroller and applied to a source-switching algorithm.

# Power

The Arduino receives 5 V from a power adapter connected to a 120-V wall outlet, and it supplies power to the LCD, SD card reader, and NodeMCU through its 5-V voltage common collector (VCC) pin. The battery management system is supplied with 9 V from a power adapter also connected to a 120-V wall outlet, and that voltage is stepped down to 4.2 V through a buck converter to supply the system battery. 

The table below lists the current and power draw of the main hardware components used in PDOS.

| Component | Input Voltage (V) | Maximum Current Draw (A) | Average Current Draw (A) | Average Power Consumption (W) |
| ------ | ----------- | ------ | ----------- | ------ |
| Arduino Mega 2560   | 5 | 0.2 | 0.07319 | 0.36595 |
| ESP-12E NodeMCU | 5 | 0.17| 0.03958 | 0.13061 |
| Adjustable Voltage Regulator | 5.3 - 32 | 4 | 3 | 35 |
| 3.7 Volt LIPO 2000 mAh Battery   | 3.7 | 2 | 1 | 7.4 |
| Micro SD TF Card Adapter | 3.3 | 2 | 0.08 | 0.4 |
| ACS712 AC and DC Current Sensor Module | 5 | 0.013 | 0.01 | 0.05 |
| HD44780U (LCD-II) | 5 | 0.0003 | 0.00015 | 0.000105

The system uses a total average current draw of 4.20 amperes (A), a total average power consumption of
43.35 watts (W), and an input voltage that does not exceed 5 V.

# Microcontrollers

PDOS uses two microcontrollers, the Arduino and the NodeMCU, for gathering and parsing data as well
as controlling their respective subsystems of connected hardware components. The Arduino is used to interface with the NodeMCU and SD card reader and control the battery management system and LCD screen. Its defining feature is the large number of input/output (I/O) pins (54) that allow it to power and communicate with multiple devices. The NodeMCU is used to gather data from online APIs, parse the data, store it into typecast variables, and serialize it for communication with the Arduino and a web server that displays that data. Its defining feature is a Wi-Fi module (ESP-8266) that allows for wireless communication over a local area network (WLAN).

The figure below details the pin connections between the NodeMCU and the Arduino.

![Microcontroller Pin Diagram](https://imgur.com/RUaQZmc)
