#define RESET_PIN D2
#include <MHZ.h>

#include "Adafruit_SHT4x.h"
#include "Adafruit_SGP40.h"

#include "ThingSpeak.h"

#define CO2_INT 18
//#define SDS10_INT D7


int diff_10, diff_25;
unsigned long previous_loop, previous_10, previous_25, prev_time;

int SDS_25 = 0;
int SDS_10 = 0;


// I2C for BME280 sensor


//ICACHE_RAM_ATTR void read_sds10()
//{
//
//  diff_10 = millis() - previous_10;
//  previous_10 = millis();
//  //if(diff_10<900)
//  SDS_10 = diff_10;
//  //digitalWrite(25, HIGH);
//
//}

ICACHE_RAM_ATTR void read_co2()
{
  diff_25 = millis() - previous_25;
  previous_25 = millis();
  // if(diff_25<900)
  SDS_25 = diff_25;
}

#include <WiFi.h>

Adafruit_SHT4x sht4 = Adafruit_SHT4x();
Adafruit_SGP40 sgp;

#define SECRET_SSID "Red"    // replace MySSID with your WiFi network name
#define SECRET_PASS "88888888"  // replace MyPassword with your WiFi password
/////////////
////Device 1
//#define SECRET_CH_ID 1591608     // replace 0000000 with your channel number
//#define SECRET_WRITE_APIKEY "MGYZNCH5TAPELMYO"    // replace XYZ with your channel write API Key
////////////
////Device 2
//#define SECRET_CH_ID 1591610     // replace 0000000 with your channel number
//#define SECRET_WRITE_APIKEY "YLZOXNHN3ZQOMU0S"    // replace XYZ with your channel write API Key
////////////
////Device 3
//#define SECRET_CH_ID 1591611     // replace 0000000 with your channel number
//#define SECRET_WRITE_APIKEY "Z3GHUG9HGXHVYKFY"    // replace XYZ with your channel write API Key
////////////
////Device 4
//#define SECRET_CH_ID 1591612     // replace 0000000 with your channel number
//#define SECRET_WRITE_APIKEY "PQP6TLA63WAOEFTG"    // replace XYZ with your channel write API Key
////////////

//Device 5
#define SECRET_CH_ID 1591614     // replace 0000000 with your channel number
#define SECRET_WRITE_APIKEY "54EBYGBCPJ2P4VLJ"    // replace XYZ with your channel write API Key
//////////


char ssid[] = SECRET_SSID;   // your network SSID (name)
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiClient  client;

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;


// pin for pwm reading
#define CO2_IN 18

// pin for uart reading
#define MH_Z19_RX 26  // D7
#define MH_Z19_TX 27  // D6
MHZ co2(MH_Z19_RX, MH_Z19_TX, CO2_IN, MHZ14A);
byte command_frame[9] = {0xAA, 0x02, 0x00, 0x00, 0x00, 0x00, 0x01, 0x67, 0xBB};
byte received_data[9];
//unsigned int prev_time = 0;
int sum = 0;



void setup() {
  // put your setup code here, to run once:
  //  pinMode(D2,OUTPUT);
  delay(10);
  //digitalWrite(RESET_PIN,HIGH);
  Serial.begin(9600);
  delay(10);
  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);  // Initialize ThingSpeak
  Serial.print("hiiii");
  send_command(0x01);
  attachInterrupt(digitalPinToInterrupt(CO2_INT), read_co2, CHANGE);

///voc////

while (!Serial)
    delay(10);     // will pause Zero, Leonardo, etc until serial console opens
  if (! sgp.begin()){
    Serial.println("SGP40 sensor not found :(");
    while (1);
  }  
  Serial.println("Adafruit SHT4x test");
  if (! sht4.begin()) {
    Serial.println("Couldn't find SHT4x");
    while (1) delay(1);
  }
  Serial.println("Found SHT4x sensor");
  Serial.print("Serial number 0x");
  Serial.println(sht4.readSerial(), HEX);
  Serial.print(sgp.serialnumber[0], HEX);
  Serial.print(sgp.serialnumber[1], HEX);
  Serial.println(sgp.serialnumber[2], HEX);

  // You can have 3 different precisions, higher precision takes longer
  sht4.setPrecision(SHT4X_HIGH_PRECISION);
  switch (sht4.getPrecision()) {
     case SHT4X_HIGH_PRECISION: 
       Serial.println("High precision");
       break;
     case SHT4X_MED_PRECISION: 
       Serial.println("Med precision");
       break;
     case SHT4X_LOW_PRECISION: 
       Serial.println("Low precision");
       break;
  }

  // You can have 6 different heater settings
  // higher heat and longer times uses more power
  // and reads will take longer too!
  sht4.setHeater(SHT4X_NO_HEATER);
  switch (sht4.getHeater()) {
     case SHT4X_NO_HEATER: 
       Serial.println("No heater");
       break;
     case SHT4X_HIGH_HEATER_1S: 
       Serial.println("High heat for 1 second");
       break;
     case SHT4X_HIGH_HEATER_100MS: 
       Serial.println("High heat for 0.1 second");
       break;
     case SHT4X_MED_HEATER_1S: 
       Serial.println("Medium heat for 1 second");
       break;
     case SHT4X_MED_HEATER_100MS: 
       Serial.println("Medium heat for 0.1 second");
       break;
     case SHT4X_LOW_HEATER_1S: 
       Serial.println("Low heat for 1 second");
       break;
     case SHT4X_LOW_HEATER_100MS: 
       Serial.println("Low heat for 0.1 second");
       break;
  }


}

void loop() {

  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);
    }
    Serial.println("\nConnected.");
  }

  // put your main code here, to run repeatedly:
  if (millis() - prev_time > 5000)
  {
    send_command(0x02);
    prev_time = millis();
  }
  if (Serial.available())
  {
    Serial.readBytes(received_data, 9);
    if (checksum())
    {
      Serial.println("This sequence was received:");
      //    digitalWrite(LED_BUILTIN,HIGH);
      for (int i = 0; i < 9; i++) {
        Serial.print("0x");
        Serial.println(received_data[i], HEX);
      }
      Serial.println("End of sequence");
      Serial.print("Calculated checksum: ");
      Serial.println(sum);
      calculate_pm();
    }
    else
      // digitalWrite(LED_BUILTIN,LOW);
      Serial.println(received_data[6]);
  }

}

void send_command(byte command)
{
  command_frame[1] = command;
  int sum = command_frame[0] + command_frame[1] + command_frame[2] + command_frame[3] + command_frame[4] + command_frame[5] + command_frame[8];
  int rem = sum % 256;
  command_frame[6] = (sum - rem) / 256;
  command_frame[7] = rem;
  //Serial.println("Sending the follwing command:");
  delay(10);
  Serial.write(command_frame, 9);

}

bool checksum()
{
  sum = received_data[0] + received_data[1] + received_data[2] + received_data[3] + received_data[4] + received_data[5] + received_data[8];
  if (sum == ((int(received_data[6]) * 256) + int(received_data[7])))
  {
    return true;
  }
  else
    return false;
}

void calculate_pm()
{
  int pm2 = int(received_data[4]) * 256 + int(received_data[5]);
  int pm10 = int(received_data[2]) * 256 + int(received_data[3]);
  Serial.println("");
  Serial.print("PM 2.5 is: "); Serial.println(pm2);
  Serial.print("PM 10 is: "); Serial.println(pm10);
//  Serial.print("\n----- Time from start: ");
//  Serial.print(millis() / 1000);
//  Serial.println(" s");
//  int ppm_pwm = co2.readCO2PWM();
//  Serial.print(", PPMpwm: ");
//  Serial.print(ppm_pwm);
//
//  Serial.println("\n------------------------------");
//  delay(5000);


  sensors_event_t humidity, temp;
  uint16_t sraw;
  int32_t voc_index;
  uint32_t timestamp = millis();
  sht4.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
  timestamp = millis() - timestamp;

  Serial.print("Temperature: "); Serial.print(temp.temperature); Serial.println(" degrees C");
  Serial.print("Humidity: "); Serial.print(humidity.relative_humidity); Serial.println("% rH");

  Serial.print("Read duration (ms): ");
  Serial.println(timestamp);
  float t = temp.temperature;
  float h = humidity.relative_humidity;
  sraw = sgp.measureRaw(temp.temperature,humidity.relative_humidity);
  Serial.print("Raw measurement: ");
  Serial.println(sraw);
  voc_index = sgp.measureVocIndex(temp.temperature,humidity.relative_humidity);
  Serial.print("Voc Index: ");
  Serial.println(voc_index);
  delay(1000);
  Serial.print("ccco2 is: "); 
  Serial.println(SDS_25);

  ThingSpeak.setField(1, pm2);
  ThingSpeak.setField(2, pm10);
  ThingSpeak.setField(3, t);
  ThingSpeak.setField(4, h);
  ThingSpeak.setField(5, sraw); 
  ThingSpeak.setField(6, SDS_25);
  ThingSpeak.setField(7,voc_index);

   //ThingSpeak.setStatus(myStatus);
  
  // write to the ThingSpeak channel
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(x == 200){
    Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
  delay(1000);


}
