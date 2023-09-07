# guidentMobileNetworkTesting
This is a C++ based program for field testing a mobile network's QOS.

This program is for using a Cradlepoint brand modem/router to simultaneously and continuously record signal strength statistics (as provided by the Cradlepoint modem), packet round trip time (RTT), and current location. The program is meant to run in a portable computer connected to the modem/router equipped with an automotive mount antenna while driving around the area under test. Each second a timestamped record is written to a file including latitude, longitude, round-trip "ping" time and RSSI, RSRQ, RSRP and SINR statistics from the modem.

The data produced by this program can be processed to produced an image similar to the following:

![image](https://github.com/miketrank/guidentMobileNetworkTesting/assets/38054960/687b8900-7a14-4fa1-b3b9-1e52d8fb5482)


