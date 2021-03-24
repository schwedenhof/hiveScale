# hiveScale
beehive scale with wireless data transmission over GPSR modem and ultra low power dissipation

## Features
* Based on an embedded system with small, ultra low power STM32 MCU STM32L031K6
* Scale with weight logger for beehives
  * configurable (e.g. 5 minutes) weight interval → e.g. 290 measurements a day, limited by available SRAM of Microcontroller
  * configurable (e.g. once a day) data transfer to a private server or public clouds like ThingSpeak, limited by power budget (battery capacity)
  * optional 2nd channel for 2 scales at one controller
* Battery driven, no mains power supply required, 6V-12V input voltage
* Low Power Design, allows simple, low maintenance and cost-efficient power supply
* Battery voltage monitoring
* Optional temperature sensor interface for DS18B20 sensors
* No WIFI required, collected data is transmitted via GPRS modem
* uses HTTP POST method to upload data to any kind of web based server
* Cost-efficient, total cost including electronics and mechanics starting at €90 
