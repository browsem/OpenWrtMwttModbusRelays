# OpenWrtMqttAndRelayPackages

Well this went south in a hurry.
I realised that i didnt actually needed to use the usbrelay, or the cRelay since i was using an Arduino Leonardo to get DS18x20 sensor data.
I could just add the output functionality to that, and use cheaper relays.
So thats what im doing.


A fugly mqqt wrapper for 
https://github.com/ondrej1024/crelay

packaged for openwrt.

I  created a cRelay simulator, as i didnt have the hardware on my devel machine.

The goal of it, is to create a wifi connected heat control system, using
Tasmota on an Esp32 on one end, and my wifi router on the other,
Using Mqtt to tie it together and lua as the main scripting language. the reason for using lua is that its native to openwrt, and aubo robots use it as well, and i need to learn.

Steps  
1: Get crelay wrapped into mqtt  
Status:  done, not tested using actual hardware ,but not needed anymore
There is a branch that still includes this file
2: Get the Tasmota temperature/relay data, and create a simulator for the non hardware machine.  
Status: done, checked up against the real thing.  
3: Create another wrapper, to get the arduino connected temperature sensors into mqtt on the router.  
Status: Done, uses a simulator to create the sensor data.
Will be expanded to also be able to control relays
4: Create the heat control logic, in another lua client. 
Status: done
Seems to run fine, the package is brfHeatControl 
5: bind the mqtt to a phone app ??  
Status: I bound the heatcontrol client to IOT MQtt
6: Throw the shit away, and use a real plc instead.  
Status: I got my hands on and ABB 654 plc, Perhaps modbus tcp +a wired arduino to the remote position
And using an rs232 arduino to get sensor data into it

