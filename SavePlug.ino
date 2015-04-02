#include <TextFinder.h>
#include <ArduinoJson.h>


// Libraries
#include <Time.h>
#include <TimeAlarms.h>
#include <Thermistor.h>
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <DHT.h>
#include <EEPROM.h>
#include <SAVEEPROM.h>


// I/O CONFIGURATION //////
// Digital
#define RELAYPIN 8
#define BUZZERPIN 10
#define DHTPIN 2
#define DHTTYPE DHT22
// Analogic
#define CURRENTPIN 1
#define VOLTAGEPIN 3
#define TEMPERATUREPIN 0
#define LIGHTPIN 2
//////////////////////////


// Timming
#define READPERIOD 2

// SAVEPLUG CONFIGURATON
//#define SAVEID 
#define SAVEFIRM String(EEPROM.read(11) + EEPROM.read(12) + EEPROM.read(13) + EEPROM.read(14) + EEPROM.read(15) + EEPROM.read(16) + EEPROM.read(17) + EEPROM.read(18) + EEPROM.read(19) + EEPROM.read(20))

// HTTP SERVER CONFIGURATION
#define HTTPPORT 85

// UDP LISTEN PORT
#define UDPPORT 8888

// CURRENT SENSOR
#define SAMPLETIME 100000UL      // sample over 100ms, it is an exact number of cycles for both 50Hz and 60Hz mains
#define NUMSAMPLES 250UL      // choose the number of samples to divide sampleTime exactly, but low enough for the ADC to keep up

// DEVICE PROPERTIES INICIALIZATION
/* Protocol Properties */
char deviceType[10] = "savePlug";
char deviceName[20] = "Abajur";
char deviceFirmware[10] = "SAVEFIRM";
char savenergyId[11] = "SN12345678";
char deviceLocation[20] = "quarto";
char deviceOwnerName[30] = "Leonardo Soliz Encinas";
char deviceOwnerEmail[30] = "leosoliz@gmail.com";
char devicePassword[10] = "J17pl71@";
float deviceCurrent = 0;
float deviceVoltage = 0;
float devicePower = 0;
float deviceEnergy = 0;
float deviceExternalTemperature = 0;
float deviceExternalHumidity = 0;
float deviceTemperature = 0;
unsigned int deviceTimeOn = 0;
unsigned int deviceLight = 0;
String deviceTime;
String deviceDate;
int deviceOn = 1;
int activeCalendar=0;
int alert[20];
StaticJsonBuffer<512> jsonBuffer;
//byte savEnergyServer[] = {208,113,220,172};
const char savEnergyServer[] = "www.savenergies.com";
const String prefixAlert = "GET /crypta.php?action=androidnotification&username=";
const String prefixLogin = "GET /crypta.php?action=androidlogin&username=";

/* Class Objects */
// Time
time_t timestamp = 0;
unsigned long currentMillis = 0;
unsigned long doneMeasurement = 0;
unsigned long doneWebServer = 0;
unsigned long doneSafety = 0;
unsigned long doneWebClient = 0;
unsigned long doneNTPSync = 0;

//HTTP
String httpBuffer = "";
EthernetServer server = EthernetServer(HTTPPORT);
EthernetClient client;
byte MAC[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };


// NTP
IPAddress timeServer(200, 160, 0, 8);
const int NTP_PACKET_SIZE= 48;
byte packetBuffer[ NTP_PACKET_SIZE];
EthernetUDP Udp;

//SENSORS
DHT dht(DHTPIN, DHTTYPE);
Thermistor temp(TEMPERATUREPIN);

//EEPROM
SAVEEPROM plugMemory;

// START UP ACTIONS
void setup(){
    // Initialize I/O
    
    // Sensors
    dht.begin();
    analogReference(EXTERNAL);
    // Serial
    Serial.begin(9600);
    
    // Ethernet
    Ethernet.begin(MAC);
    Serial.print("Local IP: ");
    Serial.println(Ethernet.localIP());
    Measurements();
	NTPSync();
}

/* Loop method */
void loop(){
	
	currentMillis = millis();
	if (currentMillis - doneMeasurement >= READPERIOD * 1000){
		Measurements();
		Alarms();
		Actions();
		doneMeasurement = currentMillis;
		//Serial.println("Measure - OK");
	}
	if (currentMillis - doneWebServer >= 800UL){
		WebServer();
		doneWebServer = currentMillis;
		//Serial.println("WebServer - OK");
	}
	if (currentMillis - doneSafety >= 100UL){
		Safety();
		doneSafety = currentMillis;
		//Serial.println("Safety - OK");
	}
	if (currentMillis - doneWebClient >= 30000UL){
		WebClient();
		doneWebClient = currentMillis;
		//Serial.println("WebClient - OK");
	}
	if (currentMillis - doneNTPSync >= 3600000UL){
		NTPSync();
		doneNTPSync = currentMillis;
		//Serial.println("NTPSync - OK");
	}
	
}

///////////// STATES OF THE MACHINE ////////////
// Read all available measurements
void Measurements(){
	 // Read Measurements
 	deviceCurrent = computeCurrent(CURRENTPIN);
	deviceVoltage = computeVoltage(VOLTAGEPIN);
 	devicePower = deviceCurrent * deviceVoltage;
 	deviceEnergy = deviceEnergy + devicePower * READPERIOD / 3600;
 	deviceTemperature = temp.getTemp();
 	deviceTimeOn = computeTimeOn(deviceCurrent, deviceTimeOn, READPERIOD);
 	deviceLight = analogRead(LIGHTPIN);
 	deviceExternalTemperature = dht.readTemperature();
 	deviceExternalHumidity = dht.readHumidity();
}

// Handling device Alarms
void Alarms(){
	String alarm;
	for (int i=0; i<20; i++){
		alarm = plugMemory.getAlarm(i);

		// CHECK IF ALARM IS ENABLED
		if (alarm.substring(0,1).equals("1")){

			// CHECK IF IS A DAYLY GOAL
			if (alarm.substring(11).equals("0")){
				// CHECK IF DAYLY ALARM IS ON
				if (plugMemory.getDayHistory(day(now())-1) > plugMemory.getDayGoal() * alarm.substring(10,11).toInt()){

					// CHECK IF ALARM TYPE IS SWITCH OFF DEVICE
					if (alarm.substring(9,10).equals("0")){
						deviceOn = 0;
					 	digitalWrite(RELAYPIN, deviceOn);
					}
					// ELSE SIGN ALERT TO HTTP REQUEST HANDLER
					else{
     					alert[i] = 1;
					}
				}
				// DAYLY ALARM IS OFF
				else{
					alert[i] = 0;
				}
			}
			
			// CHECK IF IS A WEEKLY GOAL
			if (alarm.substring(11).equals("1")){
				unsigned long week = day(now());
				week = (unsigned long) week / 7;
				
				// CHECK IF WEEKLY ALARM IS ON
				if (plugMemory.getWeekHistory((int) week) > plugMemory.getWeekGoal() * alarm.substring(10,11).toInt()){
				
					// CHECK IF ALARM TYPE IS SWITCH OFF DEVICE
					if (alarm.substring(9,10).equals("0")){
						deviceOn = 0;
					 	digitalWrite(RELAYPIN, deviceOn);
					}
					// ELSE SIGN ALERT TO HTTP REQUEST HANDLER
					else{
     					alert[i] = 2;
					}
				}
				// WEEKLY ALARM IS OFF
				else{
					alert[i] = 0;
				}
			}
			
			// CHECK IF IS A MONTHLY GOAL
			if (alarm.substring(11).equals("2")){
				// CHECK IF WEEKLY ALARM IS ON
				if (plugMemory.getMonthHistory(month(now())) > plugMemory.getMonthGoal() * alarm.substring(10,11).toInt()){
					
					// CHECK IF ALARM TYPE IS SWITCH OFF DEVICE
					if (alarm.substring(9,10).equals("0")){
						deviceOn = 0;
					 	digitalWrite(RELAYPIN, deviceOn);
					}
					// ELSE SIGN ALERT TO HTTP REQUEST HANDLER
					else{
     					alert[i] = 3;
					}
				}
				// MONTHLY ALARM IS OFF
				else{
					alert[i] = 0;
				}
			}
		}
	}
}

// Handling device Actions
void Actions(){
	String event;
	for (int i=0; i<40; i++){
		event = plugMemory.getEvent(i);
		
		// CHECK IF EVENT IS ENABLED
		if (event.substring(0,1).equals("1")){
			
			// CHECK TIME TO START THE EVENT
			if (timestamp - event.substring(9,13).toInt() >= 0){
				deviceOn = 1;
				digitalWrite(RELAYPIN, deviceOn);

				// CHECK TIME TO STOP THE EVENT
				if(timestamp < event.substring(9,13).toInt() + event.substring(13,17).toInt()){
					deviceOn = 0;
					digitalWrite(RELAYPIN, deviceOn);
					unsigned long newTime= 0;
					int repetitions = -1;
					bool enabled = false;
					
					// CHECK FOR REPEAT ALWAYS
					if (event.substring(18,19).equals("-1")){
						newTime = event.substring(9,13).toInt() + 3600L * event.substring(19).toInt();
						enabled = true;
						repetitions = -1;
					}
					// CHECK FOR NO REPETITION
					if (event.substring(18,19).equals("0")){
						newTime = event.substring(9,13).toInt();
						enabled = false;
						repetitions = 0;
					}
					
					// CHECK FOR REPETITIONS
					else{
						newTime = event.substring(9,13).toInt() + 3600L * event.substring(19).toInt();
						enabled = true;
						repetitions = event.substring(18,19).toInt() - 1;
					}
					// WRITE PLUG EEPROM WITH UPDATED EVENT
					plugMemory.setEvent(i, enabled, event.substring(1,9), newTime, (unsigned long) event.substring(13,17).toInt(), (bool) event.substring(17,18).toInt(), repetitions, event.substring(19).toInt());	
				}
				
			}
			
			// IF NOT TIME THEN TURN OFF DEVICE PLUGGED
			else{
				deviceOn = 0;
				digitalWrite(RELAYPIN, deviceOn);
			}	
		}
	}
}

// WEB SERVER TO HANDLE COMMUNICATION PROTOCOL
void WebServer(){

	// Reading ethernet command
	EthernetClient client = server.available();
	if (client) {
		TextFinder finder(client);
		while (client.connected()) {
			if (client.available()) {
				if (finder.find("GET /?action")){
					char cAction[9];
					finder.getString("=", "&", cAction, 9);

					// Request device status
					if (String(cAction) == "getStatu"){
						JsonObject& data = jsonBuffer.createObject();
						data["saveid"] = savenergyId;
						data["type"] = deviceType;
						data["name"] = deviceName;
						data["location"] = deviceLocation;
						IPAddress localIP = Ethernet.localIP();
						char cLocalIP[20];
					 	sprintf(cLocalIP, "%u.%u.%u.%u", localIP[0], localIP[1], localIP[2], localIP[3]);
						data["ipAddress"] = cLocalIP;
						String date = getCurrentDate();
						char cDate[11];
						date.toCharArray(cDate, 11); 
						data["date"] = cDate;
						String sTime = getCurrentTime();
						char cTime[9];
						sTime.toCharArray(cTime, 9);
						data["time"] = cTime;
						data["ownerName"] = deviceOwnerName;
						data["ownerEmail"] = deviceOwnerEmail;
						data["switchStatus"] = deviceOn;
						data["current"] = deviceCurrent;
						data["voltage"] = deviceVoltage;
						data["power"] = devicePower;
						data["consumption"] = deviceEnergy;
						data["timeOn"] = deviceTimeOn;
						data["temperature"] = deviceTemperature;
						data["externalTemp"] = deviceExternalTemperature;
						data["externalHumidity"] = deviceExternalHumidity;
						data["ambientLight"] = deviceLight;
						client.println("HTTP/1.1 200 OK");
         				client.println("Content-Type: application/json");
         				client.println();
						data.prettyPrintTo(client);
						Serial.println("New status requested.");
					}
        
					// Set device switch
					if (String(cAction) == "setSwitc"){
					 finder.findUntil("status=", "\n\r");
					 deviceOn = finder.getValue();
					 digitalWrite(RELAYPIN, deviceOn);
					 JsonObject& data = jsonBuffer.createObject();
					 data["saveId"] = savenergyId;
					 data["switchStatus"] = deviceOn;
					 data["response"] = "ACK";
					 client.println("HTTP/1.1 200 OK");
         			 client.println("Content-Type: application/json");
         			 client.println();
					 data.prettyPrintTo(client);
					 Serial.println("New switch status: " + String(deviceOn));
					}

					// Get device Event
					if (String(cAction) == "getEvent"){
						finder.findUntil("eventNumber", "\n\r");
						int eventNumber = finder.getValue();
						Serial.println(eventNumber);
						String result = plugMemory.getEvent(eventNumber);
						Serial.println(result);
						Serial.println("ok");
						JsonObject& data = jsonBuffer.createObject();
					 	data["eventNumber"] = result[0];
					 	data["isEnabled"] = result[1];
					 	
					 	char *cEvent;
					 	result.substring(2,10).toCharArray(cEvent, 10);
					 	data["eventName"] = cEvent;
					 	data["timestamp"] = "ACK";
					 	data["duration"] = "ACK";
					 	data["isSwitched"] = "ACK";
					 	data["repetitions"] = "ACK";
					 	data["repetitionType"] = "ACK";
					 	client.println("HTTP/1.1 200 OK");
         			 	client.println("Content-Type: application/json");
         			 	client.println();
					 	data.prettyPrintTo(client);
					 	Serial.println("Event requested: " + deviceTime);
					}
					// Set device Event
					if (String(cAction) == "setEvent"){
					 //Serial.println(cAction);
					 finder.findUntil("eventNumber=", "&");
					 int eventNumber = finder.getValue();
					 //Serial.println(eventNumber);
					 finder.findUntil("isEnabled=", "&");
					 bool isEnabled = finder.getValue();
					 //Serial.println(isEnabled);
					 finder.findUntil("eventName", "&");
					 char cEvent[9];
					 finder.getString("=", "&", cEvent, 9);
					 String eventName = String(cEvent);
					 //Serial.println(String(eventName));
					 finder.findUntil("timestamp=", "&");
					 unsigned long unixTime = finder.getValue();
					 Serial.println(unixTime);
					 finder.findUntil("duration=", "&");
					 unsigned long duration = finder.getValue();
					 finder.findUntil("isSwitched=", "&");
					 bool isSwitched = finder.getValue();
					 finder.findUntil("repetitions=", "&");
					 int repetitions = finder.getValue();
					 finder.findUntil("repetitionType=", "&");
					 int repetitionType = finder.getValue();
					 Serial.println(String(repetitionType));
					 JsonObject& data = jsonBuffer.createObject();
					 data["saveId"] = savenergyId;
					 data["eventNumber"] = eventNumber;
					 data["eventName"] = cEvent;
					 data["response"] = "ACK";
					 client.println("HTTP/1.1 200 OK");
         			 client.println("Content-Type: application/json");
         			 client.println();
					 data.prettyPrintTo(client);
					 plugMemory.setEvent( eventNumber, isEnabled, eventName, unixTime, duration, isSwitched, repetitions, repetitionType);
					 Serial.println("New event set: " + eventName);
					}

					// Set device Alarm
					if (String(cAction) == "setAlarm"){
						finder.findUntil("alarmNumber", "&");
						int alarmNumber = finder.getValue();
						finder.findUntil("isEnabled", "&");
						int isEnabled = finder.getValue();
						finder.findUntil("alarmName", "&");
						char cAlarm[9];
						finder.getString("=", "&", cAlarm, 9);
						String alarmName = cAlarm;
						finder.findUntil("alarmType", "&");
						int alarmType = finder.getValue();
						finder.findUntil("alarmThreshold", "&");
						int alarmThreshold = finder.getValue();
						finder.findUntil("alarmVariable", "&");
						int alarmVariable = finder.getValue();
						plugMemory.setAlarm(alarmNumber, isEnabled, alarmName, alarmType, alarmThreshold, alarmVariable);
						JsonObject& data = jsonBuffer.createObject();
					 	data["saveId"] = savenergyId;
					 	data["alarmNumber"] = alarmNumber;
					 	data["alarmName"] = cAlarm;
					 	data["response"] = "ACK";
					 	client.println("HTTP/1.1 200 OK");
         			 	client.println("Content-Type: application/json");
         			 	client.println();
					 	data.prettyPrintTo(client);
						Serial.println("New alarm set: " + alarmName);
					}
					
					// Set device Daily Goal
					if (String(cAction) == "setDaily"){
						finder.findUntil("goal=", "\n\r");
						unsigned long dayGoal = finder.getValue();
						plugMemory.setDayGoal(dayGoal);
						Serial.println(dayGoal);
						JsonObject& data = jsonBuffer.createObject();
					 	data["saveId"] = savenergyId;
					 	data["dailyGoal"] = dayGoal;
					 	data["response"] = "ACK";
					 	client.println("HTTP/1.1 200 OK");
         			 	client.println("Content-Type: application/json");
         			 	client.println();
					 	data.prettyPrintTo(client);
					 	Serial.println("New daily goal set: " + String(dayGoal));
					}
					
					// Set device Weekly Goal
					if (String(cAction) == "setWeekl"){
						finder.findUntil("goal=", "\n\r");
						unsigned long weekGoal = finder.getValue();
						plugMemory.setWeekGoal(weekGoal);
						Serial.println(weekGoal);
						JsonObject& data = jsonBuffer.createObject();
					 	data["saveId"] = savenergyId;
					 	data["weeklyGoal"] = weekGoal;
					 	data["response"] = "ACK";
					 	client.println("HTTP/1.1 200 OK");
         			 	client.println("Content-Type: application/json");
         			 	client.println();
					 	data.prettyPrintTo(client);
					 	Serial.println("New weekly goal set: " + String(weekGoal));
					}
					
					// Set device Monthly Goal
					if (String(cAction) == "setMonth"){
						finder.findUntil("goal=", "\n\r");
						unsigned long monthGoal = finder.getValue();
						plugMemory.setMonthGoal(monthGoal);
						Serial.println(monthGoal);
						JsonObject& data = jsonBuffer.createObject();
					 	data["saveId"] = savenergyId;
					 	data["monthlyGoal"] = monthGoal;
					 	data["response"] = "ACK";
					 	client.println("HTTP/1.1 200 OK");
         			 	client.println("Content-Type: application/json");
         			 	client.println();
					 	data.prettyPrintTo(client);
					 	Serial.println("New monthly goal set: " + String(monthGoal));
					}
						
					// Set device location
					if (String(cAction) == "setLocat"){
					 finder.findUntil("location", "\n\r");
					 finder.getString("=", " HTTP/1.1", deviceLocation, 20);
					 String sLocation = deviceLocation;
					 sLocation.replace("%20", " ");
					 sLocation.toCharArray(deviceLocation, 20);
					 plugMemory.setDeviceLocation(String(deviceLocation));
					 JsonObject& data = jsonBuffer.createObject();
					 data["saveId"] = savenergyId;
					 data["location"] = deviceLocation;
					 data["response"] = "ACK";
					 client.println("HTTP/1.1 200 OK");
         			 client.println("Content-Type: application/json");
         			 client.println();
					 data.prettyPrintTo(client);
					 Serial.println("New device location: " + String(deviceLocation));
					}

					// Set owner name
					if (String(cAction) == "setOwner"){
						finder.findUntil("name", "\n\r");
						finder.getString("=", " HTTP/1.1", deviceOwnerName, 30);
						String sOwner = deviceOwnerName;
						sOwner.replace("%20", " ");
						sOwner.toCharArray(deviceOwnerName, 30);
						plugMemory.setOwnerName(String(deviceOwnerName));
						JsonObject& data = jsonBuffer.createObject();
					 	data["saveId"] = savenergyId;
					 	data["ownerName"] = deviceOwnerName;
					 	data["response"] = "ACK";
					 	client.println("HTTP/1.1 200 OK");
         			 	client.println("Content-Type: application/json");
         			 	client.println();
					 	data.prettyPrintTo(client);
					 	Serial.println("New owner name: " + String(deviceOwnerName));
					}
					
					// Set owner email
					if(String(cAction) == "setEmail"){
					 	finder.findUntil("email", "\n\r");
					 	finder.getString("=", " HTTP/1.1", deviceOwnerEmail, 30);
					 	String sEmail = deviceOwnerEmail;
					 	sEmail.replace("%20", " ");
					 	sEmail.toCharArray(deviceOwnerEmail, 30);
					 	plugMemory.setOwnerEmail(String(deviceOwnerEmail));
					 	JsonObject& data = jsonBuffer.createObject();
					 	data["saveId"] = savenergyId;
					 	data["ownerEmail"] = deviceOwnerEmail;
					 	data["response"] = "ACK";
					 	client.println("HTTP/1.1 200 OK");
         			 	client.println("Content-Type: application/json");
         			 	client.println();
					 	data.prettyPrintTo(client);
					 	Serial.println("New owner email: " + String(deviceOwnerEmail));
					}
					
					// Set device name
					if(String(cAction) == "setName"){
						finder.findUntil("name", "\n\r");
						finder.getString("=", " HTTP/1.1", deviceName, 20);
						String sName = deviceName;
						sName.replace("%20", " ");
						sName.toCharArray(deviceName, 20);
						plugMemory.setDeviceName(String(deviceName));
						JsonObject& data = jsonBuffer.createObject();
					 	data["saveId"] = savenergyId;
					 	data["name"] = deviceName;
					 	data["response"] = "ACK";
					 	client.println("HTTP/1.1 200 OK");
         			 	client.println("Content-Type: application/json");
         			 	client.println();
					 	data.prettyPrintTo(client);
					 	Serial.println("New device name: " + sName);
					}    
					
					// Set device date
					if(String(cAction) == "setDate"){
						finder.findUntil("date", "\n\r");
						int newYear = finder.getValue();
						int newMonth = finder.getValue();
						int newDay = finder.getValue();
					 	setTime(hour(), minute(), second(), newDay, newMonth, newYear);
					 	deviceDate = getCurrentDate();
					 	char cDate[11];
					 	deviceDate.toCharArray(cDate, 11);
					 	JsonObject& data = jsonBuffer.createObject();
					 	data["saveId"] = savenergyId;
					 	data["date"] = cDate;
					 	data["response"] = "ACK";
					 	client.println("HTTP/1.1 200 OK");
         			 	client.println("Content-Type: application/json");
         			 	client.println();
					 	data.prettyPrintTo(client);
					 	Serial.println("New device date: " + deviceDate);
					}

					// Set device time
					if(String(cAction) == "setTime"){
						finder.findUntil("time", "\n\r");
						int newHour = finder.getValue();
						int newMinute = finder.getValue();
						int newSecond = finder.getValue();
					 	setTime(newHour, newMinute, newSecond, day(), month(), year());
					 	deviceTime = getCurrentTime();
					 	char cTime[9];
					 	deviceTime.toCharArray(cTime, 9);
					 	JsonObject& data = jsonBuffer.createObject();
					 	data["saveId"] = savenergyId;
					 	data["time"] = cTime;
					 	data["response"] = "ACK";
					 	client.println("HTTP/1.1 200 OK");
         			 	client.println("Content-Type: application/json");
         			 	client.println();
					 	data.prettyPrintTo(client);
					 	Serial.println("New device time: " + deviceTime);
					}    
					        
					// Discovery all parameters
					if(String(cAction) == "discover"){
						JsonObject& data = jsonBuffer.createObject();
					 	data["saveId"] = savenergyId;
					 	data["deviceType"] = deviceType;
					 	IPAddress localIP = Ethernet.localIP();
					 	char cLocalIP[20];
					 	sprintf(cLocalIP, "%u.%u.%u.%u", localIP[0], localIP[1], localIP[2], localIP[3]);
					 	data["ipAddress"] = cLocalIP;
					 	char cDate[11];
					 	deviceDate = getCurrentDate();
					 	deviceDate.toCharArray(cDate, 11);
					 	data["date"] = cDate;
					 	char cTime[9];
					 	deviceTime = getCurrentTime();
					 	deviceTime.toCharArray(cTime, 9);
					 	data["time"] = cTime;
					 	data["name"] = deviceName;
					 	data["location"] = deviceLocation;
					 	data["response"] = "ACK";
					 	client.println("HTTP/1.1 200 OK");
         			 	client.println("Content-Type: application/json");
         			 	client.println();
					 	data.prettyPrintTo(client);
					 	Serial.println("New device found: " + String(savenergyId));
					}

					// Reset command string and close connection with client

					client.stop();
				}
			}
		}
	}
}

// Handling device Safety
void Safety(){
	
}

// HTTP REQUESTS TO SAVENERGY SERVER HANDLER
void WebClient(){
	EthernetClient clientWeb;
	String buffer = "";
  	if (clientWeb.connect(savEnergyServer, 80)) {
    	//Serial.println("connected");
    	clientWeb.println(prefixLogin + deviceOwnerEmail + "&password=" + devicePassword + " HTTP/1.1");
    	clientWeb.println("Host: savenergies.com");
    	clientWeb.println("User-Agent: Arduplug/2.0");
    	clientWeb.println();
  	}
  	else {
    	//Serial.println("connection failed");
    	//Serial.println();
  	}
	while(!clientWeb.available()) delay(1); //waits for data    
	while(clientWeb.connected() && !buffer.endsWith("}")){  	
    	char c = clientWeb.read();
    	buffer = buffer + c;
    	if (buffer.endsWith("}")){
    		//Serial.print(buffer);
    		int initPos = buffer.indexOf("email") + 8;
    		int endPos = buffer.indexOf("message") - 3;
    		//Serial.println(buffer.substring(initPos,endPos));
    		if (buffer.substring(initPos,endPos).equals(deviceOwnerEmail)){
    			Serial.println("Login OK!");
    			//alert[2] = 1;
    			//alert[4] = 2;
    			for (int i=0; i<20; i++){
    				if (alert[i] > 0){
    					//Serial.println(i);
    					clientWeb.println(prefixAlert + deviceOwnerEmail + "&notification=" + String(alert[i]) + " HTTP/1.1");
    					clientWeb.println("Host: savenergies.com");
    					clientWeb.println("User-Agent: Arduplug/2.0");
    					clientWeb.println();
    					buffer = "";
    					while(!clientWeb.available()) delay(1); //waits for data
    					while(!buffer.endsWith("}")){
    						char h = clientWeb.read();
    						buffer = buffer + h;
    					}
    					buffer = buffer.substring(buffer.indexOf("{"));
    					initPos = buffer.indexOf("username") + 11;
    					endPos = buffer.indexOf("notification") - 3;
    					if (buffer.substring(initPos, endPos).equals(deviceOwnerEmail)){
    						Serial.println("Alert " + String(i) + " sent.");
    					}
    				}
    			}	
    		}
   		}
    		
    }
    
   // Serial.println(buffer);
    Serial.println();
  	Serial.println("disconnected");
    clientWeb.stop();
  	
}

// SYNC NTP SERVER TIME WITH DEVICE
void NTPSync(){
	Udp.begin(UDPPORT);
	sendNTPpacket(timeServer); // send an NTP packet to a time server

    // wait to see if a reply is available
  	delay(1000);  
  	if ( Udp.parsePacket() ) {  
    	// We've received a packet, read the data from it
    	Udp.read(packetBuffer,NTP_PACKET_SIZE);  // read the packet into the buffer

    	//the timestamp starts at byte 40 of the received packet and is four bytes,
    	// or two words, long. First, esxtract the two words:

    	unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    	unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);  
    	// combine the four bytes (two words) into a long integer
    	// this is NTP time (seconds since Jan 1 1900):
    	unsigned long secsSince1900 = highWord << 16 | lowWord;  
    	//Serial.print("Seconds since Jan 1 1900 = " );
    	//Serial.println(secsSince1900);               

    	// now convert NTP time into everyday time:
    	//Serial.print("Unix time = ");
    	// Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    	const unsigned long seventyYears = 2208988800UL;     
    	// subtract seventy years:
    	unsigned long epoch = secsSince1900 - seventyYears - 10800UL;  
    	// print Unix time:
    	//Serial.println(epoch);                               

		setTime(epoch);

    	// print the hour, minute and second:
    	Serial.print("Local time: ");       // UTC is the time at Greenwich Meridian (GMT)
    	Serial.print((epoch  % 86400L) / 3600); // print the hour (86400 equals secs per day)
    	Serial.print(':');  
    	if ( ((epoch % 3600) / 60) < 10 ) {
      		// In the first 10 minutes of each hour, we'll want a leading '0'
      		Serial.print('0');
    	}
    	Serial.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
    	Serial.print(':'); 
    	if ( (epoch % 60) < 10 ) {
      		// In the first 10 seconds of each minute, we'll want a leading '0'
      		Serial.print('0');
    	}
    	Serial.println(epoch %60); // print the second
  	}
}
/////////////////////////////////////////////////

//////////////// MEASUREMENT FUNCTIONS //////////
// COMPUTE DEVICE CURRENT IN AMPS
float computeCurrent(int analogPin){
  const unsigned long sampleInterval = SAMPLETIME/NUMSAMPLES;
  const int adc_zero = 510;
  unsigned long currentAcc = 0;
  unsigned int count = 0;
  unsigned long prevMicros = micros() - sampleInterval ;
  while (count < NUMSAMPLES)
  {
   if (micros() - prevMicros >= sampleInterval)
   {
     int adc_raw = analogRead(analogPin) - adc_zero;
     currentAcc += (unsigned long)(adc_raw * adc_raw);
     ++count;
     prevMicros += sampleInterval;
   }
  }
  float current = -0.07 + sqrt((float)currentAcc/(float)NUMSAMPLES) * (75.7576 / 1024.0);
  if (current < 0.015 ){
    current = 0;
  }
  return current;
}
// COMPUTE DEVICE VOLTAGE IN VOLTS
float computeVoltage(int analogPin){
  float voltage = random(220, 230);
  return voltage;
}
// COMPUTE DEVICE TIME ON
int computeTimeOn(float current, int timeOn, int period){
  if (current > 0.015){
    timeOn = timeOn + period;
  }
  return timeOn;
}
//////////////////////////////////////////////////

//////////////// TIME FUNCTIONS /////////////////
// GET CURRENT TIME IN HH:MM:SS FORMAT
String getCurrentTime(){
  String time;
  time = printDigits(hour()) + ":" + printDigits(minute()) + ":" + printDigits(second());
  return time;
}
// GET CURRENT DATE IN YYYY-MM-DD FORMAT
String getCurrentDate(){
  String date;
  date = String(year()) + "-" + String(printDigits(month())) + "-" + String(printDigits(day()));
  return date;
}
// PRINTS DECIMAL ZERO IN DATE AND TIME
String printDigits(int digits){
  String digital;
  if(digits < 10)
    digital = String(0) + String(digits);
   else
     digital = String(digits);
  return digital;
}
// send an NTP request to the time server at the given address 
unsigned long sendNTPpacket(IPAddress& address){
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE); 
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49; 
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:         
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer,NTP_PACKET_SIZE);
  Udp.endPacket(); 
}
//////////////////////////////////////////////////
