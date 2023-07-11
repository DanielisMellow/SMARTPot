# SmartPot
The SmartPot is an open-source project that creates a fully autonomous self-watering flowering pot. 

# General Overview
The microcontroller takes readings from a capacitance soil moisture sensor (CSMS) to determine if the soil is dry and requires watering. After MCU determines the plant needs water, a GPIO pin with a relay triggers a small water pump to water the flowerpot. An additional Temperature and humidity sensor(BME280) will take readings from the plant's environment and display them in a complementary GUI hosted on the ESP32. The GUI allows for additional controls, such as enabling the AUTO mode (self-watering mode based on sensor readings) or a manual watering sequence. The sensor readings are displayed in real-time and as a graphical timelapse. 

# Features
- Status Indication LED(Green-Auto / Blue-Watering)
- WiFi Manager
- Compact Design (Suitable for House Plants) 
- Webpage Graphic User Interface

# Functional Decomposition  
![SmartPot-Page-1](https://github.com/DanielisMellow/SMARTPot/assets/82124061/ca4d7b1e-5da4-40e6-b4b0-90f6148438ac)

# PCB 
![image](https://github.com/DanielisMellow/SMARTPot/assets/82124061/d67f55c3-e573-43b5-8a6f-f3c5e03d56ae)

# User Interface 
![GUI](https://user-images.githubusercontent.com/82124061/153277552-153ed56f-00fd-4204-a495-f35f5df36e99.png)

# SmartPot
![IMG_4085](https://user-images.githubusercontent.com/82124061/153272789-2df4694a-e000-48f3-9959-a7e7cce7b357.jpg)


