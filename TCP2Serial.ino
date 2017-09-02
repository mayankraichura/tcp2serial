
//Uncomment below line to get debug data on Serial port
//#define DEBUG 1

//Uncomment below line to use the device as Access Point. Commenting it out will default to infrastructure wifi.
#define APMODE

#include <ESP8266WiFi.h>


#ifdef APMODE
//Please update SSID and password for AP mode
const char* ssid = "TCP2Serial";
const char* password = "tcp2serial";
#else

//Please update SSID and password for Infrastructure mode WiFi
const char* ssid = "****************";
const char* password = "****************";
#endif


const uint8_t channel = 11;	//WiFi channel. You can use Android app, WiFi analyzer to find a clear channel

const uint8_t TOGGLEPIN = 4; //The pin that will be toggled when a client is connected to this device



WiFiServer server(23); //Create an instance of a TCP server
WiFiClient client;	   //This variable will store the instance currently connected to this device

//Gets whether a client is connected or not!
bool isConnected(){
	return (client && client.connected());
}


//Toggles a digital pin for 200ms
void newConnection(){
	digitalWrite(TOGGLEPIN, HIGH);
	delay(200);
	digitalWrite(TOGGLEPIN, LOW);
}

void setup() {
	pinMode(TOGGLEPIN, OUTPUT);

	//Setup UART
	Serial.begin(1200);		//1200 bps serial

#if DEBUG
	Serial.setDebugOutput(true);	//Enable debug dump to Serial

	//The below code will toggle the toggle pin for 1 sec. This is just for debug purpose and may be removed
	digitalWrite(TOGGLEPIN, HIGH);
	delay(1000);
	digitalWrite(TOGGLEPIN, LOW);
#endif

	
	


#ifdef APMODE
	//The below version is for AP without password
	//WiFi.softAP(ssid);


	//Use the below version to password protect AP
	WiFi.softAP(ssid, password, channel);
#else
#if DEBUG
	Serial.println();
	Serial.println();
	Serial.print("Connecting to ");
	Serial.println(ssid);

#endif
	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
#if DEBUG
		Serial.print(".");
#endif
	}

#if DEBUG
	Serial.println("");
	Serial.println("WiFi connected");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());
#endif
#endif


	//Setup server
	server.begin();
	server.setNoDelay(true);

#if DEBUG
	WiFi.printDiag(Serial);
#endif
}


void loop() {
	if (!isConnected()){
		if (server.hasClient()){
			client = server.available();
			newConnection();
		}
	}


	if (!isConnected()){
		//We don't have any client connected so exit the loop()
		return;
	}
	else{
		//Hurray!!! We have a connected client. 
		
		
		//If there are any other awaiting in the queue, reject them.
		if (server.hasClient()){
			while (server.hasClient()) server.available().stop();
		}

		yield();		//Allow low level WiFi processing.

		//Let us check if there is any data available on WiFi channel
		if (isConnected()){
			if (client.available()){
				//If so, read from it and dump it on Serial channel
				while (client.available()) {
					Serial.write(client.read());
					yield();	//Not sure if it's needed.
				}
			}
		}


		//Let us check if there is any data available on Serial channel
		if (Serial.available()){
			//If so, read it to the memory
			size_t slen = Serial.available();
			uint8_t sbuff[slen];  
			Serial.readBytes(sbuff, slen);


			//If we are still connected to the client, dump data to it's stream.
			if (isConnected()) client.write(sbuff, slen);
		}
	}
}
