# Set up for the VOC detection

Requirement:
- arduino uno
- adafruit sgp30 gas sensor
- ST052 humidity sensor
- DFR0229 micro-sd module
- a LED with a suitable resistance

## Installation
![alt text](/pict.png)

Connect your arduino to your usb port on your computer the execute `sgp30test` in ***Sketch/Exemples/Adafruit SGP30 sensor*** after installing the **Adafruit_SGP30** library.

Then open up the serial console at 115200 baud, you'll see the serial number printed out - this is a unique 48-bit number burned into each chip. (source : SGP30 datasheet)

This code will allow you de determinate a baseline value so don't forget to ventilate your room.
Let the code run for a few minutes, then copy the baseline value displayed in the shell.\
Replace the baseline values used in `main.ino`, line 85 in the function **sgp.setIAQBaseline()**.

Now, you only need to execute `main.io` on your arduino.\
After that, your arduino only need to be powered to operate.
