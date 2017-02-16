#Readme ESP8266

The task of the ESP8266 is to communicate to the STM32 via RS232 on the one hand and to communicate to external network devices via WLAN on the other hand. Here is how to use the code shown in this directory.

## Compile/Install

I used arduino IDE just from the arduino website and configured it in a way that it is possible to programm EPS's with it. Once thats done(Please refer to https://github.com/esp8266/Arduino for exact informatin) you have to download the PubSubClient library from knolleary (https://github.com/knolleary/pubsubclient). You can just download and extract it to your arduino installation library folder. In my case it was  ~/Downloads/arduino1.6.8/libraries. Once thats done you need the ESP8266Wifi library but this should be available automatically if you follow the instructions for the esp8266-arduino IDE. Once you have all that in place you can open a new sketch and copy & paste the contents of esp_sketch/esp_sketch.ino. If you have everything installed and running you can use the following commands to communicate with the ESP:

## Usage of the UART Interface

The Task of the ESP is to be an MQTT2UART-Gateway. After you flashed the code you should be confronted with the following Message

```
<DBG> need to be configured.
<DBG> Config me like this:<delim><mqtt_server><delim><ssid><delim><pw><delim><topic><delim><topic>
```

### How to config
You can choose any "delimiter"-character which ist not '\r'. Choose one that does not appear within the IP-address, WLAN-passphrase or the topics. The program uses the first character it receives as the delimiter. Put the IP-adress between the first and second occurence of the delimiter, the WLAN-SSID between the second and the third, the password between third and the fourth and after that you can list all topics you want to subscribe to delimited by the delimiter. Do not put the delimiter at the end of the config String. The string has to end with '\r'.

**Example**

If you want to attach to an MQTT Server at Port 1883 (this is hardcoded for simplicity) at the IP 10.0.0.1, SSID: klausdieter and password: brunhildebrigitte and you want to subscribe to the topics /lotto /gluecksspirale /bingo you have to use the following configstring

`%10.0.0.1%klausdieter%brunhildebrigitte%/lotto%/gluecksspirale%aktionmensch\n`

The response should look like this:

```
<DBG> System configured. Here is your config: 
<DBG> MQTT-Server: 10.0.0.1
<DBG> SSID: klausdieter
<DBG> pw: brunhildebrigitte
<DBG> You subscribed to 3 topics.
<DBG> /lotto
<DBG> /gluecksspirale
<DBG> /bingo
<DBG> Attempting MQTT connection...
<DBG> MQTT Connected

```

### Some things to say about the Output
As you may have seen every line that says nothing about a subscribed topic starts with <DBG>. If a line says something about a  subscribed topic the output follows this syntax:
`<topic>:<message>\n`
### Publish over UART
Once the ESP is configured you can publish messages over UART using the following input:

`<delim><topic><delim><message>\n`
