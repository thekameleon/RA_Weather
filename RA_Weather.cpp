// 
// 
// 

#include <ReefAngel_Features.h>
#include <Globals.h>
#include <RA_Wifi.h>
#include <Wire.h>
#include <OneWire.h>
#include <Time.h>
#include <DS1307RTC.h>
#include <InternalEEPROM.h>
#include <RA_NokiaLCD.h>
#include <RA_ATO.h>
#include <RA_Joystick.h>
#include <LED.h>
#include <RA_TempSensor.h>
#include <Relay.h>
#include <RA_PWM.h>
#include <Timer.h>
#include <Memory.h>
#include <InternalEEPROM.h>
#include <RA_Colors.h>
#include <RA_CustomColors.h>
#include <Salinity.h>
#include <RF.h>
#include <IO.h>
#include <ORP.h>
#include <AI.h>
#include <PH.h>
#include <WaterLevel.h>
#include <ReefAngel.h>
#include "RA_Weather.h"

void RA_Weather::Init()
{
	randomSeed(second() + (minute() * 60));
	CloudsEveryXDays = 2;
	CloudChancePerDay = 40;
    MinCloudDuration = 7;
    MaxCloudDuration = 15;
    MinCloudTransition = 30;
    MaxCloudTransition = 180;
    MinCloudsPerDay = 3;
    MaxCloudsPerDay = 5;
    StartCloudAfter = NumMins(00, 00);
    EndCloudBefore = NumMins(24, 00);
	LightningChancePerCloud = 15;
	LightningElapseMin = 5; //minimum seconds between lightning strikes
    LightningElapseMax = 120; //maximum seconds between strikes
    LightningStrikeDuration = 1500; //lightning strike duration... set to what works best for your drivers
	LightningFrequency = 10;

	weatherStatus = 255; //initialize for the first time
	cloudDuration = 0;
	cloudTransition = 0;
	cloudStart = 0;
	numClouds = 0;
	lightningChance = 0;
	cloudIndex = 0;
	lightningStatus = 0;
	lightningElapse = 0;
	forceStart = false;
	forceWithLightning = false;
}

void RA_Weather::ForceStart(byte duration, bool withLightning)
{
	forceStart = true;
	forceWithLightning = withLightning;
	forceDuration = duration;
}

void RA_Weather::RefreshPWMValues()
{
	ReefAngel.PWM.SetDaylight(WeatherPWM.DaylightPWMValue);
	ReefAngel.PWM.SetActinic(WeatherPWM.ActinicPWMValue);
#ifdef PWMEXPANSION
	ReefAngel.PWM.SetChannel(0, WeatherPWM.PWMChannelValue[0]);
	ReefAngel.PWM.SetChannel(1, WeatherPWM.PWMChannelValue[1]);
	ReefAngel.PWM.SetChannel(2, WeatherPWM.PWMChannelValue[2]);
	ReefAngel.PWM.SetChannel(3, WeatherPWM.PWMChannelValue[3]);
	ReefAngel.PWM.SetChannel(4, WeatherPWM.PWMChannelValue[4]);
	ReefAngel.PWM.SetChannel(5, WeatherPWM.PWMChannelValue[5]);
#endif
}

void RA_Weather::InitDaylight()
{
	InitDaylight(InternalMemory.PWMSlopeStartD_read(), InternalMemory.PWMSlopeEndD_read());
}

void RA_Weather::InitDaylight(byte min, byte max)
{
	MinPWM.DaylightPWMValue = min;
	MaxPWM.DaylightPWMValue = max;
}

void RA_Weather::InitActinic()
{
	InitActinic(InternalMemory.PWMSlopeStartA_read(), InternalMemory.PWMSlopeEndA_read());
}

void RA_Weather::InitActinic(byte min, byte max)
{
	MinPWM.ActinicPWMValue = min;
	MaxPWM.ActinicPWMValue = max;
}

#ifdef PWMEXPANSION
void RA_Weather::InitPWMChannel(byte channel)
{
	byte min = 0;
	byte max = 0;
	switch(channel)
	{
		case 1:
			min = InternalMemory.PWMSlopeStart1_read();
			max = InternalMemory.PWMSlopeEnd1_read();
			break;

		case 2:
			min = InternalMemory.PWMSlopeStart2_read();
			max = InternalMemory.PWMSlopeEnd2_read();
			break;

		case 3:
			min = InternalMemory.PWMSlopeStart3_read();
			max = InternalMemory.PWMSlopeEnd3_read();
			break;
		
		case 4:
			min = InternalMemory.PWMSlopeStart4_read();
			max = InternalMemory.PWMSlopeEnd4_read();
			break;
		
		case 5:
			min = InternalMemory.PWMSlopeStart5_read();
			max = InternalMemory.PWMSlopeEnd5_read();
			break;
		
		default:
			min = InternalMemory.PWMSlopeStart0_read();
			max = InternalMemory.PWMSlopeEnd0_read();
			break;
	}
	InitPWMChannel(channel, min, max);
}
#endif

#ifdef PWMEXPANSION
void RA_Weather::InitPWMChannel(byte channel, byte min, byte max)
{
	MinPWM.PWMChannelValue[channel] = min;
	MaxPWM.PWMChannelValue[channel] = max;
}
#endif

byte RA_Weather::GetCloudStartHour()
{
	return cloudStart / 60;
}

byte RA_Weather::GetCloudStartMinute()
{
	return cloudStart % 60;
}

byte RA_Weather::GetCloudDuration()
{
	return cloudDuration;
}

byte RA_Weather::GetStatus()
{
	return weatherStatus;
}

byte RA_Weather::GetCloudChance()
{
	return cloudChance;
}

byte RA_Weather::GetNumClouds()
{
	return numClouds;
}

byte RA_Weather::GetLightningChance()
{
	return weatherStatus;
}

void RA_Weather::Execute()
{
  // Every day at midnight, we check for chance of cloud happening today
	if (hour()==0 && minute()==0 && second()==0) weatherStatus = 255;

	if (weatherStatus == 255)
	{
		//Pick a random number between 0 and 99
		weatherStatus = random(100); 
		cloudChance = weatherStatus; //save original chance
		// if picked number is greater than Cloud_Chance_per_Day, we will not have clouds today
		if (weatherStatus > CloudChancePerDay) weatherStatus = 0;
		// Check if today is day for clouds. 
		if ((day() % CloudsEveryXDays)!=0) weatherStatus = 0; 
		// If we have cloud today
		if (weatherStatus || forceStart)
		{
			// pick a random number for number of clouds between Min_Clouds_per_Day and Max_Clouds_per_Day
			numClouds = random(MinCloudsPerDay, MaxCloudsPerDay);
			// pick the time that the first cloud will start
			// the range is calculated between Start_Cloud_After and the even distribuition of clouds on this day. 
			cloudStart = random(StartCloudAfter, StartCloudAfter+((EndCloudBefore - StartCloudAfter) / (numClouds * 2)));
			// pick a random number for the cloud duration of first cloud.
			cloudDuration = random(MinCloudDuration, MaxCloudDuration);

			cloudTransition = random(MinCloudTransition, MaxCloudTransition);
			
			if(cloudTransition * 2 + 2 > cloudDuration * 60) cloudDuration = (cloudTransition / 30) + 2;

			//Pick a random number between 0 and 99
			lightningChance = random(100);
			// if picked number is greater than Lightning_Change_per_Cloud, we will not have lightning today
			if (lightningChance > LightningChancePerCloud) lightningChance=0;

			lightningElapse = now(); //start with current time
			
			if(forceStart)
			{
				weatherStatus = 100;
				cloudDuration = forceDuration;
				cloudStart = NumMins(hour(), minute());
				if(forceWithLightning) lightningChance = 100;
			}
		}
		else
		{
			cloudStart = 0;
			cloudDuration = 0;
		}
    }
  // Now that we have all the parameters for the cloud, let's create the effect
  WeatherPWM.DaylightPWMValue = ReefAngel.PWM.GetDaylightValue();
  WeatherPWM.ActinicPWMValue = ReefAngel.PWM.GetActinicValue();
#ifdef PWMEXPANSION
  WeatherPWM.PWMChannelValue[0] = ReefAngel.PWM.GetChannelValue(0);
  WeatherPWM.PWMChannelValue[1] = ReefAngel.PWM.GetChannelValue(1);
  WeatherPWM.PWMChannelValue[2] = ReefAngel.PWM.GetChannelValue(2);
  WeatherPWM.PWMChannelValue[3] = ReefAngel.PWM.GetChannelValue(3);
  WeatherPWM.PWMChannelValue[4] = ReefAngel.PWM.GetChannelValue(4);
  WeatherPWM.PWMChannelValue[5] = ReefAngel.PWM.GetChannelValue(5);
#endif

  

  if (weatherStatus)
  {
    //is it time for cloud yet?
    if (NumMins(hour(), minute()) >= cloudStart && NumMins(hour(), minute()) < (cloudStart + cloudDuration))
    {
		if (CloudChannels & 1 == 1 && WeatherPWM.DaylightPWMValue >= MinPWM.DaylightPWMValue) WeatherPWM.DaylightPWMValue = 
			ReversePWMSlope(cloudStart, cloudStart + cloudDuration, WeatherPWM.DaylightPWMValue, MinPWM.DaylightPWMValue, cloudTransition);

		if (CloudChannels & 2 == 2 && WeatherPWM.ActinicPWMValue >= MinPWM.ActinicPWMValue) WeatherPWM.ActinicPWMValue = 
			ReversePWMSlope(cloudStart, cloudStart + cloudDuration, WeatherPWM.ActinicPWMValue, MinPWM.ActinicPWMValue, cloudTransition);

#ifdef PWMEXPANSION
		if (CloudChannels & 4 == 4 && WeatherPWM.PWMChannelValue[0] >= MinPWM.PWMChannelValue[0]) WeatherPWM.PWMChannelValue[0] = 
			ReversePWMSlope(cloudStart, cloudStart + cloudDuration, WeatherPWM.PWMChannelValue[0], MinPWM.PWMChannelValue[0], cloudTransition);

		if (CloudChannels & 8 == 8 && WeatherPWM.PWMChannelValue[1] >= MinPWM.PWMChannelValue[1]) WeatherPWM.PWMChannelValue[1] = 
			ReversePWMSlope(cloudStart, cloudStart+cloudDuration, WeatherPWM.PWMChannelValue[1], MinPWM.PWMChannelValue[1], cloudTransition);

		if (CloudChannels & 16 == 16 && WeatherPWM.PWMChannelValue[2] >= MinPWM.PWMChannelValue[2]) WeatherPWM.PWMChannelValue[2] = 
			ReversePWMSlope(cloudStart, cloudStart+cloudDuration, WeatherPWM.PWMChannelValue[2], MinPWM.PWMChannelValue[2], cloudTransition);

		if (CloudChannels & 32 == 32 && WeatherPWM.PWMChannelValue[3] >= MinPWM.PWMChannelValue[3]) WeatherPWM.PWMChannelValue[3] = 
			ReversePWMSlope(cloudStart, cloudStart+cloudDuration, WeatherPWM.PWMChannelValue[3], MinPWM.PWMChannelValue[3], cloudTransition);

		if (CloudChannels & 64 == 64 && WeatherPWM.PWMChannelValue[4] >= MinPWM.PWMChannelValue[4]) WeatherPWM.PWMChannelValue[4] = 
			ReversePWMSlope(cloudStart, cloudStart+cloudDuration, WeatherPWM.PWMChannelValue[4], MinPWM.PWMChannelValue[4], cloudTransition);

		if (CloudChannels & 128 == 128 && WeatherPWM.PWMChannelValue[5] >= MinPWM.PWMChannelValue[5]) WeatherPWM.PWMChannelValue[5] = 
			ReversePWMSlope(cloudStart, cloudStart+cloudDuration, WeatherPWM.PWMChannelValue[5], MinPWM.PWMChannelValue[5], cloudTransition);
#endif

        if (lightningChance && 
		    (NumMins(hour(), minute()) > (cloudStart + cloudTransition) &&
		    NumMins(hour(), minute()) < (cloudStart + cloudDuration - cloudTransition)))
        {
			//within the transition periods
            if (lightningStatus == 0 && now() > lightningElapse && random(100) < LightningFrequency) 
		    {
			    lightningStatus = 1; 
	    		lightningElapse = millis() + LightningStrikeDuration;
		    }

			if (lightningStatus == 1)
			{
				weatherStatus = 102;

				if(lightningElapse > millis())
				{
					if (LightningChannels & 1 == 1) WeatherPWM.DaylightPWMValue = MaxPWM.DaylightPWMValue;
					if (LightningChannels & 2 == 2) WeatherPWM.ActinicPWMValue = MaxPWM.ActinicPWMValue;
#ifdef PWMEXPANSION
					if (LightningChannels & 4 == 4) WeatherPWM.PWMChannelValue[0] = MaxPWM.PWMChannelValue[0];
					if (LightningChannels & 8 == 8) WeatherPWM.PWMChannelValue[1] = MaxPWM.PWMChannelValue[1];
					if (LightningChannels & 16 == 16) WeatherPWM.PWMChannelValue[2] = MaxPWM.PWMChannelValue[2];
					if (LightningChannels & 32 == 32) WeatherPWM.PWMChannelValue[3] = MaxPWM.PWMChannelValue[3];
					if (LightningChannels & 64 == 64) WeatherPWM.PWMChannelValue[4] = MaxPWM.PWMChannelValue[4];
					if (LightningChannels & 128 == 128) WeatherPWM.PWMChannelValue[5] = MaxPWM.PWMChannelValue[5];
#endif
				}
				else
				{
					lightningElapse = random(LightningElapseMin, LightningElapseMax) + now(); //elapse for next strike
					lightningStatus = 0;
				}
			}
		}
    }

    if (NumMins(hour(), minute()) > (cloudStart + cloudDuration))
    {
		lightningElapse = now(); //set to now
		lightningStatus = 0;
		cloudIndex++;

		if (cloudIndex < numClouds)
		{
			cloudStart = random(StartCloudAfter + (((EndCloudBefore - StartCloudAfter) / (numClouds * 2)) * cloudIndex * 2), 
				(StartCloudAfter + (((EndCloudBefore - StartCloudAfter) / (numClouds * 2)) * cloudIndex * 2)) + 
				((EndCloudBefore - StartCloudAfter) / (numClouds * 2)));
			// pick a random number for the cloud duration of first cloud.
			cloudDuration = random(MinCloudDuration, MaxCloudDuration);

			cloudTransition = random(MinCloudTransition, MaxCloudTransition);

			//Pick a random number between 0 and 99
			lightningChance = random(100);
			// if picked number is greater than Lightning_Change_per_Cloud, we will not have lightning today
			if (lightningChance > LightningChancePerCloud) lightningChance = 0;
		}
    }
  }
}

byte RA_Weather::ReversePWMSlope(long cstart,long cend,byte PWMStart,byte PWMEnd, byte clength)
{
  long n=elapsedSecsToday(now());
  cstart*=60;
  cend*=60;
  if (n<cstart) return PWMStart;
  if (n>=cstart && n<=(cstart+clength)) return map(n,cstart,cstart+clength,PWMStart,PWMEnd);
  if (n>(cstart+clength) && n<(cend-clength)) return PWMEnd;
  if (n>=(cend-clength) && n<=cend) return map(n,cend-clength,cend,PWMEnd,PWMStart);
  if (n>cend) return PWMStart;
}

RA_Weather Weather;

