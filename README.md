## 👀 camera-trap Overview  
<h1 align="center">A handmade camera trap</h1>  

![camera-trap Demo Image](https://github.com/FURK4NGG/Fotokapan/blob/main/%7B%7D/fotokapan.webp)

## 🚀 Features  
<h1 align="center">This is a portable mini camera trap. It detects motion with a PIR sensor, uses flash when needed, and saves every photo to an SD card with a unique name.</h1>  


## 🔎 Preparation
<details>
<summary>1. Components</summary>
'1' ESP32-CAM<br>
'1' Motion sensor (PIR Sensor Hc-Sr501)<br>
'3' AA Batteries (1.5V each)<br>
'1' MT3608 adjustable DC/DC voltage regulator (4.5V to 5.01V step-up)<br>
'1' FT232RL USB to UART Converter<br>
</details>

2. `Use an SD card with a minimum capacity of 4 GB, formatted as FAT32 with a 32 KB allocation unit size.`
3. `Then insert this SD card into your ESP32-CAM board.`
4. `Connect your 'OV2640' camera to your ESP32-CAM board.`


## 📦 Setup 
1. `Refer to the 'circuit_fotokapan-programming.png' diagram`
2. `Install the 'Arduino IDE' software and open 'fotokapan.ino' with 'camera_pins.h' file`
3. `File>Preferences>Additional Boards Manager URLs:(Click the double window button)`
>Paste this code  
```bash
https://dl.espressif.com/dl/package_esp32_index.json
```
4. `Click 'OK'`  
5. `Tools>Board>Boards Manager...`  
6. `Search 'esp32' by Espressif Systems, and install it` 
> ⚠️ **Warning:** Make sure you have installed the correct USB driver (CH340, CH341, FT232R / FTDI Driver, CP2102) before connecting the ESP32CAM to your computer.
7. `Plug the ESP32CAM into your computer`
8. `Tools>Board>esp32>'AI Thinker ESP32-CAM'`
9. `Tools>Port>'Select the esp's port'`

<details>
<summary>Apply these changes:</summary>

- CPU Frequency: 240MHz (WiFi/BT)
- Core Debug Level: None
- Erase All Flash Before Sketch Upload: Disabled
- Flash Frequency: 80MHz
- Flash Mode: QIO
- Partition Scheme: Huge APP (3MB No OTA/1MB SPIFFS)
- *115200 baud*

</details>

10. `Tools>PSRAM(Enabled)`
11. `Tools>Manage Libraries...>Install the libraries used in the code`
12. `Click the 'upload ➡️' button`  
✅ **To make sure it has been uploaded successfully, you should see the message 'Done uploading'** 
> ⚠️ **Advisory for Developers:** If you want to try this project in the future with new generation boards and you want to add new features like using 'DS1302 real time module', you must have 3 unused pins.

## 🎉 Run  
1. `Refer to the 'circuit_fotokapan.png' diagram`
2. `Position your camera trap at an appropriate angle`
3. `Let the electricity pass through the switch`
4. `Press the reset button on the esp32cam`
5. `You are ready to use your camera trap`

> ❔ `If an SD card is installed, photos are stored automatically. To access the photos without removing the SD card, connect GPIO12 to GND before powering on the device. The ESP32-CAM will then start in Web Server Mode and create a local Wi-Fi network (Fotokapan / 12345678), allowing you to browse, download, or delete photos stored on the SD card from any phone or computer.`  

> ⚠️ `After turning on the camera, wait 5 seconds before using it. Turn off the power before removing the SD card. Do not cover the front of the flash to prevent a potential fire hazard.` 


## 🔒 License  
<h1 align="center">📜 GPL-3.0 License</h1>
