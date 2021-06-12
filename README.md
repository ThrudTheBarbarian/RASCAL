
# RASCAL

**R**adio
**A**stronomy
**S**oftware
**C**ontrol
**A**nalysis &
**L**ogging

Rascal allows you to simply monitor a Soapy-compliant (eg: RTLSDR) SDR device, doing some of the things necessary for amateur radio astronomy automatically. For example, one can set a dwell time on the signal, so it can be averaged over time to reduce the noise level. 

The plan is also to show the current visible sky, so readings can be mapped to what was in the telescope's view, and provide event logging, storage and playback

Data storage will be tiered, so that raw IQ will be stored for a short period of time, the time-averaged FFT will be stored for longer, and events approved by the user will be stored indefinitely.

Also intend to perform chirping of the data to counter frequency drift, up to what can reasonably be performed by the computer in real-time. It's possible that some external hardware (maybe FPGA driven) could be useful here

The app is divided into two parts - collection and detection, and viewing. Basically all the hard work is done in the daemon, and the viewer can then connect over a socket to see what the daemon has produced.
