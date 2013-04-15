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