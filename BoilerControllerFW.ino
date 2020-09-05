#include "App/BoilerControllerApp.h"
#include "board.h"
#include "ArduinoOTA.h"
#include "FS.h"

BoilerControllerApp boilerControllerApp;

void setup()
{
	SPIFFS.begin();

	uint32_t chipId = ESP.getChipId();

	uint8_t mac[6]{ 0xfa, 0xf1, chipId >> 8, chipId >> 16, chipId >> 24, chipId >> 32 };
	wifi_set_macaddr(STATION_IF, mac);

	ArduinoOTA.begin();
	ArduinoOTA.setPassword("pcktrl_update_pwd");

	boilerControllerApp.begin();
}

void loop()
{
	boilerControllerApp.loop();
	ArduinoOTA.handle();
	delay(1);
}