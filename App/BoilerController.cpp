#include "../board.h"

#include <DallasTemperature.h>
#include <Timezone.h>
#include <Wire.h>
#include <WiFiUdp.h>

#include "../Libs/LCD/LiquidCrystal_I2C.h"
#include "../Libs/MQTTHandler/MQTTHandler.h"

#include "BoilerController.h"
#include "BoilerControllerApp.h"

TimeChangeRule BoilerController::tz_CEST = { "CEST", Last, Sun, Mar, 2, 120 };     // Central European Summer Time
TimeChangeRule BoilerController::tz_CET = { "CET ", Last, Sun, Oct, 3, 60 };       // Central European Standard Time
Timezone BoilerController::tz_CE = Timezone(tz_CEST, tz_CET);

using namespace std::placeholders;

BoilerController::BoilerController(BoilerControllerApp* _App) : App(_App)
{
	LCD_Interface = new LiquidCrystal_I2C(0x3F, 8, 2);

	oneWireInterface = new OneWire(TEMP_PIN);
	tempSensor = new DallasTemperature(oneWireInterface);
	tempSensor->begin();
	tempSensor->setWaitForConversion(false);

	LCD_Interface->begin();
	LCD_Interface->backlight();

	udp = new WiFiUDP();
	udp->setTimeout(1000);

	setState(APP_STATE::INIT);
}

void BoilerController::setState(APP_STATE::TYPE state)
{
	if (currentState != state)
	{
		APP_STATE::TYPE oldState = currentState;
		currentState = state;
		lastStateSwitch = millis();
		onEnterState(state, oldState);
		onUpdateStateEverySecond(state);
	}
}

void BoilerController::onUpdateStateEverySecond(APP_STATE::TYPE state)
{
	switch (state)
	{
		case APP_STATE::DISPLAY_TIME:
		{
			time_t rawtime;
			time(&rawtime);

			if (rawtime > 0)
			{
				TimeChangeRule *tcr;
				time_t t = tz_CE.toLocal(rawtime, &tcr);

				LCD_Interface->setCursor(0, 0);
				LCD_Interface->print("Czas:   ");
				LCD_Interface->setCursor(0, 1);
				LCD_Interface->printf("%02d:%02d:%02d", hour(t), minute(t), second(t));
			}
			else
			{
				LCD_Interface->setLCDText("Brak akt. czasu");
			}
		}
		break;

		case APP_STATE::DISPLAY_TEMP:
		{
			if (boilerState.isActive != LCD_Interface->getBacklight())
				LCD_Interface->setBacklight(boilerState.isActive);

			float temp = boilerState.waterTemp;
			LCD_Interface->setLCDText("T. wody: " + ((temp > 0) ? String(temp, 1) + "\xDF""C" : "?"));
		}
		break;
	}
}

void BoilerController::onEnterState(APP_STATE::TYPE state, APP_STATE::TYPE prevState)
{
	switch (state)
	{
		case APP_STATE::INIT:
		{
			LCD_Interface->setLCDText("Inicjalizacja...");
			tempSensor->requestTemperatures();
		}
		break;

		case APP_STATE::UPDATE:
		{
			LCD_Interface->setLCDText("Aktualizacja...");
		}
		break;
	}
}

void BoilerController::handle()
{
	updateTemperature();

	unsigned long cur_millis = millis();
	unsigned long currentStateTime = cur_millis - lastStateSwitch;

	if (cur_millis - lastEverySecondUpdate > ONE_SECOND_MS)
	{
		lastEverySecondUpdate = cur_millis;
		onUpdateStateEverySecond(currentState);
		boilerState.uptimeInSeconds++;
	}

	if (currentStateTime > STATE_ROLL_TIME)
	{
		int targetState = currentState + 1;
		setState(APP_STATE::TYPE(targetState >= APP_STATE::MAX ? APP_STATE::DISPLAY_TEMP : targetState));
		broadcastState();
	}
}

void BoilerController::updateTemperature()
{
	if (millis() - lastTemperatureUpdate > TEMP_REFRESH_TIME)
	{
		lastTemperatureUpdate = millis();

		tempSensor->requestTemperatures();
		float currentTemp = tempSensor->getTempCByIndex(0);

		String roundedTemp = String(currentTemp, 1);

		if (fabs(lastUpdatedTemp - currentTemp) > 0.1f)
		{
			lastUpdatedTemp = currentTemp;
			App->mMessageHandler->updateValue("temperature", String(roundedTemp));
		}

		if (++temperatureUpdateTicks == 20)
		{
			temperatureUpdateTicks = 0;
			App->mMessageHandler->updateValue("temperature5min", String(roundedTemp));
		}

		boilerState.updateTemp(currentTemp);
	}
}

void BoilerController::broadcastState()
{
	IPAddress localIp = WiFi.localIP();;
	IPAddress multiIp = localIp;
	multiIp[3] = 255;

	udp->beginPacketMulticast(multiIp, UDP_PORT_BROADCAST, localIp);
	udp->write((const char*)&boilerState, sizeof(boilerState));
	udp->endPacket();
}

BoilerController::~BoilerController()
{
	delete LCD_Interface;
	delete tempSensor;
	delete udp;
	delete oneWireInterface;
}
