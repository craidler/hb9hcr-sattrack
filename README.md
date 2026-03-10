# HB9HCR Sattrack &#x1F6F0;&#xFE0F;

With Prototype Mk2, I managed to control both servos in different modes. Azimuth is in relative (3), elevation in absolute (0) mode. Both axis can be "homed" at any given time. Elevation currently takes initial angle from the frame IMU and translates this to an offset of the center position (2023). Azimuth takes the "short path". Demo moves azimuth 0,45,325,0°, elevation 0,45,-45,0°.

<img src="./img/prototype-mk2.gif" width="100%">

### Actuator
- UART Servo Driver Board https://www.dfrobot.com/product-3002.html
- ST3020 Servo Azimuth https://www.waveshare.com/wiki/ST3020_Servo
- ST3020 Servo Elevation https://www.waveshare.com/wiki/ST3020_Servo

### Antenna
- [Yagi 7-Element 70cm Band](#antenna-70cm) (home brew)
- [Yagi 3-Element 2m Band](#antenna-2m) (home brew)

### Sensor
- I2C BMM350 Triple-Axis Magnetometer https://www.dfrobot.com/product-2874.html
- I2C BMI160 6-Axis IMU https://www.dfrobot.com/product-1716.html
- I2C Digital Wattmeter https://www.dfrobot.com/product-1827.html

### Other
- DFRobot SD2405 RTC
- I2C HUB https://www.dfrobot.com/product-2179.html

## Mechanic
tbd

## Antenna
Home brew, based on AI results and online Yagi calculator, fine tuned with an [NanoVNA H4](https://nanovna.com). My requirements are: Leight-weigth, non-conductive Boom, equal Boom length, Coil Design Beta Match.

<a name="antenna-70cm"></a>
### Yagi 7-Element 70cm Band &#x1F4E1;

![Yagi 70cm Band Radiator](./img//yagi-radiator-70cm.jpeg)
This is what my rapid prototyping brought out of a radiator. I duct-taped the aluminum tubes (50mm) just a bit so they fit into the acrylic tube (105mm) neatly and the result is a 5mm gap. Then I drilled 3.5mm holes 15mm apart and tapped M4 threads. For final assembly I will use conductive gel for the stainless steel screws through a lock-washer (to bite into the acryl), washer, two ring terminals and another washer on top. For the beta match I used a 100mm 1.5mm<sup>2</sup> wire and 2 windings around the aluminum tube.

#### Dimensions
The following dimensions are calculated based on the classic DL6WU design for 8mm aluminum elements mounted through or on a non-conductive PVC boom.

```
Element 	    Length      Pos on Boom     Spacing Prev
--------------------------------------------------------
Reflector (REF)	342 mm	     30 mm	        - 
Radiator  (RAD)	327 mm	    175 mm	        145 mm
Director 1 (D1)	310 mm	    230 mm	         55 mm
Director 2 (D2)	304 mm	    365 mm	        135 mm
Director 3 (D3)	299 mm	    535 mm	        170 mm
Director 4 (D4)	295 mm	    735 mm	        200 mm
Director 5 (D5)	291 mm	    960 mm	        225 mm
```

#### BOM
- [ ] 1x 1000mm M25 PVC Tube
- [ ] 2x M25 Cap
- [ ] 1x 35mm M25 PVC Tube (Choke, 5 windings, 5.5mm holes)
- [ ] 9x M25 PVC Clamp
- [ ] 4x 1000mm 8mm Aluminum Tube
- [ ] 14x 8mm Cap
- [ ] 1x 105mm 12mm Acrylic Tube (Radiator Connector, 5mm Gap)
- [ ] 50mm Duct Tape
- [ ] 2x M4x8mm (Feedpoints)
- [ ] 4x Ring Terminal 4mm (Coax + Beta Match)
- [ ] 1x 100mm 1.5mm<sup>2</sup> Copper Wire (Coil Beta Match)
- [ ] 1x Case?
- [ ] 2x Grommet (Radiator)
- [ ] 1x Aircell-5
- [ ] 1x PL Female

<a name="antenna-2m"></a>
### Yagi 3-Element 2m Band &#x1F4E1;

#### BOM
- [ ] 1x 1000mm M25 PVC Tube
- [ ] 2x M25 Cap
- [ ] 1x 35mm M25 PVC Tube (Choke, 5 windings, 5.5mm holes)
- [ ] 5x M25 PVC Clamp
- [ ] 4x 1000mm 8mm Aluminum Tube
- [ ] 1x 1000mm 10mm Aluminum Tube (Reflector Connector)
- [ ] 6x 8mm Cap
- [ ] 1x 105mm 12mm Acrylic Tube (Radiator Connector, 5mm Gap)
- [ ] 50mm Duct Tape
- [ ] 4x M4x8mm (Feedpoint + Reflector Connector)
- [ ] 4x Ring Terminal 4mm (Coax + Beta Match)
- [ ] 1x 100mm 1.5mm<sup>2</sup> Copper Wire (Coil Beta Match)
- [ ] 1x Case?
- [ ] 2x Grommet (Radiator)
- [ ] 1x Aircell-5
- [ ] 1x PL Female
