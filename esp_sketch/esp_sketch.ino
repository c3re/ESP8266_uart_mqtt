// Dreist geklaut. Bei Ihm hier: https://github.com/ItKindaWorks/ESP8266
#include <PubSubClient.h>
#include <ESP8266WiFi.h>

// Der ESP wird als Ausgabe entweder <DBG> und darauf folgende Debuggingmessages ausgeben 
// oder /topic/subtopic:message.
// Server
String s_mqtt_server = "";
const char* mqtt_server;
// SSID
String s_ssid = "";
const char* ssid;
// WLAN passwd
String s_pw = "";
const char* password;
// The Topics to subscribe to
const char* subTopics[255];
String s_subTopics[255];
int topics = 0;
// von Beginn an ist der ESP erst einmal nicht konfiguriert.
boolean configured = false;
// Diese Funktion wird aufgerufen wenn etwas neues an ein abonniertes
// Thema gesendet wird.
void callback(char* topic, byte* payload, unsigned int length) {
	Serial.print(topic);
	Serial.print(":");
	// Einmal die ganze Payload durcharbeiten...
	for(int i = 0; i < length; i++) {
		// und Seriell ausgeben
		Serial.print((char)payload[i]);
	}
	// Am ende Newline.
	Serial.println("");
}

WiFiClient wifiClient;
PubSubClient client(mqtt_server, 1883, callback, wifiClient);

void setup() {
	// Output auf 115200 baud
	Serial.begin(115200);
	// Konfiguration starten.
	configure();
}

void loop() {
	// wenn WLAN und MQTT nicht verbunden dann neue connecten
	if (!client.connected() && WiFi.status() == 3) {
		reconnect();
	}
	// Hier kommt man erst hin wenn die Verbindung steht und wenn man hier ankommt
	// dann einfach nur die Verbindung halten.
	client.loop();
	// Dieses warten braucht der ESP anscheinend fuer die WLAN-Verwaltung.
	delay(20); 
	
	// maybe someone wants to publish something so lets check the serial line:
	serial_publish();
}



void reconnect() {
	// Wenn nicht verbunden dann wieder verbinden
	if(WiFi.status() != WL_CONNECTED){
		Serial.print("<DBG> Connecting to ");
		Serial.println(ssid);
		// Das verbinden geschieht durch simples warten
		while (WiFi.status() != WL_CONNECTED) {
			delay(500);
		}
		
		//print out some more debug once connected
		Serial.println("<DBG> WLAN connected");  
		Serial.print("<DBG> IP address: ");
		Serial.println(WiFi.localIP());
	}

	// Wenn wir verbunden sind dann soll es losgehen mit dem herstellen der
	// MQTT Verbindung
	if(WiFi.status() == WL_CONNECTED){
		Serial.print("<DBG> Attempting MQTT connection to: ");
		Serial.println(mqtt_server);
		client.setServer(mqtt_server, 1883);
		//if connected, subscribe to the topic(s) we want to be notified about
		if (client.connect(WiFi.localIP().toString().c_str())) {
			Serial.println("<DBG> MQTT Connected");
			for(int i = 0; i < topics; i++) {
				client.subscribe(subTopics[i]);
			}
		}
	}
}
// this method configures the ESP, once it is done it is never called again.
void configure() {
	// Debug Output
	Serial.println("<DBG> need to be configured.");
        Serial.println("<DBG> Config me like this:<delim><mqtt_server><delim><ssid><delim><pw><delim><topic><delim><topic>");
        // The whole config string will be collected into this one and after that it will be divided into its subparts
	String config = "";
	// just run if you need to
        while(configured == false) {
        	// just read Data if it is available
		while(Serial.available()) {
			// read in tmpvar
			char c = (char) Serial.read();
			// if it is the end of the config (new line), then start parsing, if not just concat( see else branch at the bottom)
			if(c=='\r') {
				// set configured, so it is not run again
				configured = true;
				// the delimcount starts at one because the first char has to be one
				int delimcount = 1;
				// this number tells us the position in the config string where the first char describing topics is
				int startOfTopics = 0;
				// as i said first char is delim
				char delim = config.charAt(0);
				// parse every little char start with 1 because 0 is delim
				for(int i = 1; i < config.length(); i++) {
					// either it is the delim
					if(config.charAt(i) == delim) {
						// increment the delims
						delimcount++;
						// after for delims every char belongs to a topicname
						if(delimcount == 4) {
							startOfTopics = i + 1;
						}
					// or it is something else
					} else {
						// between 1 and to append to the ip
						if(delimcount == 1) {
							s_mqtt_server.concat(config.charAt(i));
						} // between 2 and three to the ssid
						if(delimcount == 2) {
							s_ssid.concat(config.charAt(i));
						} // and between 3 and for to the pw
						if(delimcount == 3) {
							s_pw.concat(config.charAt(i));
						}
					}
				}
				// write pointers.	
				mqtt_server=s_mqtt_server.c_str();
				ssid=s_ssid.c_str();
				password=s_pw.c_str();


				// the amount of topics is the delimiters - 3 for ip, ssid and pw
				topics = delimcount - 3;
				if(topics < 1) { // you need to subscribe to at least one topic or
					Serial.println("<DBG> Sorry you subscribed to no topic.");
					// the esp will reboot
					abort();
				}
				// parsing of the topics
				// array of stringbuffers
				// start at topic zero
				int topiccounter = 0;
				// for all chars starting at "startOfTopics"
				for(int i = startOfTopics; i < config.length(); i++) {
					// if delim
					if(config.charAt(i) == delim) {
						// write to array
						subTopics[topiccounter] = s_subTopics[topiccounter].c_str();
						// and use next index
						topiccounter++;

					} else {
						// if ordinary char just append
						s_subTopics[topiccounter].concat(config.charAt(i));
					}
				}
				// last one has to be done manually because there is no delim behind it.
				subTopics[topics - 1] = s_subTopics[topics - 1].c_str();
				// Print a bunch of debug stuff
				Serial.println("<DBG> System configured. Here is your config: ");
				Serial.print("<DBG> MQTT-Server: ");
				Serial.println(mqtt_server);
				Serial.print("<DBG> SSID: ");
				Serial.println(ssid);
				Serial.print("<DBG> pw: ");
				Serial.println(s_pw);
				Serial.print("<DBG> You subscribed to ");
				Serial.print(topics);
				Serial.println(" topics.");
				for(int i = 0; i < topics; i++) {
					Serial.print("<DBG> ");
					Serial.println(subTopics[i]);
				}
				
				// WLAN anschalten und verbinden
        WiFi.disconnect(true);
				WiFi.begin(ssid, password);
        WiFi.mode(WIFI_STA);
				delay(2000);
				reconnect();
			// if the current char was not '\r' just concat the char and move on.	
			} else {
				config.concat(c);
			}
		}
        }
}
void serial_publish() {
	if(Serial.available()) {
    
		// create Buffer:
		String buf = "";
		String topic = "";
		String message = "";
		char delim;
		// get the whole line
		while(Serial.available()) {
			// append
			char c = Serial.read();
			// process the lne after return
			if(c == '\r') {
				// delim is char 0
				char delim = buf.charAt(0);
				int delimcount = 1;
				// process the whole message
				for(int i = 1; i < buf.length(); i++) {
					// if delim
					if(buf.charAt(i) == delim) {
						// inc delimcount
						delimcount++;
					}
					else {
						//if topic part 
						if(delimcount == 1) {
							topic.concat(buf.charAt(i));
						} else {
						// everything else belongs to the message
							message.concat(buf.charAt(i));

						}
					}
				}
				// do a loop and publish
				if(client.loop() &&  client.publish(topic.c_str(), message.c_str())) {
					Serial.println("<DBG> Successfully sent your message.");
				} else {
					Serial.println("<DBG> Sorry, wasn't able to send your message.");
				}
			}
			// if char != return concat to buffer
			else {
				buf.concat(c);
			}
     delay(1);
		}
	}
}
