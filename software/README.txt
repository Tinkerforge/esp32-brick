Build instructions
------------------

- Install nodejs and npm (left as exercise for the reader)
- Install platformio:
  pip install -U platformio

- Run "npm install --save-dev" in web/ to install the web interface build dependencies   
- Run "platformio run" to build the firmware. The firmware can be found in .pio/build/esp32dev/firmware.bin
- Alternatively run "platformio run -t upload -t monitor" to build and upload the firmware to an ESP32 and start the serial monitor
