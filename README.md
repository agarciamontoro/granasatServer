granasatServer
========================================================================
![GranaSAT](https://cloud.githubusercontent.com/assets/3924815/3865957/261cbb64-1fb6-11e4-8724-823485676743.jpg)
========================================================================
Server side of GranaSAT experiment, selected for BEXUS 19 campaign.

The client side code can be seen in its [GitHub page](https://github.com/M42/granasatClient).

### Abstract

GranaSAT designed and built a low-cost attitude determination system, a fundamental system for any spacecraft, based in a star sensor, horizon sensor and the magnetic field and acceleration measurements. The same Charge Coupled Device was used for both the star sensor and the horizon sensor. For the star sensor the Lost in Space functionality was designed, the identification algorithm used is a variation of the Matching Group algorithm proposed by Van Benzooijen; for the horizon sensor a simple detection algorithm is proposed, with the circle fitting method based in Umbach
and Jones work and for the magnetometer and accelerometer sensors the attitude was estimated by two vector matching procedure based in Wahba solution.

The star sensor was capable to obtain an attitude matrix for the 97.80 % of the images collected. The relative error in the angles measured was kept below 1% through a camera calibration and the error obtained in the attitude matrix obtained was found less than 250 arcsecs. The horizon sensor was able to process all the images in night
conditions, but it did not fulfil the 5o accuracy requirement. Finally, for the magnetometer and accelerometer sensors, an in-flight calibration was used to keep an uncertainty of the attitude angles under 4o with a maximum standard deviation in the magnetometer measurements of 140 nT.

After the results obtained, the most accurate and preferable method is the star sensor, despite its complexity. If low accurate attitude estimation or less complex solutions are required, the horizon sensor or magnetometer and accelerometer sensor solutions are valid for a spacecraft attitude determination system.

### Flight

The experiment flew, successfully, the night of 8 October 2014. Thousands of good images were collected, both for the Horizon Sensor and for the Star Tracker. The scientific and technical results will be exposed in the 22nd ESA Symposium on European Rocket and Balloon Programme & related Research, 7-12 June 2015.

In the meanwhile, enjoy the aurora borealist captured by our camera from the stratosphere:

[![Youtube video](https://cloud.githubusercontent.com/assets/3924815/7339020/2846735e-ec60-11e4-8b13-d59831fd01a4.png)](https://www.youtube.com/watch?v=YUlWg6wuCxo)

## Do you want to know more about us?

See more information at [GranaSAT website](http://granasat.ugr.es).

Like our [fanpage](http://www.facebook.com/granasat) at Facebook.

Follow us on Twitter: [@granaSAT](http://twitter.com/GranaSAT).

### Coders
Alejandro García [(@agarciamontoro)](https://github.com/agarciamontoro), Manuel Milla and Mario Román [(@m42)](https://github.com/M42)

==========================
*The [REXUS/BEXUS]((http://www.rexusbexus.net/)) programme is realised under a bilateral Agency Agreement between the* **German Aerospace Center** *(DLR) and the* **Swedish National Space Board** *(SNSB). The Swedish share of the payload has been made available to students from other European countries through a collaboration with the* **European Space Agency** *(ESA).*
