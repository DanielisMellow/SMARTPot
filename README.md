# SmartPot
The SmartPot open source project creates a fully autonomous self-watering flowering pot. 

# General Overview
The microcontroller takes readings from a capacitance soil moisture sensor (CSMS) to determine if the soil is dry and requires watering. After MCU determines the plant needs water, a GPIO pin with a relay trigger a small water pump to water the flowerpot. An additional Temperature and humidity sensor(BME280) will take readings from the plant's environment and display them in a complementary GUI hosted on the ESP32. The GUI allows for additional controls, such as enabling the AUTO mode (self-watering mode based on sensor readings) or a manual watering sequence. The sensor readings are displayed in real-time and as a graphical timelapse. 

# Features
  -Status Indication LED(Green-Auto / Blue-Watering)
  -WiFi Connectivity
  -Solar Powered Battery Bank(Ideal for window seated plants)
  -Webpage Graphic User Interface

# Functional Decomposition  
![SmartPotFD](https://user-images.githubusercontent.com/82124061/168720465-791ae2e0-6b14-4e65-b5e2-ba1f7370572f.png)

# SmartPot:
![IMG_4085](https://user-images.githubusercontent.com/82124061/153272789-2df4694a-e000-48f3-9959-a7e7cce7b357.jpg)

# User Interface: 
![GUI](https://user-images.githubusercontent.com/82124061/153277552-153ed56f-00fd-4204-a495-f35f5df36e99.png)
