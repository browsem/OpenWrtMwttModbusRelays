{
"ipaddr_Comment": "Replace with your broker IP",
"ipaddr": "192.168.40.136",
"port_Comment": "Or 8883 for TLS",
"port": 1883,
"username": "Enter your username",
"password": "Enter your password",   
"Topic_Strings":{
	"Local":{ 
		"Relays":{
			"Topic":"/LOCAL/POWER",
			"Comment":"Relay nr as presented : reverse on/off",
			"RelaysToActuate":{
				"2":"Inv",
				"3":"NonInv"
				},
			"Timer":{
				"Comment":"From signal change til relay change",			
				"on":0,
				"off":60
				}			
			},
		"Sensors":{
			"Topic":"/LOCAL/SENSOR",
			"Comment":"sensor nr as presented : correction value to add to the measured temperatue",
			"SensorsToRead":{
				"1":"1",
				"2":"-1"
				}
			},
		"AppFeedback":{
			"Topic":"/HeatControl",
			"Comment":"sensor are presented to user",
			"Comment_":"controls are presented to user",
			"SensorsToRead":{
				"LOCAL":"",
				"TERMOKANDE":"",
				"RelayState":""	,			
				"ManuelOnOff":"",
				"RemoteState":""
				},					
			"Controls":{
				"ManuelOnOff":""
				}			
			}
		},
	"Remote":{
		"Relays":{
			"Topic":"/TERMOKANDE/POWER",
			"Comment":"Relay nr as presented : reverse on/off",			
			"RelaysToActuate":{
				"1":"NonInv"
				},
			"Timer":{
				"Comment":"From signal change til relay change",			
				"on":10,
				"off":0
				}			
			},		
		"Sensors":{
			"Topic":"/TERMOKANDE/SENSOR",
			"Comment":"sensor nr as presented : correction value to add to the measured temperatue",
			"SensorsToRead":{
				"1":"2",
				"2":"-2"
				}
			}
		}
	},
"ChangeOverTempDiff":{
	"DebounceTime":15,
	"Rising":1,
	"Falling":-1
	},
"PollTime":10,
"verbose":5
}