## NOUN
Together with a preceeding verb, the noun forms a command to be executed by the computer. The nouns point to a specific "address" in the computers memory. Some nouns are strictly hardware device outputs which can be used in subroutines (program). The value of these nouns will be refreshed prior to the execution of any logic code.

### System &#x2699;
```
00 MS               Current milliseconds
01 MS ALT           Current alternation based on milliseconds
02 HZ               Current processing frequency in Hertz
03 TS               Current unix timestamp
04 DATE             Current date from RTC
05 TIME             Current time from RTC
```

### Sensor &#x1F4DF;
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

### Program 50 &#x1F6F0;&#xFE0F; (Satellite tracker )
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