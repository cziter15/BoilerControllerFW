#pragma once
#include <WString.h>
#include <ESP8266WiFi.h>

class BoilerControllerApp
{
	friend class BoilerController;

	protected:
		class MqttHandler * mMessageHandler = nullptr;
		class BoilerController* mBoilerController = nullptr;

	public:
		BoilerControllerApp();

		void begin();
		void loop();

		virtual ~BoilerControllerApp();
};

