# HB9HCR Sattrack &#x1F6F0;&#xFE0F;

Manufacture an electromechanical pan/tilt device to keep Yagi antennae toward targets like ... satellites - or elephants.

## Architecture
The appliance is controlled in the manner of the Apollo Guidance Computer with servos as output and sensors as input. It also features a so called DSKY, a user interface consisting of a display and a keyboard (keypad). The command strucuture follows a syntax consisting of numeric sequences, a so called "Verb-Noun-Combination".

![Verb Noun List](./img/verb-noun.png)

### Hardware

#### Display
I use the liquid crystal display which has 20x4 character slots as visual feedback of computer, program and input state:

![DSKY Display](./img/display.jpeg)

### Actuator
- Serial Bus Servo Driver Board https://www.dfrobot.com/product-3002.html

### Sensors
- I2C BMM350 Triple-Axis Magnetometer Sensor https://www.dfrobot.com/product-2874.html
- I2C BMI160 6-Axis Inertial Motion Sensor https://www.dfrobot.com/product-1716.html
- I2C Digital Wattmeter https://www.dfrobot.com/product-1827.html

### Other
- I2C DS1307 RTC Module https://www.dfrobot.com/product-879.html
- I2C HUB https://www.dfrobot.com/product-2179.html

## Command

### VERB
A verb is entered first in the sentence, followed by a noun. Together both form a command which can be executed by the computer. Using unassigned verbs to not have any effect. Verbs are also being used to display and manage the values stored in the computer memory. 

#### Reserved
```
00                  not in use
01                  display NOUN
02                  display NOUN, NOUN+1
03                  display NOUN, NOUN+2, NOUN+3
.
.
11                  monitor NOUN
12                  monitor NOUN, NOUN+1
13                  monitor NOUN, NOUN+1, NOUN+2
.
.
21                  load NOUN
22                  load NOUN, NOUN+1
23                  load NOUN, NOUN+1, NOUN+2
.
.
32                  recycle program
34                  terminate program
36                  restart system
37                  execute program
```

### NOUN
Together with a preceeding verb, the noun forms a command to be executed by the computer. The nouns point to a specific "address" in the computers memory. Some nouns are strictly hardware device outputs which can be used in subroutines (program). The value of these nouns will be refreshed prior to the execution of any logic code.

#### System &#x2699;
```
00 MS               Current milliseconds
01 FF               Current flip-flop based on milliseconds
02 HZ               Current processing frequency in Hertz
03 TS               Current unix timestamp
04 DATE             Current date from RTC
05 TIME             Current time from RTC
```

#### Sensor &#x1F4DF;
```
10 PWR               Current power draw in milliamperes
.
.
20 ACC X            Accelerometer X axis in ?
21 ACC Y            Accelerometer Y axis in ?
22 ACC Z            Accelerometer Z asis in ?
23 GYR X            Gyrometer X axis in millidegrees
24 GYR Y            Gyrometer Y axis in millidegrees
25 GYR Z            Gyrometer Z asis in millidegrees
26 MAG X            Magnetometer X axis in millidegrees
27 MAG Y            Magnetometer Y axis in millidegrees
28 MAG Z            Magnetometer Z axis in millidegrees
29 MAG HEAD         Magnetometer heading in millidegrees
``` 

#### Program 50 &#x1F6F0;&#xFE0F; (Satellite tracker )
```
50 AOS TIME         Time of acquisition of signal
51 AOS AZ           Azimuth at AOS
52 AOS EL           Elevation at AOS
53 LOS TIME         Time of loss of signal
54 LOS AZ           Azimuth at LOS
55 LOS EL           Elevation at AOS
.
.
90 STATE            Program state
91 T-               Countdown in seconds
92 AOS TS           AOS unix timestamp
93 LOS TS           LOS unit timestamp
```

## BO
### Tracker
- [ ] ...

## BOM
### Antennae
- [ ] ...