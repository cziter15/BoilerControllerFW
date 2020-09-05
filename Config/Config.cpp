	#include "../board.h"
#include "FS.h"
#include "Config.h"
#include <WiFiManager.h>

const char ConfigData::settings_fn[] = "/settings.dat";
bool Config::wantForceConfig = false;

void Config::setupConfigPins()
{
	pinMode(CFG_PUSH_PIN, INPUT);
}

void Config::checkConfigButton()
{
	if ((digitalRead(CFG_PUSH_PIN) == LOW) || wantForceConfig)
		Config::startConfigPortal();
}

void ConfigData::loadSaveConfig(bool load)
{
	if (File f = SPIFFS.open(settings_fn, load ? "r" : "w+"))
	{
		if (load)
		{
			broker = f.readStringUntil('\n');
			port = f.readStringUntil('\n');
			user = f.readStringUntil('\n');
			password = f.readStringUntil('\n');
			prefix = f.readStringUntil('\n');
		}
		else
		{
			f.print(broker + '\n');
			f.print(port + '\n');
			f.print(user + '\n');
			f.print(password + '\n');
			f.print(prefix + '\n');
		}

		f.close();
	}
}

void Config::startConfigPortal()
{
	wantForceConfig = false;

	ConfigData configData;
	configData.loadSaveConfig(true);

	WiFiManager wifiManager;

	WiFiManagerParameter broker("broker", "MQTT Broker", configData.broker.c_str(), 50);
	wifiManager.addParameter(&broker);

	WiFiManagerParameter port("port", "MQTT port", configData.port.c_str(), 5);
	wifiManager.addParameter(&port);

	WiFiManagerParameter user("user", "MQTT user", configData.user.c_str(), 50);
	wifiManager.addParameter(&user);

	WiFiManagerParameter password("password", "MQTT password", configData.password.c_str(), 50);
	wifiManager.addParameter(&password);

	WiFiManagerParameter prefix("prefix", "MQTT topic prefix", configData.prefix.c_str(), 50);
	wifiManager.addParameter(&prefix);

	wifiManager.setConfigPortalTimeout(120);
	wifiManager.setConnectTimeout(10);

	wifiManager.startConfigPortal(getDeviceName().c_str());

	configData.broker = String(broker.getValue());
	configData.port = String(port.getValue());
	configData.user = String(user.getValue());
	configData.password = String(password.getValue());
	configData.prefix = String(prefix.getValue());

	configData.loadSaveConfig(false);

	delay(500);

	ESP.reset();
}

String Config::getDeviceName()
{
	return String(DEVICE_NAME_STRING) + String(ESP.getChipId());
}

void Config::forceConfig()
{
	wantForceConfig = true;
}
