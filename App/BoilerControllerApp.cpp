#include "../board.h"
#include "../Config/Config.h"
#include "../Libs/MQTTHandler/MQTTHandler.h"
#include "BoilerController.h"
#include "BoilerControllerApp.h"

using namespace std::placeholders;

BoilerControllerApp::BoilerControllerApp()
{
	Config::setupConfigPins();

	WiFi.hostname(Config::getDeviceName());
	WiFi.setAutoConnect(true);
	WiFi.setAutoReconnect(true);

	configTime(0, 0, "pool.ntp.org", "time.nist.gov");
}

void BoilerControllerApp::begin()
{
	if (WiFi.SSID().length())
	{
		ConfigData config = ConfigData();
		config.loadSaveConfig(true);

		mMessageHandler = new MqttHandler(DEVICE_NAME_STRING, config.broker, config.port, config.user, config.password, config.prefix);
		mBoilerController = new BoilerController(this);

		WiFi.mode(WIFI_STA);
		WiFi.setSleepMode(WiFiSleepType::WIFI_MODEM_SLEEP, 3);
		WiFi.setOutputPower(15.0f);
		WiFi.begin();
	}
	else
	{
		Config::startConfigPortal();
	}
}

void BoilerControllerApp::loop()
{
	if (mMessageHandler && mBoilerController)
	{
		mMessageHandler->handle();
		mBoilerController->handle();
	}

	Config::checkConfigButton();
}

BoilerControllerApp::~BoilerControllerApp()
{
	delete mBoilerController;
	delete mMessageHandler;
}