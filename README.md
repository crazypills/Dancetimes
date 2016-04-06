# Setup
Symlink both libraries into the the libraries into your Arduino libraires directory.  On mac this is located in ~/Documents/Arduino/libraries

This project also depends on the FHT library found here [ArduinoFHT2.zip](http://wiki.openmusiclabs.com/wiki/ArduinoFHT?action=AttachFile&do=view&target=ArduinoFHT2.zip)

```
cd ~/Documents/Arduino/libraries
ln -s ~/usr/src/Dancetimes/libraries/Accel
ln -s ~/usr/src/Dancetimes/libraries/NeoPatterns
ln -s ~/usr/src/Dancetimes/libraries/Quaternion
ln -s ~/usr/src/Dancetimes/libraries/Phase
ln -s ~/usr/src/Dancetimes/libraries/Adafruit_LSM9DS0_Library
```

# Dependencies

The Adafruit_LSM9DS0_Library is pulled into this repo and modified to remove the sensors api.
This is done to reduce the code size because we are buttting up against the flash memory.
This code is copyright Kevin Townsend for Adafruit Industries under the BSD license.
