#pragma once
#include <WString.h>

struct ConfigData
{
	protected:
		static const char settings_fn[];

	public:
		String broker		= "";
		String port			= "1883";
		String user			= "";
		String password		= "";
		String prefix		= "";

		void loadSaveConfig(bool load);
};

class Config
{
	protected:
		static bool wantForceConfig;

	public:
		
		static void setupConfigPins();

		static void checkConfigButton();
		static void startConfigPortal();

		static String getDeviceName();
		static void forceConfig();
};