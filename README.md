# ESP32_FreeRTOS_Demo
Re-Familiarizing with FreeRTOS

## Features
* ESP32 running 3 tasks on FreeRTOS on a single core
* "LED Task" flashes LED, IFF the button has been pressed in the last second
* "Heartbeat Task" increments a global int and prints it frequently
* "Button Handler Task" defers until button is pushed, then decrements global int and starts or resets timer for LED Task 

## Running the Project
* Get an ESP32 (they are cheap and plentiful)
* Connect it via USB_Micro to a computer
* Download free Arduino IDE from https://www.arduino.cc/en/software
* Click `File/Open.../` and navigate to this `esp32_freertos_demo.ino` 
* Open Arduino IDE and navigate to `File/Preferences.../Settings`
* From there, add the following board support URL to `Additional Board Manager URLs`: https://espressif.github.io/arduino-esp32/package_esp32_index.json 
* Click the Right Arrow `->` to compile and run