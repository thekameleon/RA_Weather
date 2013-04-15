Reference:
 byte CloudsEveryXDays; //How often clouds can happen
 byte CloudChancePerDay; //Chances that a cloud will happen per day. Specifying 1 is every day, 2 every other day, 3 every 3rd day, etc...
 byte MinCloudDuration; //Minimium duration a cloud will happen in minutes
 byte MaxCloudDuration; //Maximum duration a cloud will happen in minutes
 byte MinCloudTransition; //Minimium transition time to and from a cloud in seconds
 byte MaxCloudTransition; //Maximum transition time to and from a cloud in seconds
 byte MinCloudsPerDay; //Minimium number of clouds in a day
 byte MaxCloudsPerDay; //Mamimum number of clouds in a day
 int StartCloudAfter; //Specifies a time of day that clouds can start occuring in minutes since midnight
 int EndCloudBefore;//Specifies a time of day that clouds will end occuring in minutes since midnight
 byte LightningChancePerCloud; //odds that lightning could happen per cloud
 byte LightningElapseMin; //minimum seconds between lightning strikes
 byte LightningElapseMax; //maximum seconds between strikes
 int LightningStrikeDuration; //lightning strike duration... set to what works best for your drivers
 byte LightningFrequency; //how frequent lightning will occur within the cloud 0% to 100%

 byte CloudChannels; //specifies which channels in bits (e.g. 11000000 would be daylight and actinic only, 00110000 would be PWM Expansion 0 and 1)
 byte LightningChannels; //specifies which channels in bits (e.g. 11000000 would be daylight and actinic only, 00110000 would be PWM Expansion 0 and 1)

 void Init(); //Sets initial values. Must be called
 void Execute(); //Executes weather logic.  Typically called on each iteration of loop function
 void InitDaylight(); //initializes min and max values for Daylight PWM from memory
 void InitDaylight(byte min, byte max); //specify min and max values for Daylight
 void InitActinic(); //initializes min and max values for Actinic PWM from memory
 void InitActinic(byte min, byte max); //specify min and max values for Actinic
 void InitPWMChannel(byte channel); //initializes min and max values for PWM Expansion channel from memory
 void InitPWMChannel(byte channel, byte min, byte max); //specify min and max values for PWM Expansion channel
 void RefreshPWMValues(); //Refreshes PWM values with values from weather... Must be called in loop() before ShowInterface call
 void ForceStart(byte duration, bool withLightning); //forces start of weather
 byte GetStatus(); //get weather status.  Either shows 0 for no clouds, 1 - 100 to cloud chance, 101 cloud start, 102 cloudy, 103 cloud ending, 104 lightning occurring
 byte GetCloudStartHour(); //Gets hour that cloud will start
 byte GetCloudStartMinute(); //Gets minute that cloud will start
 byte GetCloudDuration(); //Get duration in minutes that cloud will last
 byte GetLightningChance(); //percentage that lightning could occur
 byte GetCloudChance(); //percentage that clouds could occur
 byte GetNumClouds(); //number of clouds today.

Usage:

setup()
{
  Weather.Init();
  Weather.CloudsEveryXDays = 1; //How often clouds can happen
  Weather.CloudChancePerDay = 20; //odds in percentage 0 to 100% chance
  Weather.CloudChannels = 255; //specifies which channels in bits (e.g. 11000000 would be daylight and actinic only, 00110000 would be PWM Expansion 0 and 1)
  Weather.LightningChannels = 255; //specifies which channels in bits (e.g. 11000000 would be daylight and actinic only, 00110000 would be PWM Expansion 0 and 1)
}

loop()
{
  Weather.InitActinic(); //initializes the min and max values based on memory values
  Weather.InitDaylight(); //initializes the min and max values based on memory values
  Weather.InitPWMChannel(0); //initializes the min and max values based on memory values
  Weather.InitPWMChannel(1); //initializes the min and max values based on memory values
  Weather.InitPWMChannel(2); //initializes the min and max values based on memory values
  Weather.InitPWMChannel(3); //initializes the min and max values based on memory values
  Weather.InitPWMChannel(4); //initializes the min and max values based on memory values
  Weather.InitPWMChannel(5); //initializes the min and max values based on memory values

  Weather.Execute(); //executes the weather

  Weather.RefreshPWMValues(); //sets the PWM values for your lights.
}