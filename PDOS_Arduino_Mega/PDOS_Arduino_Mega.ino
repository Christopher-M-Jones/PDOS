#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <SD.h>
#include "ACS712.h"
#include <LiquidCrystal_I2C.h>
#include <Time.h>
#include <RTClib.h>

#define SOLAR_JULY_DATA "sumsol.txt"
#define SOLAR_DEC_DATA "winsol.txt"
#define LOAD_DEMAND_JULY_DATA "sumload.txt"
#define LOAD_DEMAND_DEC_DATA "winload.txt"

/**********************************************************/
const int batterySelect = 0;            //0 = 3.7 V, 1 = 12 V                                              //Zero = Test Battery, One = Full Battery
const int gridPriceSelect = 0;          //0 = julyWeekend, 1 = julyWeekday, 2 = DecemeberWeekend, 3 = decemeberWeekday
const int solarSelect = 1;              //0 = solarJuly, 1 = solarDecmebr
const int loadDemandSelect = 1;         //0 = loadJuly, 1 = loadDecemebr
const int timeSelect = 0;               //Manual time input 0-23
const int rtcSelect = 0;                //0 = manual time input, 1 = real time
/**********************************************************/

DateTime now;
RTC_DS3231 rtc;

float solar_raw_July[24] = {};                                                                    //Array for storing July solar values
float solar_raw_December[24] = {};                                                                //Array for storing December solar values
float load_July[24] = {};                                                                         //Array for storing July load demand values
float load_December[24] = {};                                                                     //Array for storing December load demand values

int sd_read_loop = 0;                                                                             //Select for Loading SD Card values

union Solarbyte                                                                                   //Struct for Solar float transfer to NodeMCU
{
  uint8_t     solBytes[sizeof( float )];
  float       solVal;
};

Solarbyte sob;

union gridbyte
{
  uint8_t     gridBytes[sizeof( float )];                                                         //Struct for grid float transfer to NodeMCU
  float       gridVal;
};

gridbyte gob;

union battbyte
{
  uint8_t     battBytes[sizeof( float )];                                                         //Struct for battery float transfer to NodeMCU
  float       battVal;
};

battbyte bob;

struct wJson {                                                                                    //Struct for JSON weather transfer

  int cloudstruct;
  float tempstruct;
  int alertstruct;
  int alertStartstruct;

};

wJson Jweather;

struct ePlusVals {

  float Array0;
  float Array1;
  float Array2;
  float Array3;
  float Array4;
  float Array5;
  float Array6;
  float Array7;
  float Array8;
  float Array9;
  float Array10;
  float Array11;
  float Array12;
  float Array13;
  float Array14;
  float Array15;
  float Array16;
  float Array17;
  float Array18;
  float Array19;
  float Array20;
  float Array21;
  float Array22;
  float Array23;
};

ePlusVals solJuly;
ePlusVals solDecember;
ePlusVals loadJuly;
ePlusVals loadDecember;

LiquidCrystal_I2C lcd(0x27, 16, 2);                                 //Defines hex pin that LCD is assigned to. This may change depending on I2C module used.
#define sensorInput A0                                              //This sensor input may change based on where it is inputing data.
ACS712 sensor(ACS712_05B, sensorInput);                             //Take note of our sensor type. This value may also change.

/*Battery Management System Variables*/

//VOLTAGE VALUES
const float maxBatteryVoltage = 4.00;                                     //Voltage reading when battery is fully charged (100%)
const float minBatteryVoltage = 2.75;                                     //Voltage reading when battery is at 25%

//PIN ASSIGNMENT
const int chargingRelay = 5;                                        //Controls the charging subsystem relay
const int dischargingRelay = 4;                                     //Controls the discharging subsytem relay
const int buckConverterRelay = 6;
const int gridLedRelay = 12;
const int batteryLedRelay = 10;
const int solarLedRelay = 11;


//VARIABLE DECLARATIONS
const float batteryCapacity = 2000;                                       //Will change based on which battery is used. Units are mAH
const float batteryScale = 0.00074;

//CURRENT CHARACTERISTICS
float peakCurrentLimit = 0;
float cutOffCurrentLimit = 0;
float currentReading;
float constantVoltageCurrent = 0;

/*=================================================================================================================================================*/

void setup()
{
  Serial.begin(9600);
  Serial1.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  //RTCInitializer();
  pinMode(chargingRelay, OUTPUT);
  pinMode(dischargingRelay, OUTPUT);
  pinMode(batteryLedRelay, OUTPUT);
  pinMode(solarLedRelay, OUTPUT);
  pinMode(gridLedRelay, OUTPUT);
  pinMode(buckConverterRelay, OUTPUT);
}

void loop() {

  /*=================================================================================================================================================*/
  /*Generation Sources*/
  /*=================================================================================================================================================*/

  /*=================================================================================================================================================*/
  /*Gather SD Card Data*/

  //Read in Solar Panel Generation Data
  Serial.println("");
  Serial.println("Now Reading Summer Solar Data...");
  const char* solar_sum_string = ReadSDCard(SOLAR_JULY_DATA);

  delay(1000);

  Serial.println("");
  Serial.println("Now Reading Winter Solar Data...");
  const char* solar_win_string = ReadSDCard(SOLAR_DEC_DATA);

  delay(1000);

  //Read in Load Demand Data
  Serial.println("");
  Serial.println("Now Reading Summer Load Demand Data...");
  const char* load_demand_sum_string = ReadSDCard(LOAD_DEMAND_JULY_DATA);

  delay(1000);

  Serial.println("");
  Serial.println("Now Reading Winter Load Demand Data...");
  const char* load_demand_win_string = ReadSDCard(LOAD_DEMAND_DEC_DATA);

  //Parse Summer Solar data
  solJuly = ParseSDCardSolarSum(solar_sum_string);

  //Parse Winter Solar data
  solDecember = ParseSDCardSolarWin(solar_win_string);

  //Parse Summer load demand data
  loadJuly = ParseSDCardLoadSum(load_demand_sum_string);

  //Parse Winter load demand data
  loadDecember = ParseSDCardLoadWin(load_demand_win_string);

  //convert struct values to float array
  solar_raw_July[0] = solJuly.Array0;
  solar_raw_July[1] = solJuly.Array1;
  solar_raw_July[2] = solJuly.Array2;
  solar_raw_July[3] = solJuly.Array3;
  solar_raw_July[4] = solJuly.Array4;
  solar_raw_July[5] = solJuly.Array5;
  solar_raw_July[6] = solJuly.Array6;
  solar_raw_July[7] = solJuly.Array7;
  solar_raw_July[8] = solJuly.Array8;
  solar_raw_July[9] = solJuly.Array9;
  solar_raw_July[10] = solJuly.Array10;
  solar_raw_July[11] = solJuly.Array11;
  solar_raw_July[12] = solJuly.Array12;
  solar_raw_July[13] = solJuly.Array13;
  solar_raw_July[14] = solJuly.Array14;
  solar_raw_July[15] = solJuly.Array15;
  solar_raw_July[16] = solJuly.Array16;
  solar_raw_July[17] = solJuly.Array17;
  solar_raw_July[18] = solJuly.Array18;
  solar_raw_July[19] = solJuly.Array19;
  solar_raw_July[20] = solJuly.Array20;
  solar_raw_July[21] = solJuly.Array21;
  solar_raw_July[22] = solJuly.Array22;
  solar_raw_July[23] = solJuly.Array23;

  //convert struct values to float array
  solar_raw_December[0] = solDecember.Array0;
  solar_raw_December[1] = solDecember.Array1;
  solar_raw_December[2] = solDecember.Array2;
  solar_raw_December[3] = solDecember.Array3;
  solar_raw_December[4] = solDecember.Array4;
  solar_raw_December[5] = solDecember.Array5;
  solar_raw_December[6] = solDecember.Array6;
  solar_raw_December[7] = solDecember.Array7;
  solar_raw_December[8] = solDecember.Array8;
  solar_raw_December[9] = solDecember.Array9;
  solar_raw_December[10] = solDecember.Array10;
  solar_raw_December[11] = solDecember.Array11;
  solar_raw_December[12] = solDecember.Array12;
  solar_raw_December[13] = solDecember.Array13;
  solar_raw_December[14] = solDecember.Array14;
  solar_raw_December[15] = solDecember.Array15;
  solar_raw_December[16] = solDecember.Array16;
  solar_raw_December[17] = solDecember.Array17;
  solar_raw_December[18] = solDecember.Array18;
  solar_raw_December[19] = solDecember.Array19;
  solar_raw_December[20] = solDecember.Array20;
  solar_raw_December[21] = solDecember.Array21;
  solar_raw_December[22] = solDecember.Array22;
  solar_raw_December[23] = solDecember.Array23;

  //convert struct values to float array
  load_July[0] = loadJuly.Array0;
  load_July[1] = loadJuly.Array1;
  load_July[2] = loadJuly.Array2;
  load_July[3] = loadJuly.Array3;
  load_July[4] = loadJuly.Array4;
  load_July[5] = loadJuly.Array5;
  load_July[6] = loadJuly.Array6;
  load_July[7] = loadJuly.Array7;
  load_July[8] = loadJuly.Array8;
  load_July[9] = loadJuly.Array9;
  load_July[10] = loadJuly.Array10;
  load_July[11] = loadJuly.Array11;
  load_July[12] = loadJuly.Array12;
  load_July[13] = loadJuly.Array13;
  load_July[14] = loadJuly.Array14;
  load_July[15] = loadJuly.Array15;
  load_July[16] = loadJuly.Array16;
  load_July[17] = loadJuly.Array17;
  load_July[18] = loadJuly.Array18;
  load_July[19] = loadJuly.Array19;
  load_July[20] = loadJuly.Array20;
  load_July[21] = loadJuly.Array21;
  load_July[22] = loadJuly.Array22;
  load_July[23] = loadJuly.Array23;

  //convert struct values to float array
  load_December[0] = loadDecember.Array0;
  load_December[1] = loadDecember.Array1;
  load_December[2] = loadDecember.Array2;
  load_December[3] = loadDecember.Array3;
  load_December[4] = loadDecember.Array4;
  load_December[5] = loadDecember.Array5;
  load_December[6] = loadDecember.Array6;
  load_December[7] = loadDecember.Array7;
  load_December[8] = loadDecember.Array8;
  load_December[9] = loadDecember.Array9;
  load_December[10] = loadDecember.Array10;
  load_December[11] = loadDecember.Array11;
  load_December[12] = loadDecember.Array12;
  load_December[13] = loadDecember.Array13;
  load_December[14] = loadDecember.Array14;
  load_December[15] = loadDecember.Array15;
  load_December[16] = loadDecember.Array16;
  load_December[17] = loadDecember.Array17;
  load_December[18] = loadDecember.Array18;
  load_December[19] = loadDecember.Array19;
  load_December[20] = loadDecember.Array20;
  load_December[21] = loadDecember.Array21;
  load_December[22] = loadDecember.Array22;
  load_December[23] = loadDecember.Array23;

  /*Delete string objects from SD card reading*/
  delete solar_sum_string;
  delete solar_win_string;
  delete load_demand_sum_string;
  delete load_demand_win_string;

  /*Check the current system time*/
  int currentTime = Get_Time(timeSelect, rtcSelect);
  //int currentTime = 15;

  /*Gather JSON data */
  /*LISTEN FOR NODEMCU DATA*/

  int weatherarraylen = weatherArrayLength();

  const char* weatherjson = weatherSerialTransmit(weatherarraylen);
  Serial.println(F(""));
  Serial.print(F("WeatherJSON: "));
  Serial.println(weatherjson);

  int gridarraylen = gridArrayLength();

  const char* gridpricejson = gridSerialTransmit(gridarraylen);
  Serial.println(F(""));
  Serial.print(F("GridJSON: "));
  Serial.println(gridpricejson);

  /*PARSE DATA USING ARDUINOJSON*/
  Jweather = DeserializeWeather(weatherjson);

  /*OpenWeather variables: */
  int clouds = Jweather.cloudstruct;
  float temp = Jweather.tempstruct;
  int alert = Jweather.alertstruct;
  int alertStart = Jweather.alertStartstruct;

  Serial.println(F(""));
  Serial.println(F("Weather Variables:"));
  Serial.println(F(""));
  Serial.print(F("clouds: "));
  Serial.println(clouds);
  Serial.print(F("temperature: "));
  Serial.println(temp);
  Serial.print(F("alerts: "));
  Serial.println(alert);
  Serial.print(F("alert start time: "));
  Serial.println(alertStart);

  StaticJsonDocument<1024> gridpriceDoc;
  deserializeJson(gridpriceDoc, gridpricejson);

  /*OpenEI Grid Pricing variables: */
  int julyWeekdayHourly[24]; //integer arrays with 24 values
  int decWeekdayHourly[24];

  for (int i = 0; i <= 23; i++) {
    julyWeekdayHourly[i] = gridpriceDoc["julyWeekdayData"][i];
  }
  Serial.println(F(""));
  Serial.println(F(""));
  for (int i = 0; i <= 23; i++) {
    decWeekdayHourly[i] = gridpriceDoc["decWeekdayData"][i];
  }

  /*Print Grid Pricing Tier Variables*/
  Serial.println(F("Grid Variables - Summer Weekday:"));
  Serial.println(F(""));
  for (int i = 0; i <= 23; i++) {
    Serial.print(julyWeekdayHourly[i]);
    if(i <= 22){
      Serial.print(", ");
    }
    else{
      //do nothing
    }
  }

  Serial.println(F(""));
  Serial.println(F(""));
  Serial.println(F("Grid Variables - Winter Weekday:"));
  Serial.println(F(""));
  for (int i = 0; i <= 23; i++) {
    Serial.print(decWeekdayHourly[i]);
    if(i <= 22){
      Serial.print(", ");
    }
    else{
      //do nothing
    }
  }
  Serial.println(F(""));
  Serial.println(F(""));

  /*Hourly Grid Pricing Rates */
  float gridPriceHourlyRateZero = gridpriceDoc["Cat 1 Pricing"];
  float gridPriceHourlyRateOne = gridpriceDoc["Cat 2 Pricing"];
  float gridPriceHourlyRateTwo = gridpriceDoc["Cat 3 Pricing"];
  float gridPriceHourlyRateThree = gridpriceDoc["Cat 4 Pricing"];
  float gridPriceHourlyRateFour = gridpriceDoc["Cat 5 Pricing"];
  float gridPriceHourlyRateFive = gridpriceDoc["Cat 6 Pricing"];

  /*Print Grid Pricing Rate Variables*/
  Serial.print(F("Tier 1 Pricing: "));
  Serial.println(gridPriceHourlyRateZero);
  Serial.print(F("Tier 2 Pricing: "));
  Serial.println(gridPriceHourlyRateOne);
  Serial.print(F("Tier 3 Pricing: "));
  Serial.println(gridPriceHourlyRateTwo);
  Serial.print(F("Tier 4 Pricing: "));
  Serial.println(gridPriceHourlyRateThree);
  Serial.print(F("Tier 5 Pricing: "));
  Serial.println(gridPriceHourlyRateFour);
  Serial.print(F("Tier 6 Pricing: "));
  Serial.println(gridPriceHourlyRateFive);
  Serial.println(F(""));
  Serial.println(F(""));

  //Clear gridpriceDoc json document memory in the heap
  gridpriceDoc.clear();

  /*Delete string objects for Serial transfer-in */
  delete weatherjson;
  delete gridpricejson;

  /*=================================================================================================================================================*/
  /*Perform Generation Calculations*/

    /*Check Solar Irradiance value*/

    Serial.println(F("Setting Solar irradiance values...."));
    float solarIrradianceDecember = Solar_Irradiance_Calculation(solar_raw_December, temp, clouds, batteryScale, currentTime);
    float solarIrradianceJuly = Solar_Irradiance_Calculation(solar_raw_July, temp, clouds, batteryScale, currentTime);
    float solarIrradiance = Solar_Select_Val(solarSelect, solarIrradianceDecember, solarIrradianceJuly);

    Serial.print(F("December Solar Irradiance value: "));
    Serial.println(solarIrradianceDecember);


    Serial.print(F("July Solar Irradiance value: "));
    Serial.println(solarIrradianceJuly);

    Serial.print(F("Solar Irradiance value chosen: "));
    if(solarIrradiance == solarIrradianceDecember){
      Serial.print(F("December - "));
      Serial.print(solarIrradiance);
      Serial.println(" (W)");
    }
    else{
      Serial.print(F("July - "));
      Serial.print(solarIrradiance);
      Serial.println(F(" (W)"));
    }
    delay(1000);



    /*Check for inclement weather*/
    Serial.println(F(""));
    Serial.println(F("Checking severe weather..."));
    bool severeWeatherEvent = Check_Inclement_Weather(alert, alertStart);
    Serial.print(F("Severe weather value: "));

    if(severeWeatherEvent == 0){
      Serial.print(F("False"));
    }
    else{
      Serial.print(F("True"));
    }
    Serial.println(F(""));

    delay(1000);


    /*Check battery state of charge*/
    Serial.println(F(""));
    Serial.println(F("Checking battery state of charge..."));
    float StateOfCharge = Check_Battery_SOC();
    Serial.print(F("Battery state of Charge: "));
    Serial.println(StateOfCharge);
    Serial.println(F(""));
    delay(1000);


    /*Grid pricing values*/

    Serial.println(F("Calculating grid prices..."));
    float gridPricePerHourJulyWeekday = Grid_Pricing_Calculation(julyWeekdayHourly, gridPriceHourlyRateZero, gridPriceHourlyRateOne, gridPriceHourlyRateTwo, gridPriceHourlyRateThree, gridPriceHourlyRateFour, gridPriceHourlyRateFive, currentTime);
    float gridPricePerHourDecWeekday = Grid_Pricing_Calculation(decWeekdayHourly, gridPriceHourlyRateZero, gridPriceHourlyRateOne, gridPriceHourlyRateTwo, gridPriceHourlyRateThree, gridPriceHourlyRateFour, gridPriceHourlyRateFive, currentTime);

    Serial.print(F("Summer weekday grid price: "));
    Serial.println(gridPricePerHourJulyWeekday);

    Serial.print(F("Winter weekday grid price: "));
    Serial.println(gridPricePerHourDecWeekday);

    float gridPrice = Grid_Price_Select(gridPriceSelect, gridPricePerHourJulyWeekday, gridPricePerHourDecWeekday);

    if(gridPrice == gridPricePerHourJulyWeekday){
      Serial.print(F("Current grid price: "));
      Serial.print(F("Summer Weekday - $"));
      Serial.println(gridPrice);
    }
    else{
      Serial.print(F("Current grid price: "));
      Serial.print(F("Winter Weekday - $"));
      Serial.println(gridPrice);
    }

    delay(1000);


    //Check if Solar Output threshold is met
    Serial.println(F(""));
    bool SolarOutputThreshold = CheckSolarOutputThreshold(solarIrradiance);
    Serial.print(F("Solar meets output threshold: "));

    if(SolarOutputThreshold == 0){
      Serial.println(F("False"));
    }
    else{
      Serial.println(F("True"));
    }

    delay(1000);


    //Check if Battery Output Threshold is met
    bool BatteryOutputThreshold = CheckBatteryOutputThreshold(StateOfCharge);
    Serial.print(F("Battery meets output threshold: "));

    if(BatteryOutputThreshold == 0){
      Serial.println(F("False"));
    }
    else{
      Serial.println(F("True"));
    }

    Serial.println(F(""));
    delay(1000);


    //Determine which load demand values to use
    Serial.println(F("Determining load demand...."));
    float load_demand = Load_Demand_Select(loadDemandSelect, load_July, load_December, batteryScale, currentTime);
    Serial.print(F("Current load demand: "));
    Serial.print(load_demand);
    Serial.println(F(" (W)"));
    delay(1000);

    //Check if battery meets load demand
    bool batteryMeetsLoadDemand = Check_Battery_Against_Load(StateOfCharge, load_demand);

    //Check if solar meets load demand
    bool solarLoadCheck = Check_Solar_Against_Load(solarIrradiance, load_demand);

    //Fill Power Slots with Generation Sources
    Serial.println(F(""));
    Serial.println(F("Calculating power slots...."));
    Serial.println(F(""));
    //PowerSlots = FillPowerSlots(solarIrradiance, SolarOutputThreshold, severeWeatherEvent, BatteryOutputThreshold, load_demand, StateOfCharge, batteryScale, gridPrice, batteryMeetsLoadDemand, solarLoadCheck);

    //delete[] weatherSerialTransmit;
    //delete[] gridSerialTransmit;
    //delete[] cstr;
    //delete[] cstr2;
    //delete[] cstr3;


    //PowerSlots: 0 = "EMPTY", 1 = "SOLAR", 2 = "BATTERY", 3 = "GRID"

    int powerSlotOne = 0;
    int powerSlotTwo = 0;

    //Check Solar Output Threshold
    //Serial.println("Now checking solar output threshold...");
    //Serial.println(SolarOutputThreshold);
    if (SolarOutputThreshold == false)                                                     //Solar Output doesnt meet the 1.2W threshold, so solar can't be used in Slot 1
    {
      //Serial.println("Solar output threshold is false.");
      //Serial.println("Now checking Battery output threshold...");
      //Serial.println(BatteryOutputThreshold);
      //Serial.println("Now checking Severe weather alert");
      //Serial.println(severeWeatherEvent);
      //Check The Battery and Inclement Weather
      if ((BatteryOutputThreshold == true) && (severeWeatherEvent == false))             //Battery has 35% capacity and there is no inclement weather, so battery can be used in Slot 1
      {
        //Serial.println("Battery threshold true and severe weather event false");
        //Serial.println("Now checking severe weather alert: ");
        //Serial.println(severeWeatherEvent);
        //Check the battery against load demand
        Serial.print(F("Battery meets load demand: "));

        if(batteryMeetsLoadDemand == 0){
          Serial.println(F("False"));
        }
        else{
          Serial.println(F("True"));
        }

        Serial.print(F("Severe weather alert: "));

        if(severeWeatherEvent == 0){
          Serial.println(F("False"));
        }
        else{
          Serial.println(F("True"));
        }


        if (batteryMeetsLoadDemand == false)                                          //Need to add battery meets demand function and insert it here. Battery does not meet the load demand
        {
          //Serial.print("Battery does not meet load");
          Serial.print(F("Grid price is < $0.09: "));
          //Serial.println(gridPrice);

          if (gridPrice < 0.09)
          {
            //Serial.println("Grid price is less than 9 cents");
            //Serial.println("Grid is power slot 1");
            //Grid is placed in slot #1
            powerSlotOne = 3;
            //Slot #2 remains empty
            powerSlotTwo = 0;
            //Serial.println("Now checking severe weather alert: ");
            //Serial.println(severeWeatherEvent);
            Serial.println(F("True"));

            digitalWrite(batteryLedRelay, LOW);                                         //Switches off the battery LED
            digitalWrite(gridLedRelay, HIGH);                                           //Switches on the grid LED
            digitalWrite(solarLedRelay, LOW);
            ChargeBattery();                                                            //Charges battery
          }

          else
          {
            //Serial.println("Grid price is more than 9 cents.");
            //Serial.println("Now checking severe weather alert: ");
            //Serial.println(severeWeatherEvent);
            //Battery is placed in slot #1
            powerSlotOne = 2;
            //Grid is placed in slot #2
            powerSlotTwo = 3;

            Serial.println(F("False"));

            digitalWrite(batteryLedRelay, HIGH);                                        //Switches off the battery LED
            digitalWrite(gridLedRelay, HIGH);                                           //Switches on the grid LED
            digitalWrite(solarLedRelay, LOW);                                           //Switches off the solar LED
            DischargeBattery(StateOfCharge);                                                      //Discharges battery
          }
        }

        else                                                                            //Battery meets load demand
        {
          //Serial.print("battery is now placed in slot 1 and covers load.");
          //Serial.println("Now checking severe weather alert: ");
          //Serial.println(severeWeatherEvent);
          //Battery is place in Slot #1
          powerSlotOne = 2;
          //No source is placed in Slot #2
          powerSlotTwo = 0;

          digitalWrite(batteryLedRelay, HIGH);                                          //Switches on the battery LED
          digitalWrite(gridLedRelay, LOW);                                              //Switches off the grid LED
          digitalWrite(solarLedRelay, LOW);                                             //Switches off the solar LED
          DischargeBattery(StateOfCharge);                                                        //Discharges the battery
        }
      }

      else                                                                                //Battery either is at 35% capacity or there is inclement weather
      {
        //Grid is placed in slot #1
        powerSlotOne = 3;
        //No source is placed in Slot #2
        powerSlotTwo = 0;

        digitalWrite(gridLedRelay, HIGH);                                                 //Switches on the grid LED
        digitalWrite(batteryLedRelay, LOW);                                               //Switches off the battery LED
        digitalWrite(solarLedRelay, LOW);                                                 //Switches off the solar LED
        ChargeBattery();                                                                  //Charges battery
      }
    }

    else                                                                                    //Solar output meets the 1.2W threshold
    {
      //Assign The first power slot with Solar
      powerSlotOne = 1;
      //Serial.println("Power slot 1 belongs to: ");
      //Serial.println(powerSlotOne);
      digitalWrite(solarLedRelay, HIGH);                                                  //Switches on the solar LED
      //Check Solar against load demand
      Serial.print(F("Solar meets load demand: "));

      if(solarLoadCheck == 0){
        Serial.println(F("False"));
      }
      else{
        Serial.println(F("True"));
      }



      if (solarLoadCheck == false)
      {
        //Serial.println("Solar does not meet load");
        //Check if Battery threshold conditions are met
        Serial.print(F("Battery threshold met: "));

        if(BatteryOutputThreshold == 0){
          Serial.println(F("False"));
        }
        else{
          Serial.println(F("True"));
        }


        Serial.print(F("Severe weather event: "));

        if(severeWeatherEvent == 0){
          Serial.println(F("False"));
        }
        else{
          Serial.println(F("True"));
        }
        Serial.println(F(""));


        if ((BatteryOutputThreshold == true) && (severeWeatherEvent == false))           //Battery is above 25% capacity remaining and there is no inclemenet weather
        {
          //Serial.println("Battery meets threshold and no severe weather event");
          //Serial.println("Line 1243 severe weather status: ");
          //Serial.println(severeWeatherEvent);
          //Subtract Solar from load demand
          Serial.println(F("Subtracting solar from the load demand...."));
          load_demand -= solarIrradiance;
          Serial.print(F("New load demand value: "));
          Serial.println(load_demand);
          Serial.println(F(""));

          //Check the battery against load demand
          Serial.println(F("Battery meets load demand: "));

          if(batteryMeetsLoadDemand == 0){
            Serial.println(F("False"));
          }
          else{
            Serial.println(F("True"));
          }
          Serial.println(F(""));

          if (batteryMeetsLoadDemand == false)                                       //Battery does not meet the load demand
          {
            //Grid is placed in Slot #2
            powerSlotTwo = 3;
            //Serial.println("Power slot 1: ");
            //Serial.println(powerSlotOne);
            //Serial.println("Power slot 2: ");
            //Serial.println(powerSlotTwo);

            digitalWrite(gridLedRelay, HIGH);                                     //Switches on the grid LED
            digitalWrite(batteryLedRelay, LOW);                                   //Switches off the battery LED
            ChargeBattery();                                                      //Charges the battery
          }

          else                                                                      //Battery meets the demand
          {
            powerSlotTwo = 2;
            //Serial.println("Power Slot 1: ");
            //Serial.println(powerSlotOne);
            //Serial.println("Power Slot 2: ");
            //Serial.println(powerSlotTwo);

            digitalWrite(batteryLedRelay, HIGH);                                  //Switches on the battery LED
            digitalWrite(gridLedRelay, LOW);                                      //Switches off the grid LED
            DischargeBattery(StateOfCharge);                                                //Dicharges the battery
          }
        }

        else
        {
          //Serial.println("Battery either doesn't meet demand or severe weather event is true");
          //Serial.println("Power slot 2: ");
          //Serial.println(powerSlotTwo);
          powerSlotTwo = 3;

          digitalWrite(gridLedRelay, HIGH);
          digitalWrite(batteryLedRelay, LOW);
          ChargeBattery();
        }
      }

      else
      {
        powerSlotTwo = 0;
        //Serial.println("Power Slot 1: ");
        //Serial.println(powerSlotOne);
        //Serial.println("Power Slot 2: ");
        //Serial.println(powerSlotTwo);
        digitalWrite(gridLedRelay, LOW);                                                //Switches off grid LED
        digitalWrite(batteryLedRelay, LOW);                                             //Switches off battery LED
        ChargeBattery();                                                                //Charges the battery
      }

    }

    Serial.println(F(""));
    Serial.print(F("Powerslot #1: "));
    Serial.println(powerSlotOne);
    Serial.print(F("Powerslot #2: "));
    Serial.println(powerSlotTwo);
    Serial.println(F(""));

    //Transfer data to Raspberry pi
    Serial.println(F("Transmitting data to NodeMCU...."));
    TransmitRaspberryToNode(powerSlotOne, powerSlotTwo, solarIrradiance, gridPrice, StateOfCharge);
    Serial.println(F(""));
    Serial.println(F("Main loop finished...."));

    //Ten minute delay (milliseconds. 1000 = 1sec. 30sec maximum per delay)
    delay(30000);

    /*
    delay(30000);
    delay(30000);
    delay(30000);
    delay(30000);
    delay(30000);
    delay(30000);
    delay(30000);
    delay(30000);
    delay(30000);
    delay(30000);
    delay(30000);
    delay(30000);
    delay(30000);
    delay(30000);
    delay(30000);
    delay(30000);
    delay(30000);
    delay(30000);
    delay(30000);
    */

}

/*=================================================================================================================================================*/
/*solar select*/

float Solar_Select_Val(int solarSelect, float solarIrradianceDecember, float solarIrradianceJuly) {

  float irradiance = 0;

  //Decides which solar irradiance calculation is based on the solar select variable
  switch (solarSelect) {

  case 0:
    irradiance = solarIrradianceJuly;
    break;

  case 1:
    irradiance = solarIrradianceDecember;
    break;

  }

  return irradiance;

}

/*=================================================================================================================================================*/
/*grid select*/
float Grid_Price_Select(int gridPriceSelect, float gridPricePerHourJulyWeekday, float gridPricePerHourDecWeekday) {

  float pricing = 0;

  //Decides which grid price calculation is used based on the grid price select variable
  switch (gridPriceSelect) {

  case 0:
    pricing = gridPricePerHourJulyWeekday;
    break;

  case 1:
    pricing = gridPricePerHourDecWeekday;
    break;

  }

  return pricing;

}

/*=================================================================================================================================================*/
/*load select*/
float Load_Demand_Select(int loadDemandSelect, float load_July[], float load_December[], float batteryScale, int currentTime) {

  float loadDemandjuly = (load_July[currentTime] * batteryScale);                   //Calculates the current load demand in July
  float loadDemanddec = (load_December[currentTime] * batteryScale);                //Calculates the current load demand in December

  float demarray = 0;                                                               //Demand array

  //Decides which load array is used based on the load demand select variable
  switch (loadDemandSelect) {

  case 0:
    demarray = loadDemandjuly;
    break;

  case 1:
    demarray = loadDemanddec;
    break;
  }
  return demarray;
}

/*=================================================================================================================================================*/
/*battery select*/

float Battery_Select(int batterySelect) {

  float batteryScale = 0;

  if (batterySelect == 0) {
    batteryScale = 0.00074;
  }

  else {
    batteryScale = 0.024;
  }

  return batteryScale;

}
/*=================================================================================================================================================*/
/*Specify time input for the program*/


int Get_Time(int timeSelect, int rtcSelect)
{
  int time_of_day = 15;

  //Determines whether time is determined by Real time clock peripheral or manual input
  if(rtcSelect == 1){
    //time_of_day = rtc.now().hour();
  }
  else{
    //time_of_day = timeSelect;                                 //user-defined input
  }

  return time_of_day;
}


/*=================================================================================================================================================*/
/*Get Weather Serial JSON Length*/

int weatherArrayLength() {

  String intcontent = "";
  char intcharacter = '\0';
  int weatherint = 0;

  Serial.println(F(""));
  Serial.println(F("Waiting on Weather Serial data...."));
  while (!Serial1.available()) {
    //do nothing
  }

  delay(100);

  while (Serial1.available()) {
    intcharacter = Serial1.read();
    intcontent.concat(intcharacter);
  }

  weatherint = intcontent.toInt();

  return weatherint;
}


/*=================================================================================================================================================*/
/*Get Grid Price Serial JSON Length*/

int gridArrayLength() {

  String gridintcontent = "";
  char gridintcharacter = '\0';
  int gridint = 0;

  //Serial.println(F("Waiting on Grid Length Serial data...."));
  while (!Serial1.available()) {
    //do nothing
  }

  delay(100);

  while (Serial1.available()) {
    gridintcharacter = Serial1.read();
    gridintcontent.concat(gridintcharacter);
  }

  gridint = gridintcontent.toInt();

  return gridint;
}

/*=================================================================================================================================================*/
/*Get Weather Serial JSON*/

const char* weatherSerialTransmit(int arrayLength) {

  String content = "";
  char character = '\0';


  while (!Serial1.available()) {
    //do nothing
  }
  //Serial.println("Data found");
  delay(100);

  while (arrayLength >= 0) {

    //Serial.println("Entering array length loop");
    while (Serial1.available()) {

      //Serial.println("Entering available loop");
      character = Serial1.read();
      content.concat(character);
      arrayLength--;
      //Serial.print(arrayLength);
    }
  }

  //Store SD Card data in char array in heap
  char* cstr = new char[content.length() + 1];
  strcpy(cstr, content.c_str());

  return cstr;
}

/*=================================================================================================================================================*/
/*Get Grid Price Serial JSON*/

const char* gridSerialTransmit(int arrayLength) {

  String gridcontent = "";
  char gridcharacter = '\0';

  Serial.println(F(""));
  Serial.println(F("Waiting on Grid Pricing Serial data...."));
  while (!Serial1.available()) {
    //do nothing
  }

  //Serial.println("Serial available");

  delay(100);

  while (arrayLength >= 0) {
    while (Serial1.available()) {
      gridcharacter = Serial1.read();
      gridcontent.concat(gridcharacter);
      arrayLength--;
      //Serial.print(arrayLength);
    }
  }

  //Store SD Card data in char array in heap
  char* cstr2 = new char[gridcontent.length() + 1];
  strcpy(cstr2, gridcontent.c_str());

  return cstr2;
}

/*=================================================================================================================================================*/
/*Initialize and read the SD card*/

const char* ReadSDCard(String SD_file){

  String sd_data = "";
  char letter;

  //Initialize SD Card
  Serial.print(F("Initializing SD card..."));

  if (!SD.begin(53)) {
    Serial.println(F("initialization failed!"));
    while (1);
  }
  Serial.println(F("initialization done."));

  //Open file for reading:
  File myFile = SD.open(SD_file);

  if (myFile) {
    //read from file until empty
    while (myFile.available()) {
      letter = myFile.read();
      sd_data.concat(letter);
      Serial.print(letter);
      delay(5);
    }
    // close the file:
    myFile.close();
  }

  else {
    //Error: file didn't open
    Serial.println("error: could not open " + SD_file);
  }

  //Store SD Card data in char array in heap
  char* cstr3 = new char [sd_data.length()+1];
  strcpy(cstr3, sd_data.c_str());


  return cstr3;
}

/*=================================================================================================================================================*/
/*Initialize and read the API keys from the SD card*/

const char* ReadAPIKey(String SD_file){

  String sd_data = "";
  char letter;

  //Initialize SD Card
  Serial.print(F("Initializing SD card..."));

  if (!SD.begin(53)) {
    Serial.println(F("initialization failed!"));
    while (1);
  }
  Serial.println(F("initialization done."));

  //Open file for reading:
  File myFile = SD.open(SD_file);

  if (myFile) {
    //read from file until empty
    while (myFile.available()) {
      letter = myFile.read();
      sd_data.concat(letter);
      Serial.print(letter);
      delay(5);
    }
    // close the file:
    myFile.close();
  }

  else {
    //Error: file didn't open
    Serial.println("error: could not open " + SD_file);
  }

  //Store SD Card data in char array in heap
  char* cstr4 = new char [sd_data.length()+1];
  strcpy(cstr4, sd_data.c_str());


  return cstr4;
}

/*=================================================================================================================================================*/
/*Parse Solar Summer Values from SD card*/

ePlusVals ParseSDCardSolarSum(const char* sd_array) {

  //Create JSON document based on which data to use
  StaticJsonDocument<512> SolarJulyDoc;
  deserializeJson(SolarJulyDoc, sd_array);

  solJuly.Array0 = SolarJulyDoc["1am (W)"];
  solJuly.Array1 = SolarJulyDoc["2am (W)"];
  solJuly.Array2 = SolarJulyDoc["3am (W)"];
  solJuly.Array3 = SolarJulyDoc["4am (W)"];
  solJuly.Array4 = SolarJulyDoc["5am (W)"];
  solJuly.Array5 = SolarJulyDoc["6am (W)"];
  solJuly.Array6 = SolarJulyDoc["7am (W)"];
  solJuly.Array7 = SolarJulyDoc["8am (W)"];
  solJuly.Array8 = SolarJulyDoc["9am (W)"];
  solJuly.Array9 = SolarJulyDoc["10am (W)"];
  solJuly.Array10 = SolarJulyDoc["11am (W)"];
  solJuly.Array11 = SolarJulyDoc["12pm (W)"];
  solJuly.Array12 = SolarJulyDoc["1pm (W)"];
  solJuly.Array13 = SolarJulyDoc["2pm (W)"];
  solJuly.Array14 = SolarJulyDoc["3pm (W)"];
  solJuly.Array15 = SolarJulyDoc["4pm (W)"];
  solJuly.Array16 = SolarJulyDoc["5pm (W)"];
  solJuly.Array17 = SolarJulyDoc["6pm (W)"];
  solJuly.Array18 = SolarJulyDoc["7pm (W)"];
  solJuly.Array19 = SolarJulyDoc["8pm (W)"];
  solJuly.Array20 = SolarJulyDoc["9pm (W)"];
  solJuly.Array21 = SolarJulyDoc["10pm (W)"];
  solJuly.Array22 = SolarJulyDoc["11pm (W)"];
  solJuly.Array23 = SolarJulyDoc["12am (W)"];

  SolarJulyDoc.clear();

  return solJuly;
}


/*=================================================================================================================================================*/
/*Parse Solar Winter Values from SD card*/

ePlusVals ParseSDCardSolarWin(const char* sd_array) {

  //Create JSON document based on which data to use
  StaticJsonDocument<512> SolarWinDoc;
  deserializeJson(SolarWinDoc, sd_array);

  solDecember.Array0 = SolarWinDoc["1am (W)"];
  solDecember.Array1 = SolarWinDoc["2am (W)"];
  solDecember.Array2 = SolarWinDoc["3am (W)"];
  solDecember.Array3 = SolarWinDoc["4am (W)"];
  solDecember.Array4 = SolarWinDoc["5am (W)"];
  solDecember.Array5 = SolarWinDoc["6am (W)"];
  solDecember.Array6 = SolarWinDoc["7am (W)"];
  solDecember.Array7 = SolarWinDoc["8am (W)"];
  solDecember.Array8 = SolarWinDoc["9am (W)"];
  solDecember.Array9 = SolarWinDoc["10am (W)"];
  solDecember.Array10 = SolarWinDoc["11am (W)"];
  solDecember.Array11 = SolarWinDoc["12pm (W)"];
  solDecember.Array12 = SolarWinDoc["1pm (W)"];
  solDecember.Array13 = SolarWinDoc["2pm (W)"];
  solDecember.Array14 = SolarWinDoc["3pm (W)"];
  solDecember.Array15 = SolarWinDoc["4pm (W)"];
  solDecember.Array16 = SolarWinDoc["5pm (W)"];
  solDecember.Array17 = SolarWinDoc["6pm (W)"];
  solDecember.Array18 = SolarWinDoc["7pm (W)"];
  solDecember.Array19 = SolarWinDoc["8pm (W)"];
  solDecember.Array20 = SolarWinDoc["9pm (W)"];
  solDecember.Array21 = SolarWinDoc["10pm (W)"];
  solDecember.Array22 = SolarWinDoc["11pm (W)"];
  solDecember.Array23 = SolarWinDoc["12am (W)"];

  SolarWinDoc.clear();

  return solDecember;
}

/*=================================================================================================================================================*/
/*Parse Load Demand Summer Values from SD card*/

ePlusVals ParseSDCardLoadSum(const char* sd_array) {

  //Create JSON document based on which data to use
  StaticJsonDocument<512> LoadSumDoc;
  deserializeJson(LoadSumDoc, sd_array);

  loadJuly.Array0 = LoadSumDoc["1am (W)"];
  loadJuly.Array1 = LoadSumDoc["2am (W)"];
  loadJuly.Array2 = LoadSumDoc["3am (W)"];
  loadJuly.Array3 = LoadSumDoc["4am (W)"];
  loadJuly.Array4 = LoadSumDoc["5am (W)"];
  loadJuly.Array5 = LoadSumDoc["6am (W)"];
  loadJuly.Array6 = LoadSumDoc["7am (W)"];
  loadJuly.Array7 = LoadSumDoc["8am (W)"];
  loadJuly.Array8 = LoadSumDoc["9am (W)"];
  loadJuly.Array9 = LoadSumDoc["10am (W)"];
  loadJuly.Array10 = LoadSumDoc["11am (W)"];
  loadJuly.Array11 = LoadSumDoc["12pm (W)"];
  loadJuly.Array12 = LoadSumDoc["1pm (W)"];
  loadJuly.Array13 = LoadSumDoc["2pm (W)"];
  loadJuly.Array14 = LoadSumDoc["3pm (W)"];
  loadJuly.Array15 = LoadSumDoc["4pm (W)"];
  loadJuly.Array16 = LoadSumDoc["5pm (W)"];
  loadJuly.Array17 = LoadSumDoc["6pm (W)"];
  loadJuly.Array18 = LoadSumDoc["7pm (W)"];
  loadJuly.Array19 = LoadSumDoc["8pm (W)"];
  loadJuly.Array20 = LoadSumDoc["9pm (W)"];
  loadJuly.Array21 = LoadSumDoc["10pm (W)"];
  loadJuly.Array22 = LoadSumDoc["11pm (W)"];
  loadJuly.Array23 = LoadSumDoc["12am (W)"];

  LoadSumDoc.clear();

  return loadJuly;
}

/*=================================================================================================================================================*/
/*Parse Load Demand Summer Values from SD card*/

ePlusVals ParseSDCardLoadWin(const char* sd_array) {

  //Create JSON document based on which data to use
  StaticJsonDocument<512> LoadWinDoc;
  deserializeJson(LoadWinDoc, sd_array);

  loadDecember.Array0 = LoadWinDoc["1am (W)"];
  loadDecember.Array1 = LoadWinDoc["2am (W)"];
  loadDecember.Array2 = LoadWinDoc["3am (W)"];
  loadDecember.Array3 = LoadWinDoc["4am (W)"];
  loadDecember.Array4 = LoadWinDoc["5am (W)"];
  loadDecember.Array5 = LoadWinDoc["6am (W)"];
  loadDecember.Array6 = LoadWinDoc["7am (W)"];
  loadDecember.Array7 = LoadWinDoc["8am (W)"];
  loadDecember.Array8 = LoadWinDoc["9am (W)"];
  loadDecember.Array9 = LoadWinDoc["10am (W)"];
  loadDecember.Array10 = LoadWinDoc["11am (W)"];
  loadDecember.Array11 = LoadWinDoc["12pm (W)"];
  loadDecember.Array12 = LoadWinDoc["1pm (W)"];
  loadDecember.Array13 = LoadWinDoc["2pm (W)"];
  loadDecember.Array14 = LoadWinDoc["3pm (W)"];
  loadDecember.Array15 = LoadWinDoc["4pm (W)"];
  loadDecember.Array16 = LoadWinDoc["5pm (W)"];
  loadDecember.Array17 = LoadWinDoc["6pm (W)"];
  loadDecember.Array18 = LoadWinDoc["7pm (W)"];
  loadDecember.Array19 = LoadWinDoc["8pm (W)"];
  loadDecember.Array20 = LoadWinDoc["9pm (W)"];
  loadDecember.Array21 = LoadWinDoc["10pm (W)"];
  loadDecember.Array22 = LoadWinDoc["11pm (W)"];
  loadDecember.Array23 = LoadWinDoc["12am (W)"];

  LoadWinDoc.clear();

  return loadDecember;
}

/*=================================================================================================================================================*/
/*Checks the current battery state of charge*/

float Check_Battery_SOC()
{
  digitalWrite(buckConverterRelay, LOW);
  delay(1000);
  int sensorValue = analogRead(A1);                                                                                    //read the A1 pin value
  float voltageReading = sensorValue * (5.0 / 1023.00);                                                              //Convert the value to a true voltage.
  delay(1000);
  digitalWrite(buckConverterRelay, HIGH);
  return voltageReading;                                                                                               //Returns the voltage level
}

/*=================================================================================================================================================*/
/*Checks the current battery state of charge*/

void printChargePercentage(float voltageReading)
{
  //Calculate Percent Charge
  float chargePercentage = ((voltageReading - minBatteryVoltage) / (maxBatteryVoltage - minBatteryVoltage)) * 100;       //Calculates the % battery remaining

  Serial.print(F("Current Battery Charge: "));
  Serial.print(chargePercentage);
  Serial.println(F("%"));
  Serial.println(F(""));

  //Print Charge to LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Charge = ");
  lcd.print(chargePercentage);                                                                                         //Print the voltage to LCD
  lcd.print(" %");                                                                                                     //Returns the voltage level

}

/*=================================================================================================================================================*/
/*Checks for any relevant weather alerts*/

bool Check_Inclement_Weather(int alert, int alertStart)
{

  bool inclement_weather = false;

  //int utcTime = rtc.now().unixtime();

  //Checks the OpenWeather Alert Value to see if its null
  if (alert == 0)                                                                       //Checks the API input to see if there is an alert.
  {                                                                                    //If there is no alert, then the bool is false,
    return inclement_weather = false;                                                  //If there is an alert, then the bool is true
  }
  else
  {
    /*
    if(alertStart <= utcTime){
      return inclement_weather = false;
    }
    */
    return inclement_weather = true;
  }
}

/*=================================================================================================================================================*/
/*Calculates the true solar output*/

float Solar_Irradiance_Calculation(float solar_raw[], float temp, int clouds, float batteryScale, int currentTime)
{
  //Setting output and efficiency values

  float solarOutputBaseValue = solar_raw[currentTime];
  float temp_efficiency_change = 0;

  //Convert temperature in Kelvin to Celsius
  float temp_c = temp - 273;

  //Calculate Solar panel efficiency loss based on cloud coverage
  float cloud_efficiency_change = ((100 - clouds) * .01);

  //Calculate Solar panel efficiency loss based on ambient temperature if Celsius > 25
  if (temp_c > 25) {
    temp_efficiency_change = (temp_c - 25) * 0.5 / 100;           //need to specify this calculation in document
  }
  else {
    temp_efficiency_change = 0;
  }

  //Calculate new Solar Irradiance
  float solarIrradiance = (solarOutputBaseValue * cloud_efficiency_change) - (solarOutputBaseValue * temp_efficiency_change);

  //Scale Irradiance Value based on Battery being used
  solarIrradiance = solarIrradiance * batteryScale;

  return solarIrradiance;
}

/*=================================================================================================================================================*/

//Function takes one of the Grid tier arrays, the 6 tier price floats, and current time
float Grid_Pricing_Calculation(int grid_tier_array[], float HourlyRateZero, float HourlyRateOne, float HourlyRateTwo, float HourlyRateThree, float HourlyRateFour, float HourlyRateFive, int currentTime)
{
  //Grid Pricing set by time of day from OpenEI Data
  int grid_price_tier = grid_tier_array[currentTime];
  float pricePerHour = 0;

  //Calculates price based on tier value obtained from the grid array
  switch (grid_price_tier) {

  case 0:
    pricePerHour = HourlyRateZero;
    break;

  case 1:
    pricePerHour = HourlyRateOne;
    break;

  case 2:
    pricePerHour = HourlyRateTwo;
    break;

  case 3:
    pricePerHour = HourlyRateThree;
    break;

  case 4:
    pricePerHour = HourlyRateFour;
    break;

  case 5:
    pricePerHour = HourlyRateFive;
    break;

  }
  return pricePerHour;
}

/*=================================================================================================================================================*/

/*======================================================= Begin Source-Switching Functions ========================================================*/

/*=================================================================================================================================================*/
/*Checks if Solar Threshold meets minimum (50W or 0.074W Depending on the battery being tested)*/
//0.037 for 3.7v???

bool CheckSolarOutputThreshold(float solarIrradiance)
{

  //Check Solar Irradiance against 1.2W threshold for small battery, 50W threshold for full size battery
  if (solarIrradiance < 0.037) {
    return false;
  }
  else {
    return true;
  }
}

/*=================================================================================================================================================*/
/*Checks if battery Threshold meets minimum (35%)*/

bool CheckBatteryOutputThreshold(float StateOfCharge)
{
  if (StateOfCharge <= 3.1875) {
    return false;
  }
  else {
    return true;
  }
}

/*=================================================================================================================================================*/
/*Checks to see if solar output meets load demand and gives the excess or insufficient amount of power */

bool Check_Solar_Against_Load(float solarIrradiance, float load_Demand)
{

  float solar_load_power_output = solarIrradiance - load_Demand;                          //Calculates if solar meets the load demand

  if (solar_load_power_output >= 0)                                                       //Determines if solar meets the load
  {
    return true;
  }
  else
  {
    return false;
  }
}

/*=================================================================================================================================================*/
/*Charges the battery if solar power output exceeds the load demand unless the battery is already at 100%*/
/*we would take the input from the check solar against load above and if the value is positive we check the batter SOC*/

void ChargeBattery()
{
  Serial.println(F(""));
  Serial.println(F("Charging battery..."));
  Serial.println(F(""));
  digitalWrite(dischargingRelay, LOW);                        //Disconnects the discharging circuit for safety reasons
  peakCurrentLimit = (batteryCapacity) * (0.7) * (0.001);           //Sets peak current limit to determine if there is overload
  cutOffCurrentLimit = (batteryCapacity) * (0.1) * (0.001);         //Determines when the current is too low and battery should say it is fully charged

  Serial.println(F("Calibrating current..."));
  Serial.println(F(""));
  currentCalibration();
  Serial.println(F("Constant Current Constant Voltage..."));
  Serial.println(F(""));                                       //Calls the function to calibrate the current
  //constantCurrentConstantVoltage();                           //Calls function to determine what mode the battery is charged with

  //Constant Current Constant Voltage
  lcd.clear();                                                        //Clears LCD screen
  lcd.setCursor(0, 0);                                                //Sets where statement is printed
  lcd.print("Analyzing CC/CV");                                       //Outputs to user what is happening
  lcd.setCursor(4, 1);                                                //Sets where statement is printed
  lcd.print("Modes...");
  digitalWrite(chargingRelay, HIGH);                                  //Closes the relay to activate the circuit
  for (int i = 0; i < 20; i++)
  {
    currentReading = sensor.getCurrentDC();                           //Gets current reading 20 times in 2 seconds, getCurrent is located in ACS712.cpp
    delay(100);
  }

  constantVoltageCurrent = (currentReading * 0.8);                      //If current is the correct reading the constant voltage current is calculated

  for (int i = 0; i < 10; i++)                                    //Gets current reading 10 times in 1 second
  {
    currentReading = sensor.getCurrentDC();                   //Calls function to get DC current and stores it in current reading variable. Located in ACS712.cpp
    delay(100);
  }

  Serial.println(F("Recalibrating battery..."));
  Serial.println(F(""));
  digitalWrite(chargingRelay, LOW);                               //Opens the circuit
  currentCalibration();                                           //Calls the sensor to calbrate
  digitalWrite(chargingRelay, HIGH);                              //Closes the circuit
  lcd.clear();
  lcd.setCursor(0, 0);

  if (currentReading <= constantVoltageCurrent)               //Determines if current reading is less than or equal to constant voltage current
  {
    lcd.setCursor(4, 0);
    lcd.print("MODE: CV");                                    //If current reading is less than or equal to constant voltage current than the battery is charged using constant voltage mode                                                //If current reading is less than or equal to constant voltage current than the battery is charged using constant voltage mode
  }

  if (currentReading > constantVoltageCurrent)                //Checks current reading against constant voltage current
  {
    lcd.setCursor(4, 0);
    lcd.print("MODE: CC");                                    //Tells the user that the battery is being charged using constant current mode

  }

  currentReading = sensor.getCurrentDC();

  lcd.setCursor(0, 1);
  lcd.print("Current: ");
  lcd.print(currentReading);
  lcd.print(" A");
  delay(2000);

  if (currentReading <= cutOffCurrentLimit)                   //Checks current reading against cutoff current limit
  {
    for (int i = 0; i < 10; i++)                                  //Checks current reading 10 times every second
    {
      currentReading = sensor.getCurrentDC();                 //located in ACS712.cpp
      delay(100);
    }

    if (currentReading <= cutOffCurrentLimit)                 //Checks current reading against cutoff current limit
    {
      digitalWrite(chargingRelay, LOW);                       //If current reading is less than cutoff the circuit is disconnected
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.print("Battery Fully");                             //Outputs to the user the battery is fully charged
      lcd.setCursor(4, 1);
      lcd.print("Charged");
    }
  }

  Serial.println(F("Getting current reading..."));
  //currentReading = sensor.getCurrentDC();                     //Sets current reading equal to get current DC value

  if (currentReading >= peakCurrentLimit)                     //Checks current reading against peak current limit
  {
    digitalWrite(chargingRelay, LOW);                         //If current is equal to or more than peak the circuit is disconnected
    currentCalibration();                                     //The current is calibrated
    digitalWrite(chargingRelay, HIGH);                        //The circuit is reconnected
    delay(3000);
    currentReading = sensor.getCurrentDC();                   //The current reading is set again

    if (currentReading >= peakCurrentLimit)                   //The new current reading is checked againts the peak
    {
      Serial.println(F("Shutting down circuit..."));
      Serial.println(F(""));
      digitalWrite(chargingRelay, LOW);                       //The circuit is disconnected
      lcd.clear();
      lcd.setCursor(2, 0);
      lcd.print("Overcharging");                              //It outputs to the user that overcharging current it detected
      lcd.setCursor(0, 1);
      lcd.print("Current Detected");
      delay(2000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Charging Halted");                           //Tells the user to hit resit on the arduino
      delay(2000);
    }

  }
}

/*=================================================================================================================================================*/
/*Discharges the battery until it hits 25%*/

void DischargeBattery(float StateOfCharge)
{
  Serial.println(F("Taking charging reading...."));

  printChargePercentage(StateOfCharge);

  Serial.println(F("Switching charging relay off..."));
  Serial.println(F(""));
  digitalWrite(chargingRelay, LOW);           //Disconnects the charging circuit for safety reasons

  float chargeState = Check_Battery_SOC();    //Get voltage reading
  Serial.print(F("Current state of charge: "));
  Serial.println(chargeState);
  Serial.println(F(""));

  if (chargeState <= 2.75)                    //Miniumum battery voltage
  {
    Serial.println(F("Charge level is under 25 percent"));
    Serial.println(F(""));
    Serial.println(F("Switching discharge relay off"));
    Serial.println(F(""));
    digitalWrite(dischargingRelay, LOW);      //Disconnects discharging circuit
    chargeState = Check_Battery_SOC();
    if (chargeState <= 3.375)                  //Ensures battery stays disconnected once miniumum battery voltage is detected to account for voltage spikes
    {
      Serial.println(F("Voltage spike... "));
      Serial.println(F(""));
      Serial.println(F("Switching off discharge relay..."));
      Serial.println(F(""));
      digitalWrite(dischargingRelay, LOW);    // Switches off the discharging circuit
    }
  }

  else
  {
    Serial.println(F("Switching discharge relay on...."));
    Serial.println(F(""));
    digitalWrite(dischargingRelay, HIGH);     //Switches on the discharing circuit
  }
}

/*=================================================================================================================================================*/
/*Checks battery to see if it meets the load demand*/

bool Check_Battery_Against_Load(float stateOfCharge, float loadDemand)
{
    Serial.println(F(""));
    Serial.println(F("Checking if battery meets the load demand...."));

    float batteryPower = (stateOfCharge - 2.75) * 2;
    Serial.print(F("Battery power calculated: "));
    Serial.println(batteryPower);
    Serial.print(F("Load demand calculated: "));                                //For the 3.7 volt battery. Multiplies voltage by capacity to get power.
    Serial.println(loadDemand);

    float batteryPowerVersusLoad = batteryPower - loadDemand;                             //Calculates if battery meets the load demand
    Serial.print(F("Battery power after subtracting load demand: "));
    Serial.println(batteryPowerVersusLoad);

    if (batteryPowerVersusLoad >= 0)                                                      //Determines if battery meets load demand
    {
      return true;
      Serial.println(F("Battery meets the demand."));
    }
    else
    {
      return false;
      Serial.println(F("Battery doesn't meet the demand"));
    }


}

/*======================================================================================================================================*/
/*currentCalibration function*/
/*Outputs to the user that the sensor is being calibrated, sets current reading to analog input from the get DC current function in ACS712.cpp*/

void currentCalibration()
{
  lcd.clear();                                                    //Clears screen
  lcd.setCursor(0, 0);                                            //Sets where the statement is printed
  lcd.print("Auto calibrating");                                  //Outputs the sensor is calibrating
  lcd.setCursor(0, 1);                                            //Sets where the statement is printed
  lcd.print("Current Sensor...");
  sensor.calibrate();                                             //Takes the current reading 10 times and find the mean of the measurements
  delay(1000);                                                    //Delays 1 second

  currentReading = sensor.getCurrentDC();                         //Calls calibrate and applies current sensor characteristcs
  if (currentReading >= 0.02 || currentReading <= -0.02)          //For 3.7V battery, CHANGE WHEN WE SWITCH TO 12 VOLT
  {
    sensor.calibrate();                                           //Takes the current reading 10 times and find the mean of the measurements, located in ACS712.cpp
    delay(5000);                                                  //Delays 5 seconds
    currentReading = sensor.getCurrentDC();                       //Calls calibrate and applies current sensor characterisitcs
    if (currentReading >= 0.02)                                   //For 3.7V battery, CHANGE WHEN SWITCH TO 12V
    {
      currentCalibration();                                       //Keeps calibrating until it returns expected value
    }
  }
}

/*======================================================================================================================================*/
/*constantCurrentConstantVoltage function*/
/*takes current reading to make sure battery is connected correctly, then sets constantVoltageCurrent*/
/*
void constantCurrentConstantVoltage()
{
  lcd.clear();                                                        //Clears LCD screen
  lcd.setCursor(0, 0);                                                //Sets where statement is printed
  lcd.print("Analyzing CC/CV");                                       //Outputs to user what is happening
  lcd.setCursor(4, 1);                                                //Sets where statement is printed
  lcd.print("Modes...");
  digitalWrite(chargingRelay, HIGH);                                  //Closes the relay to activate the circuit
  for (int i = 0; i < 20; i++)
  {
    currentReading = sensor.getCurrentDC();                           //Gets current reading 20 times in 2 seconds, getCurrent is located in ACS712.cpp
    delay(100);
  }
  if (currentReading <= -0.1)                                         //Checks if current is negative
  {
    while (true)                                                      //While current is negative this loop will run
    {
      digitalWrite(chargingRelay, LOW);                               //Disconnects circuit
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Reverse Current");                                   //Outputs to the user that negative current is deteced
      lcd.setCursor(0, 1);
      lcd.print("detected");
      delay(2000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Flip Current");                                      //Tells the user to reverse the polarity on the sensor
      lcd.setCursor(0, 1);
      lcd.print("Sensor Polarity");
      delay(2000);
    }
  }
  constantVoltageCurrent = (currentReading * 0.8);                      //If current is the correct reading the constant voltage current is calculated
}
*/
/*=================================================================================================================================================*/
/*Send data to NodeMCU for RaspberryPi*/
/*Transmit Power Slots, Solar watts, Grid Pricing, Battery Percentage*/

void TransmitRaspberryToNode(int powerSlotOne, int powerSlotTwo, float solarIrradiance,float gridPrice, float StateOfCharge){

  //Send power slot #1 data to NodeMCU
  Serial.print("PowerSlot 1 data: ");
  String slot1_string_send = String(powerSlotOne, 10);
  Serial.print(slot1_string_send);
  Serial1.print(slot1_string_send);
  Serial.println(F(""));
  Serial.println(F("Power Slot #1 Sent..."));
  Serial.println(F(""));
  delay(2000);


  //Send power slot #2 data to NodeMCU
  Serial.print("PowerSlot 2 data: ");
  String slot2_string_send = String(powerSlotTwo, 10);
  Serial.print(slot2_string_send);
  Serial1.print(slot2_string_send);
  Serial.println(F(""));
  Serial.println(F("Power Slot #2 Sent..."));
  Serial.println(F(""));
  delay(2000);


  //Send solar irradiance data to NodeMCU
  Serial.print("Solar Irradiance data: ");
  Serial.print(solarIrradiance);
  sob.solVal = solarIrradiance;
  Serial1.write( '>' );
  Serial1.write( sob.solBytes, sizeof( float ) );
  Serial1.write( '<' );

  Serial.println(F(""));
  Serial.println(F("Solar Irradiance Sent..."));
  Serial.println(F(""));
  delay(2000);

  //Send grid price data to NodeMCU
  Serial.print("Grid price data: ");
  Serial.print(gridPrice);
  gob.gridVal = gridPrice;
  Serial1.write( '>' );
  Serial1.write( gob.gridBytes, sizeof( float ) );
  Serial1.write( '<' );


  Serial.println(F(""));
  Serial.println(F("Grid Price Sent..."));
  Serial.println(F(""));
  delay(2000);


  //Send battery charge data to NodeMCU
  Serial.print("Battery Charge data: ");
  Serial.print(StateOfCharge);
  bob.battVal = StateOfCharge;
  Serial1.write( '>' );
  Serial1.write( bob.battBytes, sizeof( float ) );
  Serial1.write( '<' );


  Serial.println(F(""));
  Serial.println(F("Battery Charge Sent..."));
  Serial.println(F(""));
  delay(1000);

}

/*=================================================================================================================================================*/
/*Weather JSON*/

wJson DeserializeWeather(const char* weatherjson) {

//char weatherjson[] = "{\"clouds\":1,\"temp\":312.35,\"Alert\":\"null\",\"OpenAlertStart\":\"null\"}";

StaticJsonDocument<128> weatherDoc;
//DynamicJsonDocument weatherDoc(128);
deserializeJson(weatherDoc, weatherjson);

//Jweather.cloudstruct = weatherDoc["clouds"];
//Jweather.tempstruct = weatherDoc["temp"];
//Jweather.alertstruct = weatherDoc["Alert"];
//Jweather.alertStartstruct = weatherDoc["OpenAlertStart"];

Jweather.cloudstruct = weatherDoc["clouds"];
Jweather.tempstruct = weatherDoc["temp"];

if(weatherDoc["Alert"] == "null" || weatherDoc["Alert"] == NULL || weatherDoc["Alert"] == "NULL"){
  Jweather.alertstruct = 0;
}
else{
  Jweather.alertstruct = 1;
}

Jweather.alertStartstruct = weatherDoc["OpenAlertStart"];

weatherDoc.clear();

return Jweather;
}

/*=================================================================================================================================================*/
/*RTC Initializer*/

void RTCInitializer(){

  if (!rtc.begin())
  {
    Serial.println(F("Couldn't find RTC Module"));
    while (1);
  }
  if (rtc.lostPower())
  {
    Serial.println(F("RTC lost power, lets set the time!"));
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

}
