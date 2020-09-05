#pragma once

namespace APP_STATE
{
	enum TYPE
	{
		NONE,
		UPDATE,
		INIT,
		DISPLAY_TEMP,
		DISPLAY_TIME,
		MAX
	};
};

struct BC_BoilerState
{
	uint32_t sig = 'BC01';
	float waterTemp = 0;
	bool isActive = false;
	uint32_t uptimeInSeconds = 0;

	void updateTemp(float temp) 
	{
		waterTemp = temp;
		isActive = temp > 25;
	}
};


class BoilerController
{
protected:
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	class BoilerControllerApp* App = nullptr;
	class LiquidCrystal_I2C * LCD_Interface = nullptr;

	class OneWire* oneWireInterface = nullptr;
	class DallasTemperature * tempSensor = nullptr;

	class WiFiUDP * udp = nullptr;

	BC_BoilerState boilerState;
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	static class TimeChangeRule tz_CEST;
	static class TimeChangeRule tz_CET;
	static class Timezone tz_CE;
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	unsigned long lastTemperatureUpdate = 0;
	unsigned char temperatureUpdateTicks = 0;
	float lastUpdatedTemp = 0.0f;
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	APP_STATE::TYPE currentState = APP_STATE::TYPE(0);
	unsigned long lastStateSwitch = 0;
	unsigned long lastEverySecondUpdate = 0;
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	void updateTemperature();

	void setState(APP_STATE::TYPE state);
	void onEnterState(APP_STATE::TYPE state, APP_STATE::TYPE prevState);
	void onUpdateStateEverySecond(APP_STATE::TYPE state);
	void broadcastState();

public:
	BoilerController(class BoilerControllerApp* _App);
	void handle();

	virtual ~BoilerController();
};



