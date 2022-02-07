The SmartPot open source project is designed to create a fully autonomous self-watering flowing pot. 

General Overview:
The microcontroller takes readings from a capacitance soil moisture sensor (CSMS) to determine if the soil is dry and requires watering. After MCU determines the plant needs water, a GPIO pin with a relay trigger a small water pump to water the flowerpot. An additional Temperature and humidity sensor(BME280) will take readings from the plant's environment. A complementary GUI is hosted on the ESP32 that allows the user to activate the water pump manually or initiate AUTO mode. In addition, the GUI displays the sensor readings taken with the BME280. 

