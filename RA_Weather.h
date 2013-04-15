#ifndef __WEATHER__
#define __WEATHER__

typedef struct
{
  byte ActinicPWMValue;
  byte DaylightPWMValue;
#ifdef PWMEXPANSION
  byte PWMChannelValue[6];
#endif
} WeatherPWM_t;

class RA_Weather
{
	public:
		WeatherPWM_t MaxPWM;
		WeatherPWM_t MinPWM;
		WeatherPWM_t WeatherPWM;

		byte CloudsEveryXDays;
		byte CloudChancePerDay;
		byte MinCloudDuration;
		byte MaxCloudDuration;
		byte MinCloudTransition;
		byte MaxCloudTransition;
		byte MinCloudsPerDay;
		byte MaxCloudsPerDay;
		int StartCloudAfter;
		int EndCloudBefore;
		byte LightningChancePerCloud;
		byte LightningElapseMin; //minimum seconds between lightning strikes
		byte LightningElapseMax; //maximum seconds between strikes
		int LightningStrikeDuration; //lightning strike duration... set to what works best for your drivers
		byte LightningFrequency;

		byte CloudChannels;
		byte LightningChannels;

		void Init();
		void Execute();
		void InitDaylight();
		void InitDaylight(byte min, byte max);
		void InitActinic();
		void InitActinic(byte min, byte max);
#ifdef PWMEXPANSION
		void InitPWMChannel(byte channel);
		void InitPWMChannel(byte channel, byte min, byte max);
#endif
		void RefreshPWMValues();
		void ForceStart(byte duration, bool withLightning);
		byte GetStatus();
		byte GetCloudStartHour();
		byte GetCloudStartMinute();
		byte GetCloudDuration();
		byte GetLightningChance();
		byte GetCloudChance();
		byte GetNumClouds();

	private:
		byte cloudChance;
		byte cloudDuration;
		byte cloudTransition;
		int cloudStart;
		byte numClouds;
		byte lightningChance;
		byte cloudIndex;
		byte lightningStatus;
		bool forceStart;
		bool forceWithLightning;
		byte forceDuration;
		unsigned long lightningElapse;
		byte weatherStatus;

		byte ReversePWMSlope(long cstart,long cend,byte PWMStart,byte PWMEnd, byte clength);
};

extern RA_Weather Weather;

#endif
