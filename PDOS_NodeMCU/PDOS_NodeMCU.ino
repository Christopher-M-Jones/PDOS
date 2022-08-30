#include <stdio.h>
#include <string.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>

//Initialize NodeMcu Rx and Tx pins for transfer to ArduinoMega (Rx = D5, Tx = D6)
//SoftwareSerial softser1;
SoftwareSerial softser1(D5, D6);//Rx,Tx

//Network Settings SSID-PASSWORD
char ssid[] = ""; //SSID redacted
char password[] = ""; // network password redacted

//Create WiFiClient object
WiFiClientSecure client;

//Host Servers
#define OPEN_WEATHER_HOST "api.openweathermap.org"
#define OPEN_EI_HOST "api.openei.org"
#define THINGSPEAK_HOST "api.thingspeak.com"

//GET Requests

String OPEN_WEATHER_API_KEY = ""; //API Key redacted
String OPEN_EI_API_KEY = "";      //API Key redacted

String OPEN_WEATHER_REQUEST = "/data/2.5/onecall?lat=33.45&lon=-88.78&exclude=minutely,hourly,daily&appid=" + OPEN_WEATHER_API_KEY;
String OPEN_EI_REQUEST = "/utility_rates?version=latest&format=json_plain&detail=full&getpage=5d03d6ac5457a3ad2d557796&api_key=" + OPEN_EI_API_KEY;

//ThingSpeak Requests
String ThingSpeakDeleteAPIKey = ""; //API Key redacted
String ThingSpeakUpdateAPIKey = ""; //API Key redacted

String ThingSpeakRequestWeather = "/update.json?api_key=" + ThingSpeakUpdateAPIKey + "&field1=";
String ThingSpeakDeletejson = "/channels/1564946/feeds.json?api_key=" + ThingSpeakDeleteAPIKey;

int jsonString = 0;

union Solarbyte
{
  uint8_t     solBytes[sizeof( float )];
  float       solVal;
};

Solarbyte sob;

union gridbyte
{
  uint8_t     gridBytes[sizeof( float )];
  float       gridVal;
};

gridbyte gob;

union battbyte
{
  uint8_t     battBytes[sizeof( float )];
  float       battVal;
};

battbyte bob;

float solarIrradiance = 0;
float gridPrice = 0;
float chargeState = 0;

int powerslotone = 0;
int powerslottwo = 0;

void setup() {

  Serial.begin(9600);
  //softser1.begin(9600, D5, D6);
  softser1.begin(9600);


  //Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  Serial.print(F("Connecting to Wifi: "));
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  //WiFi is connecting
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(500);
  }

  Serial.println(F(""));
  Serial.println(F("WiFi connected"));
  Serial.println(F("IP address: "));
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);
}



void makeOpenWeatherHTTPRequest(){

    Serial.println(F(""));
    Serial.println(F(""));
    Serial.println(F(""));
    Serial.println(F("Sending GET Request to OpenWeather Server..."));

    // If you don't need to check the fingerprint
    client.setInsecure();

    if (!client.connect(OPEN_WEATHER_HOST, 443)){
      Serial.println(F("Connection failed"));
      return;
    }

    yield();

    //Send HTTP Request
    client.print(F("GET "));
    client.print(OPEN_WEATHER_REQUEST);
    client.println(F(" HTTP/1.1"));

    //Headers
    client.print(F("Host: "));
    client.println(OPEN_WEATHER_HOST);
    client.println ("User-Agent: arduino-API-test");
    client.println(F("Cache-Control: no-cache"));


    if (client.println() == 0){
      Serial.println(F("Failed to send request"));
      return;
    }

    //delay(100);
    //Read the request until return character
    char status[32] = {0};
    client.readBytesUntil('\r', status, sizeof(status));

    //200 OK means request was read without issue
    if (strcmp(status, "HTTP/1.1 200 OK") !=0){
      Serial.print(F("Unexpected response: "));
      Serial.println(status);
      return;
    }


    // Skip HTTP headers
    char endOfHeaders[] = "\r\n\r\n";
    if (!client.find(endOfHeaders)){
      Serial.println(F("Invalid Response"));
      return;
    }

}

void makeOEIHTTPRequest(){

    Serial.println(F(""));
    Serial.println(F(""));
    Serial.println(F(""));
    Serial.println("Sending GET Request to OpenEI Server...");

    // If you don't need to check the fingerprint
    client.setInsecure();

    if (!client.connect(OPEN_EI_HOST, 443)){
      Serial.println(F("Connection failed"));
      return;
    }

    yield();

    //Send HTTP Request
    client.print(F("GET "));
    client.print(OPEN_EI_REQUEST);
    client.println(F(" HTTP/1.1"));

    //Headers
    client.print(F("Host: "));
    client.println(OPEN_EI_HOST);
    client.println ("User-Agent: arduino-API-test");
    client.println(F("Cache-Control: no-cache"));


    if (client.println() == 0){
      Serial.println(F("Failed to send request"));
      return;
    }

    //delay(100);
    //Read the request until return character
    char status[32] = {0};
    client.readBytesUntil('\r', status, sizeof(status));

    //200 OK means request was read without issue
    if (strcmp(status, "HTTP/1.1 200 OK") !=0){
      Serial.print(F("Unexpected response: "));
      Serial.println(status);
      return;
    }


    // Skip HTTP headers
    char endOfHeaders[] = "\r\n\r\n";
    if (!client.find(endOfHeaders)){
      Serial.println(F("Invalid Response"));
      return;
    }

}



const char* createJsonString(int jsonString){

    //Create string for Json
    String openWeatherJsonString = "";
    String openEIJsonString = "";


    Serial.println(F(""));
    Serial.println(F("JSON RETURNED: "));

    while (client.available() && client.peek() != '{' && client.peek() != '[')
    {
      char c = 0;
      client.readBytes(&c, 1);
      Serial.print(c);
      Serial.println("BAD");
    }

    if (jsonString == 0){
      while(client.available()){
      char c = 0;
      client.readBytes(&c, 1);
      openWeatherJsonString = openWeatherJsonString + c;
      Serial.print(c);
    }

       char* cstr = new char [openWeatherJsonString.length()+1];
       strcpy(cstr, openWeatherJsonString.c_str());
       return cstr;
    }

    else {
      while(client.available()){
      char c = 0;
      client.readBytes(&c, 1);
      openEIJsonString = openEIJsonString + c;
      Serial.print(c);
    }
      char* cstr = new char [openEIJsonString.length()+1];
      strcpy(cstr, openEIJsonString.c_str());
      return cstr;
    }
}




const char* ParseSerializeOpenWeather(const char* cstr){

  DynamicJsonDocument openWeatherDoc(768);

  deserializeJson(openWeatherDoc, cstr);

  float openTemperature = openWeatherDoc["current"]["temp"];
  int openClouds = openWeatherDoc["current"]["clouds"];
  String openAlert = openWeatherDoc["alerts"];
  String openAlertStart = openWeatherDoc["alerts"]["start"];

  Serial.println(F(""));
  Serial.println(F(""));
  Serial.print(F("RESERIALIZED JSON: "));
  Serial.println(F(""));


  DynamicJsonDocument openWeatherTransmit(128);

  openWeatherTransmit["clouds"]   = openClouds;
  openWeatherTransmit["temp"] = openTemperature;
  openWeatherTransmit["Alert"] = openAlert;
  openWeatherTransmit["OpenAlertStart"] = openAlertStart;

  serializeJson(openWeatherTransmit, Serial);

  String jsonWeatherOut = "" + openWeatherTransmit.as<String>();

  char* openWeatherTransmitString = new char [jsonWeatherOut.length()+1];
  strcpy(openWeatherTransmitString, jsonWeatherOut.c_str());

  return openWeatherTransmitString;
}


const char* ParseSerializeOpenEI(const char* cstr){

  DynamicJsonDocument openEIDoc(2048);

  deserializeJson(openEIDoc, cstr);

  /*****JULY WEEKEND DATA*****/
/*
  int julwkndgridprice[24];

  julwkndgridprice[0] = openEIDoc["items"][0]["energyweekendschedule"][6][0];
  julwkndgridprice[1] = openEIDoc["items"][0]["energyweekendschedule"][6][1];
  julwkndgridprice[2] = openEIDoc["items"][0]["energyweekendschedule"][6][2];
  julwkndgridprice[3] = openEIDoc["items"][0]["energyweekendschedule"][6][3];
  julwkndgridprice[4] = openEIDoc["items"][0]["energyweekendschedule"][6][4];
  julwkndgridprice[5] = openEIDoc["items"][0]["energyweekendschedule"][6][5];
  julwkndgridprice[6] = openEIDoc["items"][0]["energyweekendschedule"][6][6];
  julwkndgridprice[7] = openEIDoc["items"][0]["energyweekendschedule"][6][7];
  julwkndgridprice[8] = openEIDoc["items"][0]["energyweekendschedule"][6][8];
  julwkndgridprice[9] = openEIDoc["items"][0]["energyweekendschedule"][6][9];
  julwkndgridprice[10] = openEIDoc["items"][0]["energyweekendschedule"][6][10];
  julwkndgridprice[11] = openEIDoc["items"][0]["energyweekendschedule"][6][11];
  julwkndgridprice[12] = openEIDoc["items"][0]["energyweekendschedule"][6][12];
  julwkndgridprice[13] = openEIDoc["items"][0]["energyweekendschedule"][6][13];
  julwkndgridprice[14] = openEIDoc["items"][0]["energyweekendschedule"][6][14];
  julwkndgridprice[15] = openEIDoc["items"][0]["energyweekendschedule"][6][15];
  julwkndgridprice[16] = openEIDoc["items"][0]["energyweekendschedule"][6][16];
  julwkndgridprice[17] = openEIDoc["items"][0]["energyweekendschedule"][6][17];
  julwkndgridprice[18] = openEIDoc["items"][0]["energyweekendschedule"][6][18];
  julwkndgridprice[19] = openEIDoc["items"][0]["energyweekendschedule"][6][19];
  julwkndgridprice[20] = openEIDoc["items"][0]["energyweekendschedule"][6][20];
  julwkndgridprice[21] = openEIDoc["items"][0]["energyweekendschedule"][6][21];
  julwkndgridprice[22] = openEIDoc["items"][0]["energyweekendschedule"][6][22];
  julwkndgridprice[23] = openEIDoc["items"][0]["energyweekendschedule"][6][23];
*/

/*JSON DATA SENT FROM SERVER IS CORRUPTED AFTER THIS POINT.
 *THE JSON DOES NOT VALIDATE ON ARDUINOJSON.ORG BECAUSE OF EXTRA
 *CHARACTERS (COMMAS, BRACKETS) THAT ARE OUT OF PLACE. THE REMAINING
 *DATA IS HARD CODED BASED ON THE STATIC API DATA PULLED DOWN.
 *DATA IS CORRECT - BUT IS NOT DYNAMICALLY PARSED THROUGH THE JSON LIBRARY.
 */


  /******DECEMBER WEEKEND DATA*****/

  /*
  int decweekend12am = openEIDoc["items"][0]["energyweekendschedule"][7][0];
  int decweekend1am = openEIDoc["items"][0]["energyweekendschedule"][7][1];
  int decweekend2am = openEIDoc["items"][0]["energyweekendschedule"][7][2];
  int decweekend3am = openEIDoc["items"][0]["energyweekendschedule"][7][3];
  int decweekend4am = openEIDoc["items"][0]["energyweekendschedule"][7][4];
  int decweekend5am = openEIDoc["items"][0]["energyweekendschedule"][7][5];
  int decweekend6am = openEIDoc["items"][0]["energyweekendschedule"][7][6];
  int decweekend7am = openEIDoc["items"][0]["energyweekendschedule"][7][7];
  int decweekend8am = openEIDoc["items"][0]["energyweekendschedule"][7][8];
  int decweekend9am = openEIDoc["items"][0]["energyweekendschedule"][7][9];
  int decweekend10am = openEIDoc["items"][0]["energyweekendschedule"][7][10];
  int decweekend11am = openEIDoc["items"][0]["energyweekendschedule"][7][11];
  int decweekend12pm = openEIDoc["items"][0]["energyweekendschedule"][7][12];
  int decweekend1pm = openEIDoc["items"][0]["energyweekendschedule"][7][13];
  int decweekend2pm = openEIDoc["items"][0]["energyweekendschedule"][7][14];
  int decweekend3pm = openEIDoc["items"][0]["energyweekendschedule"][7][15];
  int decweekend4pm = openEIDoc["items"][0]["energyweekendschedule"][7][16];
  int decweekend5pm = openEIDoc["items"][0]["energyweekendschedule"][8][17];
  int decweekend6pm = openEIDoc["items"][0]["energyweekendschedule"][9][18];
  int decweekend7pm = openEIDoc["items"][0]["energyweekendschedule"][10][19];
  int decweekend8pm = openEIDoc["items"][0]["energyweekendschedule"][10][20];
  int decweekend9pm = openEIDoc["items"][0]["energyweekendschedule"][10][21];
  int decweekend10pm = openEIDoc["items"][0]["energyweekendschedule"][10][22];
  int decweekend11pm = openEIDoc["items"][0]["energyweekendschedule"][10][23];
  */

  /*ACTUAL DECEMBER WEEKEND GRID PRICING*/
/*
  int decwkndgridprice[24];

  for (int i = 0; i <= 5; i++)
  {
    decwkndgridprice[i] = 0;
  }
  for (int i = 6; i <= 22; i++)
  {
    decwkndgridprice[i] = 1;
  }
  decwkndgridprice[23] = 0;
*/

  /*****JULY WEEKDAY DATA*****/

/*

  int julyweekday12am = openEIDoc["items"][0]["energyweekdayschedule"][6][0];
  int julyweekday1am = openEIDoc["items"][0]["energyweekdayschedule"][6][1];
  int julyweekday2am = openEIDoc["items"][0]["energyweekdayschedule"][6][2];
  int julyweekday3am = openEIDoc["items"][0]["energyweekdayschedule"][6][3];
  int julyweekday4am = openEIDoc["items"][0]["energyweekdayschedule"][6][4];
  int julyweekday5am = openEIDoc["items"][0]["energyweekdayschedule"][6][5];
  int julyweekday6am = openEIDoc["items"][0]["energyweekdayschedule"][6][6];
  int julyweekday7am = openEIDoc["items"][0]["energyweekdayschedule"][6][7];
  int julyweekday8am = openEIDoc["items"][0]["energyweekdayschedule"][6][8];
  int julyweekday9am = openEIDoc["items"][0]["energyweekdayschedule"][6][9];
  int julyweekday10am = openEIDoc["items"][0]["energyweekdayschedule"][6][10];
  int julyweekday11am = openEIDoc["items"][0]["energyweekdayschedule"][6][11];
  int julyweekday12pm = openEIDoc["items"][0]["energyweekdayschedule"][6][12];
  int julyweekday1pm = openEIDoc["items"][0]["energyweekdayschedule"][6][13];
  int julyweekday2pm = openEIDoc["items"][0]["energyweekdayschedule"][6][14];
  int julyweekday3pm = openEIDoc["items"][0]["energyweekdayschedule"][6][15];
  int julyweekday4pm = openEIDoc["items"][0]["energyweekdayschedule"][6][16];
  int julyweekday5pm = openEIDoc["items"][0]["energyweekdayschedule"][6][17];
  int julyweekday6pm = openEIDoc["items"][0]["energyweekdayschedule"][6][18];
  int julyweekday7pm = openEIDoc["items"][0]["energyweekdayschedule"][6][19];
  int julyweekday8pm = openEIDoc["items"][0]["energyweekdayschedule"][6][20];
  int julyweekday9pm = openEIDoc["items"][0]["energyweekdayschedule"][6][21];
  int julyweekday10pm = openEIDoc["items"][0]["energyweekdayschedule"][6][22];
  int julyweekday11pm = openEIDoc["items"][0]["energyweekdayschedule"][6][23];

*/

  /*ACTUAL JULY WEEKDAY GRID PRICING*/


  int julwkdgridprice[24];

  for (int i = 0; i <= 5; i++)
  {
    julwkdgridprice[i] = 0;
  }
  for (int i = 6; i <= 12; i++)
  {
    julwkdgridprice[i] = 1;
  }
  for (int i = 13; i <= 17; i++)
  {
    julwkdgridprice[i] = 2;
  }
  for (int i = 18; i <= 22; i++)
  {
    julwkdgridprice[i] = 1;
  }

  julwkdgridprice[23] = 0;



   /*****DEC WEEKDAY DATA*****/

   /*
  int decweekday12am = openEIDoc["items"][0]["energyweekdayschedule"][11][0];
  int decweekday1am = openEIDoc["items"][0]["energyweekdayschedule"][11][1];
  int decweekday2am = openEIDoc["items"][0]["energyweekdayschedule"][11][2];
  int decweekday3am = openEIDoc["items"][0]["energyweekdayschedule"][11][3];
  int decweekday4am = openEIDoc["items"][0]["energyweekdayschedule"][11][4];
  int decweekday5am = openEIDoc["items"][0]["energyweekdayschedule"][11][5];
  int decweekday6am = openEIDoc["items"][0]["energyweekdayschedule"][11][6];
  int decweekday7am = openEIDoc["items"][0]["energyweekdayschedule"][11][7];
  int decweekday8am = openEIDoc["items"][0]["energyweekdayschedule"][11][8];
  int decweekday9am = openEIDoc["items"][0]["energyweekdayschedule"][11][9];
  int decweekday10am = openEIDoc["items"][0]["energyweekdayschedule"][11][10];
  int decweekday11am = openEIDoc["items"][0]["energyweekdayschedule"][11][11];
  int decweekday12pm = openEIDoc["items"][0]["energyweekdayschedule"][11][12];
  int decweekday1pm = openEIDoc["items"][0]["energyweekdayschedule"][11][13];
  int decweekday2pm = openEIDoc["items"][0]["energyweekdayschedule"][11][14];
  int decweekday3pm = openEIDoc["items"][0]["energyweekdayschedule"][11][15];
  int decweekday4pm = openEIDoc["items"][0]["energyweekdayschedule"][11][16];
  int decweekday5pm = openEIDoc["items"][0]["energyweekdayschedule"][11][17];
  int decweekday6pm = openEIDoc["items"][0]["energyweekdayschedule"][11][18];
  int decweekday7pm = openEIDoc["items"][0]["energyweekdayschedule"][11][19];
  int decweekday8pm = openEIDoc["items"][0]["energyweekdayschedule"][11][20];
  int decweekday9pm = openEIDoc["items"][0]["energyweekdayschedule"][11][21];
  int decweekday10pm = openEIDoc["items"][0]["energyweekdayschedule"][11][22];
  int decweekday11pm = openEIDoc["items"][0]["energyweekdayschedule"][11][23];
*/


/*ACTUAL JULY WEEKDAY GRID PRICING*/

  int decwkdgridprice[24];

  for (int i = 0; i <= 4; i++)
  {
    decwkdgridprice[i] = 0;
  }
  decwkdgridprice[5] = 1;

  for (int i = 6; i <= 9; i++)
  {
    decwkdgridprice[i] = 2;
  }
  for (int i = 10; i <= 22; i++)
  {
    decwkdgridprice[i] = 1;
  }
  decwkdgridprice[23] = 0;



/*ENERGY RATE STRUCTURE*/

  float rateZero = 0.08651;
  float rateOne = 0.096;
  float rateTwo = 0.096;
  float rateThree = 0.07503;
  float rateFour = 0.09958;
  float rateFive = 0.20159;

  Serial.println(F(""));
  Serial.println(F(""));
  Serial.print(F("JSON RETURNED: "));
  Serial.println(F(""));

  DynamicJsonDocument openEITransmit(2048);

/*
  openEITransmit["julyWeekendData"][0] = julwkndgridprice[0];
  openEITransmit["julyWeekendData"][1] = julwkndgridprice[1];
  openEITransmit["julyWeekendData"][2] = julwkndgridprice[2];
  openEITransmit["julyWeekendData"][3] = julwkndgridprice[3];
  openEITransmit["julyWeekendData"][4] = julwkndgridprice[4];
  openEITransmit["julyWeekendData"][5] = julwkndgridprice[5];
  openEITransmit["julyWeekendData"][6] = julwkndgridprice[6];
  openEITransmit["julyWeekendData"][7] = julwkndgridprice[7];
  openEITransmit["julyWeekendData"][8] = julwkndgridprice[8];
  openEITransmit["julyWeekendData"][9] = julwkndgridprice[9];
  openEITransmit["julyWeekendData"][10] = julwkndgridprice[10];
  openEITransmit["julyWeekendData"][11] = julwkndgridprice[11];
  openEITransmit["julyWeekendData"][12] = julwkndgridprice[12];
  openEITransmit["julyWeekendData"][13] = julwkndgridprice[13];
  openEITransmit["julyWeekendData"][14] = julwkndgridprice[14];
  openEITransmit["julyWeekendData"][15] = julwkndgridprice[15];
  openEITransmit["julyWeekendData"][16] = julwkndgridprice[16];
  openEITransmit["julyWeekendData"][17] = julwkndgridprice[17];
  openEITransmit["julyWeekendData"][18] = julwkndgridprice[18];
  openEITransmit["julyWeekendData"][19] = julwkndgridprice[19];
  openEITransmit["julyWeekendData"][20] = julwkndgridprice[20];
  openEITransmit["julyWeekendData"][21] = julwkndgridprice[21];
  openEITransmit["julyWeekendData"][22] = julwkndgridprice[22];
  openEITransmit["julyWeekendData"][23] = julwkndgridprice[23];

  openEITransmit["decWeekendData"][0] = decwkndgridprice[0];
  openEITransmit["decWeekendData"][1] = decwkndgridprice[1];
  openEITransmit["decWeekendData"][2] = decwkndgridprice[2];
  openEITransmit["decWeekendData"][3] = decwkndgridprice[3];
  openEITransmit["decWeekendData"][4] = decwkndgridprice[4];
  openEITransmit["decWeekendData"][5] = decwkndgridprice[5];
  openEITransmit["decWeekendData"][6] = decwkndgridprice[6];
  openEITransmit["decWeekendData"][7] = decwkndgridprice[7];
  openEITransmit["decWeekendData"][8] = decwkndgridprice[8];
  openEITransmit["decWeekendData"][9] = decwkndgridprice[9];
  openEITransmit["decWeekendData"][10] = decwkndgridprice[10];
  openEITransmit["decWeekendData"][11] = decwkndgridprice[11];
  openEITransmit["decWeekendData"][12] = decwkndgridprice[12];
  openEITransmit["decWeekendData"][13] = decwkndgridprice[13];
  openEITransmit["decWeekendData"][14] = decwkndgridprice[14];
  openEITransmit["decWeekendData"][15] = decwkndgridprice[15];
  openEITransmit["decWeekendData"][16] = decwkndgridprice[16];
  openEITransmit["decWeekendData"][17] = decwkndgridprice[17];
  openEITransmit["decWeekendData"][18] = decwkndgridprice[18];
  openEITransmit["decWeekendData"][19] = decwkndgridprice[19];
  openEITransmit["decWeekendData"][20] = decwkndgridprice[20];
  openEITransmit["decWeekendData"][21] = decwkndgridprice[21];
  openEITransmit["decWeekendData"][22] = decwkndgridprice[22];
  openEITransmit["decWeekendData"][23] = decwkndgridprice[23];

*/

  openEITransmit["julyWeekdayData"][0] = julwkdgridprice[0];
  openEITransmit["julyWeekdayData"][1] = julwkdgridprice[1];
  openEITransmit["julyWeekdayData"][2] = julwkdgridprice[2];
  openEITransmit["julyWeekdayData"][3] = julwkdgridprice[3];
  openEITransmit["julyWeekdayData"][4] = julwkdgridprice[4];
  openEITransmit["julyWeekdayData"][5] = julwkdgridprice[5];
  openEITransmit["julyWeekdayData"][6] = julwkdgridprice[6];
  openEITransmit["julyWeekdayData"][7] = julwkdgridprice[7];
  openEITransmit["julyWeekdayData"][8] = julwkdgridprice[8];
  openEITransmit["julyWeekdayData"][9] = julwkdgridprice[9];
  openEITransmit["julyWeekdayData"][10] = julwkdgridprice[10];
  openEITransmit["julyWeekdayData"][11] = julwkdgridprice[11];
  openEITransmit["julyWeekdayData"][12] = julwkdgridprice[12];
  openEITransmit["julyWeekdayData"][13] = julwkdgridprice[13];
  openEITransmit["julyWeekdayData"][14] = julwkdgridprice[14];
  openEITransmit["julyWeekdayData"][15] = julwkdgridprice[15];
  openEITransmit["julyWeekdayData"][16] = julwkdgridprice[16];
  openEITransmit["julyWeekdayData"][17] = julwkdgridprice[17];
  openEITransmit["julyWeekdayData"][18] = julwkdgridprice[18];
  openEITransmit["julyWeekdayData"][19] = julwkdgridprice[19];
  openEITransmit["julyWeekdayData"][20] = julwkdgridprice[20];
  openEITransmit["julyWeekdayData"][21] = julwkdgridprice[21];
  openEITransmit["julyWeekdayData"][22] = julwkdgridprice[22];
  openEITransmit["julyWeekdayData"][23] = julwkdgridprice[23];

  openEITransmit["decWeekdayData"][0] = decwkdgridprice[0];
  openEITransmit["decWeekdayData"][1] = decwkdgridprice[1];
  openEITransmit["decWeekdayData"][2] = decwkdgridprice[2];
  openEITransmit["decWeekdayData"][3] = decwkdgridprice[3];
  openEITransmit["decWeekdayData"][4] = decwkdgridprice[4];
  openEITransmit["decWeekdayData"][5] = decwkdgridprice[5];
  openEITransmit["decWeekdayData"][6] = decwkdgridprice[6];
  openEITransmit["decWeekdayData"][7] = decwkdgridprice[7];
  openEITransmit["decWeekdayData"][8] = decwkdgridprice[8];
  openEITransmit["decWeekdayData"][9] = decwkdgridprice[9];
  openEITransmit["decWeekdayData"][10] = decwkdgridprice[10];
  openEITransmit["decWeekdayData"][11] = decwkdgridprice[11];
  openEITransmit["decWeekdayData"][12] = decwkdgridprice[12];
  openEITransmit["decWeekdayData"][13] = decwkdgridprice[13];
  openEITransmit["decWeekdayData"][14] = decwkdgridprice[14];
  openEITransmit["decWeekdayData"][15] = decwkdgridprice[15];
  openEITransmit["decWeekdayData"][16] = decwkdgridprice[16];
  openEITransmit["decWeekdayData"][17] = decwkdgridprice[17];
  openEITransmit["decWeekdayData"][18] = decwkdgridprice[18];
  openEITransmit["decWeekdayData"][19] = decwkdgridprice[19];
  openEITransmit["decWeekdayData"][20] = decwkdgridprice[20];
  openEITransmit["decWeekdayData"][21] = decwkdgridprice[21];
  openEITransmit["decWeekdayData"][22] = decwkdgridprice[22];
  openEITransmit["decWeekdayData"][23] = decwkdgridprice[23];

  openEITransmit["Cat 1 Pricing"] = rateZero;
  openEITransmit["Cat 2 Pricing"] = rateOne;
  openEITransmit["Cat 3 Pricing"] = rateTwo;
  openEITransmit["Cat 4 Pricing"] = rateThree;
  openEITransmit["Cat 5 Pricing"] = rateFour;
  openEITransmit["Cat 6 Pricing"] = rateFive;

  Serial.println("");
  serializeJson(openEITransmit, Serial);

  String jsonEIOut = "" + openEITransmit.as<String>();

  char* openEITransmitString = new char [jsonEIOut.length()+1];
  strcpy(openEITransmitString, jsonEIOut.c_str());

  return openEITransmitString;

}

void makeThingSpeakPOSTRequest(const char* apiString, const char* rpiString){

    String baseRequest = ThingSpeakRequestWeather;

    Serial.println(F(""));
    //Serial.println(F("Sending POST Request to Web Server..."));

    int apiLength = strlen(apiString);
    int rpiLength = strlen(rpiString);

    for (int i = 0; i < apiLength; i++){
      baseRequest += apiString[i];
    }

    baseRequest += "&field2=";

    for (int i = 0; i < rpiLength; i++){
      baseRequest += rpiString[i];
    }

    // If you don't need to check the fingerprint
    client.setInsecure();

    if (!client.connect(THINGSPEAK_HOST, 443)){
      Serial.println(F("Connection failed"));
      return;
    }

    yield();

    //Send HTTP Request
    client.print(F("GET "));
    client.print(baseRequest);
    client.println(F(" HTTP/1.1"));

    //Headers
    client.print(F("Host: "));
    client.println(THINGSPEAK_HOST);
    client.println ("User-Agent: arduino-API-test");
    client.println(F("Cache-Control: no-cache"));


    if (client.println() == 0){
      Serial.println(F("Failed to send request"));
      return;
    }

    //delay(100);
    //Read the request until return character
    char status[32] = {0};
    client.readBytesUntil('\r', status, sizeof(status));

    //200 OK means request was read without issue
    if (strcmp(status, "HTTP/1.1 200 OK") !=0){
      Serial.print(F("Unexpected response: "));
      Serial.println(status);
      return;
    }


    // Skip HTTP headers
    char endOfHeaders[] = "\r\n\r\n";
    if (!client.find(endOfHeaders)){
      Serial.println(F("Invalid Response"));
      return;
    }

    Serial.println("Weather data successfully sent to web server...");
    Serial.println(F(""));
}

void makeThingSpeakDeleteRequest(){

    String baseRequest = ThingSpeakDeletejson;

    Serial.println(F(""));
    Serial.println(F(""));
    Serial.println(F(""));
    Serial.println(F("Sending Delete Request to Web Server..."));

    // If you don't need to check the fingerprint
    client.setInsecure();

    if (!client.connect(THINGSPEAK_HOST, 443)){
      Serial.println(F("Connection failed"));
      return;
    }

    yield();

    //Send HTTP Request
    client.print(F("DELETE "));
    client.print(baseRequest);
    client.println(F(" HTTP/1.1"));

    //Headers
    client.print(F("Host: "));
    client.println(THINGSPEAK_HOST);
    client.println ("User-Agent: arduino-API-test");
    client.println(F("Cache-Control: no-cache"));


    if (client.println() == 0){
      Serial.println(F("Failed to send request"));
      return;
    }

    //delay(100);
    //Read the request until return character
    char status[32] = {0};
    client.readBytesUntil('\r', status, sizeof(status));

    //200 OK means request was read without issue
    if (strcmp(status, "HTTP/1.1 200 OK") !=0){
      Serial.print(F("Unexpected response: "));
      Serial.println(status);
      return;
    }


    // Skip HTTP headers
    char endOfHeaders[] = "\r\n\r\n";
    if (!client.find(endOfHeaders)){
      Serial.println(F("Invalid Response"));
      return;
    }

    Serial.println(F("Web Server Data Deleted..."));
}




void TransmitWeatherToArduino(const char* weatherjArray){

  //Find length of weatherArray
  int weatherALen = strlen(weatherjArray);

  softser1.print(weatherALen);

  delay(100);

  int quotient = 0;
  int lowerbound = 0;
  int upperbound = 0;
  int remainder = 0;

  while(weatherALen >= 0){

    weatherALen -= 50;

    if(weatherALen > 0){

      quotient++;

      upperbound = 50*quotient;

      if(quotient == 1){
        lowerbound = upperbound - 50;
      }
      else{
        lowerbound = (upperbound - 50) + 1;
      }

      for(int i = lowerbound; i <= upperbound; i++){
        softser1.print(weatherjArray[i]);
        //delay(5);
      }

    }

    else{

      remainder = weatherALen + 50;

      if(quotient == 0){

      upperbound = remainder;
      lowerbound = 0;

      }

      else {

      upperbound = (50 * quotient) + remainder;
      lowerbound = (50 * quotient) + 1;

      }

      for(int i = lowerbound; i <= upperbound; i++){
        softser1.print(weatherjArray[i]);
      }

    }

    delay(100);

  }


  delay(100);

}

void TransmitGridToArduino(const char* gridjArray){

  //Find length of gridArray
  int gridALen = strlen(gridjArray);

  softser1.print(gridALen);

  delay(1000);

  int quotient = 0;
  int lowerbound = 0;
  int upperbound = 0;
  int remainder = 0;

  while(gridALen >= 0){

    gridALen -= 50;

    if(gridALen > 0){

      quotient++;

      upperbound = 50*quotient;

      if(quotient == 1){
        lowerbound = upperbound - 50;
      }
      else{
        lowerbound = (upperbound - 50) + 1;
      }

      for(int i = lowerbound; i <= upperbound; i++){
        softser1.print(gridjArray[i]);
        //delay(5);
      }

    }

    else{

      remainder = gridALen + 50;

      if(quotient == 0){

      upperbound = remainder;
      lowerbound = 0;

      }

      else {

      upperbound = (50 * quotient) + remainder;
      lowerbound = (50 * quotient) + 1;

      }

      for(int i = lowerbound; i <= upperbound; i++){
        softser1.print(gridjArray[i]);
      }

    }

    delay(100);

  }
}


const char* ThingSpeakJsonSerializer(int powerslotone, int powerslottwo, float solarIrradiance, float gridPrice, float chargeState){

  DynamicJsonDocument rpiTransmit(256);

  rpiTransmit["Power Slot One"] = powerslotone;
  rpiTransmit["Power Slot Two"] = powerslottwo;
  rpiTransmit["Solar Irradiance"] = solarIrradiance;
  rpiTransmit["Grid Price"] = gridPrice;
  rpiTransmit["State of Charge"] = chargeState;

  //Serial.println("JSON to Raspberry Pi");
  Serial.println(F(""));
  serializeJson(rpiTransmit, Serial);

  String jsonrpiOut = "" + rpiTransmit.as<String>();

  char* rpiTransmitString = new char [jsonrpiOut.length()+1];
  strcpy(rpiTransmitString, jsonrpiOut.c_str());

  return rpiTransmitString;
}


float SolarIrradianceReceive()
{
    byte
        ch,
        idx;
    bool
        done;

    float solreturn = 0;

    Serial.println(F(""));
    Serial.println(F("Waiting on Arduino....Solar Irradiance"));

    while(!softser1.available()){
    //do nothing
    }

    if(softser1.available() > 0)
    {
        if(softser1.read() == '>')
        {
            done = false;
            idx = 0;
            while(!done)
            {
                if(softser1.available() > 0)
                {
                    ch = softser1.read();
                    if( ch == '<' )
                        done = true;
                    else
                    {
                        if( idx < sizeof( float ) )
                            sob.solBytes[idx++] = ch;

                    }//else

                }//if

            }//while
              
            solreturn = sob.solVal;
           
        }//if

    }//if

    return solreturn;

}//loop

float GridPriceReceive()
{
    byte
        ch,
        idx;
    bool
        done;

    float gridreturn = 0;

    Serial.println(F(""));
    Serial.println(F("Waiting on Arduino....Grid Price"));

    while(!softser1.available()){
    //do nothing
    }

    if(softser1.available() > 0)
    {
        if(softser1.read() == '>')
        {
            done = false;
            idx = 0;
            while(!done)
            {
                if(softser1.available() > 0)
                {
                    ch = softser1.read();
                    if( ch == '<' )
                        done = true;
                    else
                    {
                        if( idx < sizeof( float ) )
                            gob.gridBytes[idx++] = ch;

                    }//else

                }//if

            }//while

            gridreturn = gob.gridVal;

        }//if

    }//if

    return gridreturn;

}//loop


float BatteryChargeReceive()
{
    byte
        ch,
        idx;
    bool
        done;

    float batteryreturn = 0;

    Serial.println(F(""));
    Serial.println(F("Waiting on Arduino....Battery Charge"));

    while(!softser1.available()){
    //do nothing
    }

    if(softser1.available() > 0)
    {
        if(softser1.read() == '>')
        {
            done = false;
            idx = 0;
            while(!done)
            {
                if(softser1.available() > 0)
                {
                    ch = softser1.read();
                    if( ch == '<' )
                        done = true;
                    else
                    {
                        if( idx < sizeof( float ) )
                            bob.battBytes[idx++] = ch;

                    }//else

                }//if

            }//while
            
            batteryreturn = bob.battVal;

        }//if

    }//if

    return batteryreturn;

}//loop


int powerSlotOneReceive(){

  int powerslot1 = 0;
  String content = "";
  char character = '\0';

  Serial.println(F(""));
  Serial.println(F(""));
  Serial.println(F("Waiting on Arduino....power slot #1"));
  while(!softser1.available()){
    //do nothing
  }

  delay(100);

  while(softser1.available()){
    character = softser1.read();
    content.concat(character);
  }

  powerslot1 = content.toInt();
  return powerslot1;
}


int powerSlotTwoReceive(){

  int powerslot2 = 0;
  String content = "";
  char character = '\0';

  Serial.println(F(""));
  Serial.println(F("Waiting on Arduino....power slot #2"));
  while(!softser1.available()){
    //do nothing
  }

  while(softser1.available()){
    character = softser1.read();
    content.concat(character);
  }

  powerslot2 = content.toInt();
  return powerslot2;
}


void loop() {

  /*****DELETE DATA FROM THINGSPEAK SERVER*****/

  makeThingSpeakDeleteRequest();

  /**********OPENWEATHER GET AND PARSE*********/

  //Make OpenWeather GET Request
  makeOpenWeatherHTTPRequest();

  //Create JSON from OpenWeather Server data
  const char* cstr2 = createJsonString(jsonString);

  //Parse OpenWeather data and store in char array
  const char* openWeatherArduinoData = ParseSerializeOpenWeather(cstr2);


  /***********OPENEI GET AND PARSE************/

  //Make OpenEI GET Request
  makeOEIHTTPRequest();

  //Changes which JSON string is created (OpenWeather = 0, OEI = 1);
  jsonString = 1;

  //Create JSON from OpenEI Server data
  const char* cstr3 = createJsonString(jsonString);

  //Parse OpenEI data and store in char array
  const char* openEIArduinoData = ParseSerializeOpenEI(cstr3);

  /*************TRANSMIT JSON TO ARDUINO**************/

  Serial.println(F(""));
  Serial.println(F(""));
  Serial.println(F("Sending Weather and Grid Pricing Data to Arduino...."));

  //Begin Serial Communication To Arduino - Weather
  TransmitWeatherToArduino(openWeatherArduinoData);

  //Begin Serial Communication To Arduino - Grid Pricing
  TransmitGridToArduino(openEIArduinoData);

  /***************RECEIVE DATA FROM ARDUINO*************************/

  //Listen for Arduino Communication and store the values from the Arduino
  powerslotone = powerSlotOneReceive();
  Serial.print(F("Power Slot #1 value received: "));
  Serial.println(powerslotone);

  powerslottwo = powerSlotTwoReceive();
  Serial.print(F("Power Slot #2 value received: "));
  Serial.println(powerslottwo);

  solarIrradiance = SolarIrradianceReceive();
  Serial.print(F("Solar Output value received: "));
  Serial.println(solarIrradiance);

  gridPrice = GridPriceReceive();
  Serial.print(F("Grid Price value received: "));
  Serial.println(gridPrice);

  chargeState = BatteryChargeReceive();
  Serial.print(F("Battery voltage level received: "));
  Serial.println(chargeState);

  /***************CONVERT ARDUINO DATA TO JSON FOR Pi*************/

  //Serial.println("Serializing raspberry pi data....");
  const char* rpi_Arduino_String = ThingSpeakJsonSerializer(powerslotone, powerslottwo, solarIrradiance, gridPrice, chargeState);

  /*************OPENWEATHER POST THINGSPEAK**************/

  Serial.println(F(""));
  Serial.println(F(""));
  Serial.println(F("Sending Data to Web Server...."));

  //Make Post Request to ThingSpeak server
  makeThingSpeakPOSTRequest(openWeatherArduinoData, rpi_Arduino_String);

  /*************DELETE DATA IN THE HEAP**************/
  //delete string objects before looping
  delete cstr2;
  delete cstr3;
  delete openWeatherArduinoData;
  delete openEIArduinoData;
  delete rpi_Arduino_String;

  /*************END OF PROGRAM - LOOP**************/
  Serial.println(F("End of main loop."));

  //Loop every 35 seconds
  delay(35000);
}
