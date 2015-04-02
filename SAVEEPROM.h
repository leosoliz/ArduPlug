#ifndef SAVEEPROM_H
#define SAVEEPROM_H

#include <Arduino.h>

class SAVEEPROM{
	public:
	SAVEEPROM();
	
	// GET-SET ReadPeriod
	int getReadPeriod();
	void setReadPeriod(int readPeriod);
	
	//GET-SET SAVEID
	String getSaveId();
	void setSaveId(String saveId);
	
	// GET-SET SAVEFIRM
	String getSaveFirm();
	void setSaveFirm(String saveFirm);
	
	// GET-SET DHCP
	bool isDHCPEnabled();
	void setDHCPEnabled(bool enabled);
	
	// GET-SET IP
	String getDeviceIp();
	void setDeviceIp(String ip);
	
	// GET-SET GATEWAY IP
	String getGatewayIp();
	void setGatewayIp(String ip);
	
	// GET-SET SUBNET
	String getSubNet();
	void setSubNet(String mask);
	
	// GET-SET HTTP PORT
	int getHttpPort();
	void setHttpPort( int httpPort);
	
	// GET-SET NTP SERVER
	String getNtpServer();
	void setNtpServer(String ip);
	
	// GET-SET UDP PORT
	int getUdpPort();
	void setUdpPort( int udpPort);

	// GET-SET DEVICE NAME
	String getDeviceName();
	void setDeviceName(String deviceName);

	// GET-SET DEVICE LOCATION
	String getDeviceLocation();
	void setDeviceLocation(String location);

	// GET-SET OWNER NAME
	String getOwnerName();
	void setOwnerName(String ownerName);

	// GET-SET OWNER EMAIL
	String getOwnerEmail();
	void setOwnerEmail(String ownerEmail);

	// GET-SET PASSWORD
	String getPassword();
	void setPassword(String password);

	// GET-SET EVENT
	String getEvent(int eventNumber);
	void setEvent(int eventNumber, bool isEnabled, String eventName, unsigned long timestamp, unsigned long duration, bool isSwitched, int numberRepetitions, int repetitionType);

	// GET-SET HOUR HISTORY
	unsigned long getHourHistory(int hour);
	void setHourHistory(int hour, unsigned long consumption);

	// GET-SET DAY HISTORY
	unsigned long getDayHistory(int day);
	void setDayHistory(int day, unsigned long consumption);

	// GET-SET WEEK HISTORY
	unsigned long getWeekHistory(int week);
	void setWeekHistory(int week, unsigned long consumption);

	// GET-SET MONTH HISTORY
	unsigned long getMonthHistory(int month);
	void setMonthHistory(int month, unsigned long consumption);

	// GET-SET CONSUMPTION DAY GOAL
	unsigned long getDayGoal();
	void setDayGoal(unsigned long dayGoal);

	// GET-SET CONSUMPTION WEEK GOAL
	unsigned long getWeekGoal();
	void setWeekGoal(unsigned long weekGoal);

	// GET-SET CONSUMPTION MONTH GOAL
	unsigned long getMonthGoal();
	void setMonthGoal(unsigned long monthGoal);

	// GET-SET ALARMS
	String getAlarm(int alarmNumber);
	void setAlarm(int alarmNumber, bool isEnabled, String name, int alarmType, int threshold, int priority);

	// GET-SET TIME ON
	unsigned long getTimeOn();
	void setTimeOn(unsigned long timeOn);

	// GET-SET TOTAL CONSUMPTION
	unsigned long getTotalConsumption();
	void setTotalConsumption(unsigned long totalConsumption);

	private:
	String readEepromString(int initAddress, int stringLength);
	void writeEepromString(int initAddress, String value, int addressLength);
	unsigned long readEepromLong(int initAddress);
	void writeEepromLong(int initAddress, unsigned long value);
	unsigned int readEepromDouble(int initAddress);
	void writeEepromDouble(int initAddress, unsigned int value);
	String readEepromIp(int initAddress);
	void writeEepromIp(int initAddress, String ip);
	
};

#endif
