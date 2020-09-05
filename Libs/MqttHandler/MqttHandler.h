#pragma once

#include <WString.h>
#include <Arduino.h>

typedef std::function<void(String, String)> MqttMessageCallback;

typedef struct _MqttMessageCallbackInfo
{
	unsigned int callbackId;
	MqttMessageCallback function;
}
MqttMessageCallbackInfo;

class MqttHandler
{
protected:
	unsigned int callbackCounter = 0;
	unsigned int connectionTimeSeconds = 0;
	unsigned int lastConnectionTimeTick = 0;

	class PubSubClient* Mqtt = nullptr;
	class WiFiClient *MqttWifiClient = nullptr;
	class NexaTransmitter* RfTransmitter = nullptr;

	unsigned long lastTryReconnectTime = 0;
	unsigned int reconnectCounter = 0;

	String cachedUser;
	String cachedPassword;
	String cachedPrefix;
	String cachedBroker;

	String userDeviceName;

	void HandleMqttMessage(char * topic, byte * payload, unsigned int length);

	std::vector<MqttMessageCallbackInfo> messageCallbacks;

public:
	MqttHandler(String deviceName, String& broker, String& port, String& user, String& password, String& prefix);
	String BytesToString(byte * payload, unsigned int length);
	void subscribe(String topic);
	void unsubscribe(String topic);
	void handle();
	void sendLog(String log);
	void updateValue(String valueName, String value);
	virtual ~MqttHandler();

	unsigned int addCallback(MqttMessageCallback callback);
	void removeCallback(unsigned int callbackId);
};

