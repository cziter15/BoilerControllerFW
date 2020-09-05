#include "../../board.h"
#include "../../Config/Config.h"
#include <PubSubClient.h>
#include "MqttHandler.h"

#define ONE_SECOND_MS 1000UL

using namespace std::placeholders;

MqttHandler::MqttHandler(String deviceName, String& broker, String& port, String& user, String& password, String& prefix)
{
	userDeviceName = deviceName;

	MqttWifiClient = new WiFiClient();
	Mqtt = new PubSubClient(*MqttWifiClient);

	cachedUser = user;
	cachedPassword = password;
	cachedPrefix = prefix;
	cachedBroker = broker;

	Mqtt->setCallback(std::bind(&MqttHandler::HandleMqttMessage, this, _1, _2, _3));

	IPAddress serverIP;

	if (serverIP.fromString(cachedBroker.c_str()))
		Mqtt->setServer(serverIP, port.toInt());
	else
		Mqtt->setServer(cachedBroker.c_str(), port.toInt());

	lastTryReconnectTime = millis();
}

String MqttHandler::BytesToString(byte * payload, unsigned int length)
{
	String value;

	for (int i = 0; i < length; i++)
		value += String((char)payload[i]);

	return value;
}

void MqttHandler::subscribe(String topic)
{
	Mqtt->subscribe(String(cachedPrefix + userDeviceName + topic).c_str());
}

void MqttHandler::unsubscribe(String topic)
{
	Mqtt->unsubscribe(String(cachedPrefix + userDeviceName + topic).c_str());
}

void MqttHandler::HandleMqttMessage(char * topic, byte * payload, unsigned int length)
{
	String topicString = String(topic);

	if (topicString.startsWith("!resub"))
	{
		subscribe("/c/#");

		for (auto& entry : messageCallbacks)
			entry.function("!resub", "");
	}
	else if (topicString.length() > cachedPrefix.length())
	{
		if (topicString.startsWith(cachedPrefix + userDeviceName))
		{
			topicString = topicString.substring(cachedPrefix.length() + userDeviceName.length());

			if (topicString.startsWith("/c/reset"))
			{
				ESP.restart();
			}
			else if (topicString.startsWith("/c/dbg"))
			{
				sendLog(
					"Conn. time: " + String(connectionTimeSeconds) + " sec, " +
					"Conn. count: " + String(reconnectCounter) + ", " +
					"RSSI " + String(WiFi.RSSI()) + " dBm"
				);
			}
			else if (topicString.startsWith("/c/cfg"))
			{
				WiFi.disconnect();
				Config::forceConfig();
			}
			else
			{
				if (topicString.equals(topicString))
				{
					for (auto& entry : messageCallbacks)
					{
						entry.function(topicString, BytesToString(payload, length));
					}
				}
			}
		}
	}
}

void MqttHandler::handle()
{
	if (!Mqtt->loop())
	{
		if (millis() - lastTryReconnectTime > MQTT_RECONNECTION_TIME * ONE_SECOND_MS)
		{
			lastTryReconnectTime = millis();

			if (WiFi.isConnected())
			{
				if (Mqtt->connect(String(ESP.getChipId()).c_str(), cachedUser.c_str(), cachedPassword.c_str()))
				{
					connectionTimeSeconds = 0;
					reconnectCounter++;

					MqttWifiClient->setSync(true);
					MqttWifiClient->setNoDelay(true);
					MqttWifiClient->setTimeout(MQTT_SOCKET_TIMEOUT * ONE_SECOND_MS);

					HandleMqttMessage("!resub", (byte*)"", 1);

					sendLog(WiFi.localIP().toString());
				}
			}
		}
	}
	else 
	{
		if (millis() - lastConnectionTimeTick > ONE_SECOND_MS)
		{
			lastConnectionTimeTick = millis();
			connectionTimeSeconds++;
		}
	}
}

void MqttHandler::sendLog(String log)
{
	String finalStr =  "[" + String(millis() / ONE_SECOND_MS) + "] " + log;
	Mqtt->publish(String(cachedPrefix + userDeviceName + "/log").c_str(), (uint8_t*)finalStr.c_str(), finalStr.length(), false);
}

void MqttHandler::updateValue(String valueName, String value)
{
	String topicName = String(cachedPrefix + userDeviceName + "/" + valueName);
	Mqtt->publish(topicName.c_str(), (uint8_t*)value.c_str(), value.length(), true);
}

unsigned int MqttHandler::addCallback(MqttMessageCallback callback)
{
	MqttMessageCallbackInfo info;
	info.function = callback;
	info.callbackId = callbackCounter++;
	messageCallbacks.push_back(info);
	return info.callbackId;
}

void MqttHandler::removeCallback(unsigned int callbackId)
{
	for (std::vector<MqttMessageCallbackInfo>::iterator it = messageCallbacks.end() - 1; it != messageCallbacks.begin() - 1; it--)
		if (it->callbackId == callbackId)
			it = messageCallbacks.erase(it);
}

MqttHandler::~MqttHandler()
{
	delete Mqtt;
	delete MqttWifiClient;
	delete RfTransmitter;
}