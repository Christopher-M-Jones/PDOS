# Power-Distribution Optimization System (PDOS)
PDOS is a rule-based energy management feature for residential building management systems that optimizes energy usage through automation for systems that incorporate solar, battery, and grid power generation sources.

&nbsp;

![overviewdiagram](https://user-images.githubusercontent.com/110949954/187375425-ee90b6fe-5291-410f-875f-50d41d8c3f64.png)

&nbsp;

The system uses intelligent power source-switching through the calculation of solar irradiance data, instantaneous battery current, and power grid pricing to optimize solar, battery, and power grid usage to meet the energy demand of residential applications. The following document expounds on PDOS’s design by detailing the hardware and software components that comprise its subsystems. 

&nbsp;

# Hardware

PDOS uses two microcontrollers, an Arduino Mega 2560 (Arduino) and an ESP-12E NodeMCU
(NodeMCU), to gather data and control the connecting subsystems. A battery management system comprised of an input power supply, buck converter, current sensor, and hardware relay is controlled by the Arduino and charges the connected system battery, monitors current, and prevents current overdraw and
power surges. A liquid crystal display (LCD) is used to monitor the battery management system by displaying the battery’s state of charge and instantaneous current draw. Lastly, a secure digital (SD) card reader is used to store load demand data and solar panel wattage-per-hour data for a residential home model and is read by the Arduino microcontroller and applied to a source-switching algorithm. The figure below shows the system overview for the hardware subsystems and their inputs in PDOS.

&nbsp;


![topdowndiagram](https://user-images.githubusercontent.com/110949954/187375628-679a3a28-9793-426c-858c-627abdd28d8f.png)


&nbsp;

## Power

The Arduino receives 5 V from a power adapter connected to a 120-V wall outlet, and it supplies power to the LCD, SD card reader, and NodeMCU through its 5-V voltage common collector (VCC) pin. The battery management system is supplied with 9 V from a power adapter also connected to a 120-V wall outlet, and that voltage is stepped down to 4.2 V through a buck converter to supply the system battery. The table below lists the current and power draw of the main hardware components used in PDOS.

&nbsp;

| Component | Input Voltage (V) | Maximum Current Draw (A) | Average Current Draw (A) | Average Power Consumption (W) |
| ------ | ----------- | ------ | ----------- | ------ |
| Arduino Mega 2560   | 5 | 0.2 | 0.07319 | 0.36595 |
| ESP-12E NodeMCU | 5 | 0.17| 0.03958 | 0.13061 |
| Adjustable Voltage Regulator | 5.3 - 32 | 4 | 3 | 35 |
| 3.7 Volt LIPO 2000 mAh Battery   | 3.7 | 2 | 1 | 7.4 |
| Micro SD TF Card Adapter | 3.3 | 2 | 0.08 | 0.4 |
| ACS712 AC and DC Current Sensor Module | 5 | 0.013 | 0.01 | 0.05 |
| HD44780U (LCD-II) | 5 | 0.0003 | 0.00015 | 0.000105

&nbsp;

The system uses a total average current draw of 4.20 amperes (A), a total average power consumption of 43.35 watts (W), and an input voltage that does not exceed 5 V.

&nbsp;

## Microcontrollers

PDOS uses two microcontrollers, the Arduino and the NodeMCU, for gathering and parsing data as well
as controlling their respective subsystems of connected hardware components. The Arduino is used to interface with the NodeMCU and SD card reader and control the battery management system and LCD screen. Its defining feature is the large number of input/output (I/O) pins (54) that allow it to power and communicate with multiple devices. The NodeMCU is used to gather data from online APIs, parse the data, store it into typecast variables, and serialize it for communication with the Arduino and a web server that displays that data. Its defining feature is a Wi-Fi module (ESP-8266) that allows for wireless communication over a local area network (WLAN).
The figure below details the pin connections between the NodeMCU and the Arduino.

&nbsp;

![pindiagram](https://user-images.githubusercontent.com/110949954/187375704-8288c94f-4024-47e4-b92e-7c7eba074452.png)

&nbsp;

## Arduino Mega 2560 Microcontroller
The Arduino is responsible for controlling each hardware peripheral in PDOS’s battery management system, receiving data from the NodeMCU and SD card reader, and performing the calculations that automate and optimize the energy sources it uses. Control signals are sent from the Arduino to the battery management system that enable the charging and discharging of the battery management system and activate its surge protection features. If the battery-charging algorithm enables the charging state for the battery, a signal is sent from the Arduino to the battery management system to switch a relay on for charging. The battery charges when solar power generation is greater than load demand, the battery discharges down to twenty-five percent capacity, or a severe weather event is detected. If the current sensor reads a value above 5 A, a signal is sent to the relay that cuts off power to the battery. Finally, the Arduino receives voltage and current data from the battery management system, calculates its current charge, and sends a control signal to the LCD to display that data. 

The Arduino is also responsible for reading the data stored by the NodeMCU and SD card reader. Both the NodeMCU and SD card reader supply input data for the source-switching algorithms. Serialized data is received by the Arduino through universal asynchronous receiver-transmitter (UART) communication
with the NodeMCU and the Serial Peripheral Interface (SPI) from the SD card reader, and the data is deserialized and stored into variables. These variables are used in calculations that determine solar irradiance, power grid pricing profile, battery state of charge, load demand, and optimal source-switching. The figure below shows the hardware peripherals that are directly connected to the Arduino.

&nbsp;

![arduinodiagram](https://user-images.githubusercontent.com/110949954/187375789-08ec088c-ce7f-4317-b9bb-37f7d7e944bd.png)

&nbsp;

## ESP-8266 NodeMCU Microcontroller
The NodeMCU is responsible for downloading data from the OpenWeather and OpenEI online APIs, parsing that data into a readable format, and sending the data to the Arduino and a web server. The NodeMCU connects to an existing Wi-Fi network using the first available 802.11n, 802.11g, or 802.11b protocols and the network service set identifier (SSID) and password. Once connected to a wireless network, the NodeMCU sends a HTTP GET request to the OpenWeather and OpenEI servers and downloads
the response data in JSON format. The microcontroller then parses the JSON for data relevant to PDOS, serializes it for transmission to the Arduino and web server, and sends the data over the UART protocol to the Arduino and inter-integrated circuit (I2C) protocol to the web server. Figure 2.4 shows wireless communication between the NodeMCU and the OpenWeather and OpenEI web servers.

&nbsp;

![nodediagram](https://user-images.githubusercontent.com/110949954/187375879-588662dc-679e-48e9-abf7-dbc9f9842e36.png)

&nbsp;

## Battery Management System
The battery management system is responsible for charging and discharging the system battery, protecting the circuit from power surges and current overdraw, and sending battery power to the home. A lithium polymer (LiPo) battery running at 3.7 V and 2 ampere-hours (Ah) is the heart of the battery management system and is used as a power source when solar power cannot meet the load demand of the home. The battery management system with the LiPO battery was chosen for its practicality and economic feasibility. Battery simulation software was first considered as a solution to show the battery functionality of PDOS but was deemed impractical for both the customization needed and potential of system hardware integration. A full-sized 10 kW/h lithium-ion battery used in similar commercial products was also considered but was rejected for its excessive cost. The scaled-down battery and battery management system allow PDOS to demonstrate its intended functionality on a smaller scale that meets the economic constraints of the product budget. 

&nbsp;


| Product | Voltage (V) | Energy (Ah) |
| ------ | ----------- | ------ |
| EEMB 3.7 Volt LiPo 2000 mAh Battery   | 3.7 | 2 |

&nbsp;


The EEMB 3.7-volt, 2-Ah battery was chosen for the system because it meets the maximum voltage and power level of the design. The typical solar capacity of existing residential battery products such as Tesla’s Powerwall run at 10 kW/h, so using the EEMB battery means that PDOS scales down its power distribution model to 0.074% of the original size of the system [29]. The battery management system exists
for two primary functions: charging and discharging the battery. The charging subsystem is responsible for enabling the energization of the battery and preventing overcharging and power surges, while the discharge subsystem is responsible for dissipating energy from the battery and ensuring current is not overdrawn. The schematic for the charging function is detailed in the figure below.

&nbsp;

![batterychargediagram](https://user-images.githubusercontent.com/110949954/187375972-df2e853a-468b-478a-8f27-da31367b4751.png)

&nbsp;


The charging subsystem circuit consists of a 9-volt power supply, a direct-current to direct-current (DC-DC) constant-current, constant-voltage (CC-CV) buck converter, an LCD screen with an I2C adapter, a current sensor, a relay, a fuse, and the EEMB battery. The design of the charging subsystem is inspired by a Li-ion Arduino battery charging project on [Electronics Project Hub](https://electronics-project-hub.com/arduino-lithium-ion-battery-charger/). The EEMB battery is charged using the power stepped down from the buck converter. The current sensor detects how much current is needed to charge the battery at its current capacity, and it relays the data to the Arduino to determine the mode that is used to charge the battery. The two modes are constant voltage and constant current. Con-
stant voltage is used to charge the battery linearly until it reaches 90 percent capacity, and constant current is used to charge the battery with a small current until it reaches fully charged. Once the battery is fully charged, the relay disconnects the circuit, and the battery no longer receives power. 

The DC-DC CC-CV buck converter is required by the system to match the voltage of the 9-V input power supply with the EEMB battery voltage and allow PDOS to regulate current draw and prevent current overdraw. The table below shows the characteristics of the buck converter for the battery charging subsystem.

&nbsp;

| Product | Input Voltage (V) | Output Voltage (V) | Max Current (A) |
| ------ | ----------- | ------ | ------ | 
| DROK DC to DC Adjustable Voltage Regulator   | 5.3 - 32 | 1.2 - 32 | 12

&nbsp;

The battery discharge subsystem is comprised of a relay, Hyudup load tester, and the EEMB battery. The schematic for the discharging subsystem is detailed in the figure below.

&nbsp;

![batterydischargediagram](https://user-images.githubusercontent.com/110949954/187376050-1d08945a-e25a-407e-ac2a-5458305a3342.png)

&nbsp;


The EEMB battery is connected to the Hyudup load tester to discharge the battery and the Arduino to measure the voltage. The relay is connected to the power supply of the discharger and the Arduino. If the Arduino detects the battery is at 25% capacity, it sends a signal to the relay to disconnect the power supply from the discharger, and the battery can no longer discharge. The table below shows the characteristics of the discharging unit.

&nbsp;

| Product | Power Input (W) | Voltage Input (V) | Current Input (A) |
| ------ | ----------- | ------ | ------ | 
| Hyudup Electronic Load Tester   | 250 | 12 | 20

&nbsp;

The Hyudup load tester is used by PDOS as an energy dissipator for discharging the battery. When the battery begins discharging, the Hyudup load tester absorbs and dissipates the energy through a heat sink and fan.

&nbsp;

## SD Card Reader
The SD card reader allows PDOS to read the EnergyPlus model data from a microSD card and use the data as an input for its source-switching algorithms. The Arduino interfaces with the SD card reader through the SPI communication protocol library. The table below shows the characteristics of the SD card reader in the system.

&nbsp;

| Product | Voltage (VCC) | Current (mA) | Supported Cards |
| ------ | ----------- | ------ | ------ | 
| HiLetGo Micro SD TF Card Adapter   | 5 | 80 | Micro SD (<=2Gb), Micro SDHC (<= 32Gb)

&nbsp;

# Software
PDOS uses the Arduino IDE to program both the Arduino and NodeMCU microcontrollers. In addition to
the libraries provided by the standard library of the Arduino IDE, PDOS makes use of several open-
source libraries downloaded from the Internet to connect to web servers, download JSON files from
online APIs, and parse the data. EnergyPlus energy modeling software is used to calculate the baseline solar irradiance data and typical load demand data for a residential home, and that data is sent to the Arduino through the SD card reader
as input data for its solar irradiance algorithm. Algorithms for PDOS involve calculating input data values
for the main source-switching algorithm, determining if the unit is in inclement weather mode, charging
the battery through the battery management system, and switching the power sources based on the input
data. Data such as weather conditions, inclement weather alerts, battery state information, current grid power
pricing, total power intake, and load demand are periodically uploaded to a web server and updated dynamically to demonstrate the response of the system to its input data.

&nbsp;

## EnergyPlus

EnergyPlus is used to model energy consumption in buildings for heating, cooling, ventilation, lighting, and plug and process loads. PDOS is using an EnergyPlus model of an existing single-family home and has implemented two add-on measures to the model. The first measure simulates the solar output of the home, and the second measure outputs the EnergyPlus data into a Microsoft Excel-readable comma-separated value (CSV) file. The CSV file contains load demand and solar output data simulated in the summer and winter seasons and is recorded in one-hour increments. The summer and winter seasons were chosen because they represent the greatest variation in solar output and load demand.

In total, EnergyPlus records 48 data points over the course of two 24-hour simulations. For the data to be processed by the Arduino, the CSV is converted into a JSON file and written to the SD card. Once the SD card is read by the Arduino, the data is parsed and stored in input variables for the source-switching algorithm. Because PDOS’s battery management system is using a battery scaled down to 0.074% of a full residential battery system, EnergyPlus data has also been scaled down to 0.074% of the original values.The figure below shows the flow of data from EnergyPlus to the SD card.

&nbsp;

![simdatadiagram](https://user-images.githubusercontent.com/110949954/187376131-bc15f70a-d1b1-4a94-89a0-17cfabdf9b42.png)

&nbsp;

## Algorithms
Optimal power-source switching is the primary goal of PDOS, and this goal is accomplished through a series of algorithms that calculate input data, assign identifying characteristics to the power sources, establish a priority queue that slots the optimal source(s) for use, and checks the total output against load demand. PDOS uses the EnergyPlus solar simulation, OpenEI grid pricing, battery management system, and OpenWeather API data to calculate the characteristics of each power source and to determine the input data for the source-switching algorithm. This data is passed into the source-switching algorithm, and the optimum power source is chosen for slots one and two in a priority queue. When the queue has been decided, the summation of power output from both slots is checked against load demand to verify that the system is successful.

&nbsp;

## Solar Irradiance
The solar irradiance algorithm determines the true value of solar irradiance for the solar panels under all weather and time conditions. The algorithm takes inputs from the solar power output data of EnergyPlus for the current hour, and OpenWeather data for the current forecast (cloudy, rainy, sunny, and so forth) and temperature.

The EnergyPlus solar power output data is the baseline reading for solar power generation, and it is modified by the weather conditions. Forecast data acts as a multiplier for the power generated by solar panels. For example, a sunny day with no clouds multiplies the baseline power number with “1” to represent full solar irradiance intake efficiency, whereas a partly cloudy day multiplies the baseline number with “0.8” to represent slightly diminished solar irradiance intake efficiency. These numbers represent an average loss of 10-25% efficiency for [solar intake on a cloudy day](https://www.sullivansolarpower.com/about/blog/do-solar-panels-get-hot). The solar panels also lose some of their efficiency due to variations in surface temperature. Outside of the surface temperature range of 59 degrees to 95 degrees Fahrenheit, solar panels lose 0.5% efficiency for every two degrees Fahrenheit above or below this range. In cases with temperature outside of full efficiency range, 0.5% is subtracted from the forecast modified power output for every two degrees Fahrenheit above or below the range.

&nbsp;

## Grid Pricing
The grid pricing algorithm determines the pricing demand profile of the power grid cost based on the time of day. The pricing demand profiles are designated as peak power, off-peak power, super-peak power, and super off-peak power. These profiles are taken from the Tennessee Valley Authority (TVA) hourly pricing tiers of the same name and represent how much TVA charges for residential electricity based on the [time of day](https://www.mpus.ms.gov/mpus/oktibbeha).

&nbsp;

## Battery State
The battery state algorithm reads the current and voltage characteristics of the battery, calculates the instantaneous current supply ratio, and returns that value.

&nbsp;

## Inclement Weather Mode
The inclement weather mode algorithm determines whether to place an identifier characteristic on the battery that denotes incoming inclement weather. The inputs for the algorithm consist of a bool that is set high or low depending on if an alert is detected in the OpenWeather API data, and a string for the start time of the inclement weather event.If an alert is detected, inclement weather mode is enabled, and the battery cannot be used as a power source until the start time of the weather event.

&nbsp;

## Source-Switching
The source-switching algorithm begins by assigning three identifying characteristics to the generation sources based on the values of the input data: solar irradiance, instantaneous battery current, and inclement weather mode. Two power “slots” are created as a priority queue for the sources to meet load demand. The algorithm determines which power source should be selected for the first slot based on its identifying characteristics. A calculation is then performed to determine if the selected source for the first slot meets the full load demand. If it does, the second slot is not used. If it does not, the power source for the second slot is likewise to be determined by the characteristics of the remaining available power sources.

The first characteristic determined for the solar power source is whether the solar irradiance meets a threshold of 50-W output. If solar irradiance hits below this threshold, it would represent either nighttime or a very cloudy day when solar panel usage is not viable. Because PDOS is only operating at 0.074% of a full residential system, the value PDOS uses is scaled down to 0.037 W.The second characteristic assigned is for the battery and determines whether it is in inclement weather mode. If the battery is in inclement weather mode, the battery cannot be placed in a power slot until the start time of the weather event. The last characteristic designates whether the battery is above or below the 25% discharge threshold. If it is below the threshold, the battery is set in charging mode and cannot be placed in a power slot until it reaches 35% charge.

After assigning the characteristics to the power sources, the algorithm begins to assign a source to power slot one. If solar power is above the 0.037-W threshold, it is assigned to the first slot. A calculation is then made to determine if solar can meet the full load demand. If solar cannot fully meet load demand, its provided power is subtracted from the load demand, and the algorithm determines what is used in the second power slot. If solar power can meet the full demand, the second slot is not used, and a calculation is made to determine how much solar output exceeds demand. Excess solar output is used to charge the battery. If the battery is fully charged, no interaction with the battery occurs, and the excess solar power is ignored. If solar cannot be used for the first slot, battery power is considered instead. 

For the battery to be used in the first slot, its charge level must be above 35%, and the battery must not be in inclement weather mode. If both conditions are satisfied, the battery is set to power slot one, and a calculation is performed to see if the battery can meet the load demand. If it can meet the demand, the second slot is unused. If it cannot meet the demand, the second slot is filled by grid power. If the first slot cannot be fulfilled by either solar or battery power, grid power occupies the first slot, and the second slot is unused. Once the status of both power slots is determined, a calculation is performed to verify that the sum of the output power for both slots meet total load demand.

&nbsp;

## Battery Charging

If the battery is set in battery-charging mode, the algorithm enables an Arduino control pin for the battery management system. The input for this algorithm is a true/false bool that is determined by the solar power generation exceeding load demand, the battery discharge reaching twenty-five percent of total capacity, or the occurrence of a severe weather event.

&nbsp;

## Online APIs
PDOS gathers input data for its algorithms using online data provided by website APIs for the Open-Weather service and the OpenEI database. OpenWeather provides weather hourly forecasting data for a 24-hour period, and OpenEI provides hourly energy pricing data over the same period. Both sets of data are used as input values for PDOS’s power-balancing algorithms. For both the OpenWeather and OpenEI API call, the NodeMCU makes an HTTP GET request to the OpenWeather and OpenEI web API services, respectively. The data is then downloaded in JSON format to the microcontroller over Wi-Fi and stored in a buffer. The buffer data is parsed using the ArduinoJson library, and key data points are stored in dictionary data structures. The data is serialized and sent to the Arduino over a UART connection, and it is then deserialized and stored in variables by the Arduino.

&nbsp;

## User Interface

The first iteration of PDOS used a webpage hosted by a flask python webserver hosted on a Raspberry Pi 3, and it dynamically displayed battery power, grid power, solar power, current temperature, and the currently occupied power slots for PDOS.


Since then, the user interface has been redesigned into a mobile app using flutter. That project can be found [here]().