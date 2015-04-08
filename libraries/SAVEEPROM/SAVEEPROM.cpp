// Biblioteca para ler EEPROM do ArduPlug
	// Autor: Leonardo Soliz Encinas

// Bibliotecas utilizadas
#include<SAVEEPROM.h>
#include<EEPROM.h>

// MACROS
#define highWord(x) ((x) >> 16)
#define lowWord(x) ((x) & 0xffff)



// READ STRING FROM EEPROM
String SAVEEPROM::readEepromString(int initAddress, int stringLength){
	String value;
	for (int i=0; i < stringLength; i++){
		value[i] = EEPROM.read(i+initAddress);
	}
	return value;
}

// WRITE STRING TO EEPROM
void SAVEEPROM::writeEepromString(int initAddress, String value, int addressLength){
	for (int i=0; i < addressLength; i++){
		if (i < value.length())
		EEPROM.write(i+initAddress, value[i]);
		else
		EEPROM.write(i+initAddress, String("").toInt());
	}
}

// READ LONG FROM EEPROM
unsigned long SAVEEPROM::readEepromLong(int initAddress){
	unsigned long firstByte = EEPROM.read(0 + initAddress);
	unsigned long secondByte = EEPROM.read(1 + initAddress);
	secondByte << 8;
	unsigned long thirdByte = EEPROM.read(2 + initAddress);
	thirdByte << 16;
	unsigned long fourthByte = EEPROM.read(3 + initAddress);
	fourthByte << 24;
	unsigned long longNumber = fourthByte + thirdByte + secondByte + firstByte;
	return longNumber;
}

// WRITE LONG TO EEPROM
void SAVEEPROM::writeEepromLong(int initAddress, unsigned long value){
	EEPROM.write(initAddress, lowByte(lowWord(value)));
	EEPROM.write(1 + initAddress, highByte(lowWord(value)));
	EEPROM.write(2 + initAddress, lowByte(highWord(value)));
	EEPROM.write(3 + initAddress, highByte(highWord(value)));
}

// READ DOUBLE FROM EEPROM
unsigned int SAVEEPROM::readEepromDouble(int initAddress){
	return word(EEPROM.read(initAddress + 1), EEPROM.read(initAddress));
}

// WRITE DOUBLE TO EEPROM
void SAVEEPROM::writeEepromDouble(int initAddress, unsigned int value){
	EEPROM.write(initAddress, lowByte(value));
	EEPROM.write(1 + initAddress, highByte(value));
}

// READ IP ADDRESS FROM EEPROM
String SAVEEPROM::readEepromIp(int initAddress){
	String ip;
  	for (int i = 0; i < 4; i++){
    	ip[i] = EEPROM.read(i+initAddress);
      	if (i<3){
      	ip[i+1] = '.';
      	i++;
      	}
    }
  return ip;
}
void SAVEEPROM::writeEepromIp(int initAddress, String ip){
	for (int i = 0; i < 4; i++){
    EEPROM.write(i+initAddress, ip[2*i]);
	}
}
// PUBLIC METHODS

// CONSTRUCTOR
SAVEEPROM::SAVEEPROM(){
	
}

// GET-SET ReadPeriod
int SAVEEPROM::getReadPeriod(){
  int readPeriod = EEPROM.read(0);
  return readPeriod;
}
void SAVEEPROM::setReadPeriod( int readPeriod){
  EEPROM.write(0, readPeriod);
}

//GET-SET SAVEID
String SAVEEPROM::getSaveId(){
  String saveId;
  for (int i = 0; i < 10; i++){
    saveId[i] = EEPROM.read(i+1);
  }
  return saveId;
}
void SAVEEPROM::setSaveId( String saveId){
  for (int i = 0; i < 10; i++){
    EEPROM.write(i+1, saveId[i]);
  }
}

// GET-SET SAVEFIRM
String SAVEEPROM::getSaveFirm(){
  String saveFirm;
  for (int i = 0; i < 10; i++){
    saveFirm[i] = EEPROM.read(i+11);
  }
  return saveFirm;
}
void SAVEEPROM::setSaveFirm( String saveFirm){
  for (int i = 0; i < 10; i++){
    EEPROM.write(i+11, saveFirm[i]);
  }
}

// GET-SET DHCP
bool SAVEEPROM::isDHCPEnabled(){
  bool enabled = EEPROM.read(21);
  return enabled;
}
void SAVEEPROM::setDHCPEnabled(bool enabled){
  EEPROM.write(21, enabled);
}

// GET-SET IP
String SAVEEPROM::getDeviceIp(){
  return readEepromIp(22);
}
void SAVEEPROM::setDeviceIp(String ip){
    writeEepromIp(22, ip);
}

// GET-SET GATEWAY IP
String SAVEEPROM::getGatewayIp(){
  return readEepromIp(26);
}
void SAVEEPROM::setGatewayIp(String ip){
    writeEepromIp(26, ip);
}

// GET-SET SUBNET
String SAVEEPROM::getSubNet(){
	return readEepromIp(30);
}
void SAVEEPROM::setSubNet(String mask){
	writeEepromIp(30, mask);
}

// GET-SET HTTP PORT
int SAVEEPROM::getHttpPort(){
  return readEepromDouble(34);
}
void SAVEEPROM::setHttpPort( int httpPort){
  writeEepromDouble(34, httpPort);
}

// GET-SET NTP SERVER
String SAVEEPROM::getNtpServer(){
  String ip;
  for (int i = 0; i < 4; i++){
      ip[i] = EEPROM.read(i+36);
      if (i<3){
      ip[i+1] = '.';
      i++;
      }
    }
  return ip;
}
void SAVEEPROM::setNtpServer(String ip){
	    for (int i = 0; i < 4; i++){
    EEPROM.write(i+36, ip[2*i]);
  }
  
}

// GET-SET UDP PORT
int SAVEEPROM::getUdpPort(){
  return readEepromDouble(40);
}
void SAVEEPROM::setUdpPort( int udpPort){
  writeEepromDouble(40, udpPort);
}

// GET-SET DEVICE NAME
String SAVEEPROM::getDeviceName(){
	String deviceName = readEepromString(42,20);
	return deviceName;
}
void SAVEEPROM::setDeviceName(String deviceName){
	writeEepromString(42,deviceName,20);
}

// GET-SET DEVICE LOCATION
String SAVEEPROM::getDeviceLocation(){
	String location = readEepromString(62,20);
	return location;
}
void SAVEEPROM::setDeviceLocation(String location){
	writeEepromString(62, location, 20);
}

// GET-SET OWNER NAME
String SAVEEPROM::getOwnerName(){
	String ownerName = readEepromString(82,30);
	return ownerName;
}
void SAVEEPROM::setOwnerName(String ownerName){
	writeEepromString(82, ownerName, 30);
}

// GET-SET OWNER EMAIL
String SAVEEPROM::getOwnerEmail(){
	String ownerEmail = readEepromString(112,30);
	return ownerEmail;
}
void SAVEEPROM::setOwnerEmail(String ownerEmail){
	writeEepromString(112, ownerEmail, 30);
}

// GET-SET PASSWORD
String SAVEEPROM::getPassword(){
	String password = readEepromString(142,8);
	return password;
}
void SAVEEPROM::setPassword(String password){
	writeEepromString(142, password, 8);
}

// GET-SET EVENT
String SAVEEPROM::getEvent(int eventNumber){
	return readEepromString(150 + eventNumber*20, 20);
}
void SAVEEPROM::setEvent(int eventNumber, bool isEnabled, String eventName, unsigned long timestamp, unsigned long duration, bool isSwitched, int numberRepetitions, int repetitionType){
	EEPROM.write(150 + eventNumber*20, isEnabled);
	writeEepromString(151, eventName, 8);
	EEPROM.write(159 + eventNumber*20, lowByte(lowWord(timestamp)));
	EEPROM.write(160 + eventNumber*20, highByte(lowWord(timestamp)));
	EEPROM.write(161 + eventNumber*20, lowByte(highWord(timestamp)));
	EEPROM.write(162 + eventNumber*20, highByte(highWord(timestamp)));
	EEPROM.write(163 + eventNumber*20, lowByte(lowWord(duration)));
	EEPROM.write(164 + eventNumber*20, highByte(lowWord(duration)));
	EEPROM.write(165 + eventNumber*20, lowByte(highWord(duration)));
	EEPROM.write(166 + eventNumber*20, highByte(highWord(duration)));
	EEPROM.write(167 + eventNumber*20, isSwitched);
	EEPROM.write(168 + eventNumber*20, numberRepetitions);
	EEPROM.write(169 + eventNumber*20, repetitionType);
	
}

// GET-SET HOUR HISTORY
unsigned long SAVEEPROM::getHourHistory(int hour){
	return readEepromLong(950 + 4*hour);
}
void SAVEEPROM::setHourHistory(int hour, unsigned long consumption){
	writeEepromLong(950 + 4*hour, consumption);
}

// GET-SET DAY HISTORY
unsigned long SAVEEPROM::getDayHistory(int day){
	return readEepromLong(1046 + 4*day);
}
void SAVEEPROM::setDayHistory(int day, unsigned long consumption){
	writeEepromLong(1046 + 4*day, consumption);
}

// GET-SET WEEK HISTORY
unsigned long SAVEEPROM::getWeekHistory(int week){
	return readEepromLong(1170 + 4*week);
}
void SAVEEPROM::setWeekHistory(int week, unsigned long consumption){
	writeEepromLong(1170 + 4*week, consumption);
}

// GET-SET MONTH HISTORY
unsigned long SAVEEPROM::getMonthHistory(int month){
	return readEepromLong(1186 + 4*month);
}
void SAVEEPROM::setMonthHistory(int month, unsigned long consumption){
	writeEepromLong(1186 + 4*month, consumption);
}

// GET-SET CONSUMPTION DAY GOAL
unsigned long SAVEEPROM::getDayGoal(){
	return readEepromLong(1234);
}
void SAVEEPROM::setDayGoal(unsigned long dayGoal){
	writeEepromLong(1234, dayGoal);
}

// GET-SET CONSUMPTION WEEK GOAL
unsigned long SAVEEPROM::getWeekGoal(){
	return readEepromLong(1238);
}
void SAVEEPROM::setWeekGoal(unsigned long weekGoal){
	writeEepromLong(1238, weekGoal);
}

// GET-SET CONSUMPTION MONTH GOAL
unsigned long SAVEEPROM::getMonthGoal(){
	return readEepromLong(1242);
}
void SAVEEPROM::setMonthGoal(unsigned long monthGoal){
	writeEepromLong(1242, monthGoal);
}

// GET-SET ALARMS
String SAVEEPROM::getAlarm(int alarmNumber){
	return readEepromString(1246 + 12*alarmNumber, 12);
}
void SAVEEPROM::setAlarm(int alarmNumber, bool isEnabled, String alarmName, int alarmType, int alarmThreshold, int alarmVariable){
	EEPROM.write(1246 + 12*alarmNumber, isEnabled);
	writeEepromString(1247 + 12*alarmNumber, alarmName, 12);
	EEPROM.write(1248 + 12*alarmNumber, alarmType);
	EEPROM.write(1249 + 12*alarmNumber, alarmThreshold);
	EEPROM.write(1250 + 12*alarmNumber, alarmVariable);
	
}

// GET-SET TIME ON
unsigned long SAVEEPROM::getTimeOn(){
	return readEepromLong(1486);
}
void SAVEEPROM::setTimeOn(unsigned long timeOn){
	writeEepromLong(1486, timeOn);
}

// GET-SET TOTAL CONSUMPTION
unsigned long SAVEEPROM::getTotalConsumption(){
	return readEepromLong(1490);
}
void SAVEEPROM::setTotalConsumption(unsigned long totalConsumption){
	writeEepromLong(1490, totalConsumption);
}
