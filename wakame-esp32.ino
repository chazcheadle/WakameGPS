

/**
 * GPS Logger
 *
 * Configure the NMEAGPS_cfg.h last sentence for the Ublox 6m module.
 * #define LAST_SENTENCE_IN_INTERVAL NMEAGPS::NMEA_GLL
 *
 * CREDITS:
 * - https://github.com/markfickett/gpslogger
 * - https://github.com/SlashDevin/NeoGPS
 */

/***** Libraries *****/
// #include <ESP8266WiFi.h>
// #include <ESP8266mDNS.h>
// #include <ESP8266WebServer.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <DNSServer.h>
#include "FS.h"
#include "SD.h"
#include <ArduinoOTA.h>
#include <NMEAGPS.h>
#include <Streamers.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_HMC5883_U.h>

#define DEBUG_PORT Serial
#define GPS_PORT Serial1
#define GPSBaud 9600 // Initial baud rate
// Define GPS sample rates:
// 0 = walking (1hz)
// 5 = biking/driving
// 10 = flying
int GPS_RATE = 0;

// Good ole' pin 13!
#define LED 13
#define LED_ON      LOW
#define LED_OFF     HIGH

// Battery monitoring configuration
// #define VBATPIN A9
const float BATTERY_LOW_VOLTAGE = 3.21;
const float BATTERY_MEDIUM_VOLTAGE = 3.6;
const float BATTERY_HIGH_VOLTAGE = 4;
const float VOLTAGE_ANALOG_CONSTANT = .0064;
const float BATT_CHECK_INTERVAL = 5000; // 5 seconds
// Declare/initialize battery monitoring variables
bool BATTERY_LOW = false;
float last_vbat = 0;
float measured_vbat;

// SPI configuration for LCD and SD card
#define TFT_CS         21
#define TFT_RST        -1
#define TFT_DC         A1
#define SD_CS          A0
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);
// Custom color definitions
#define ST7735_DARKGREY 0xC618
#include <Fonts/FreeSans9pt7b.h>

/***** BME280 Sensor *****/
#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme; // I2C
int WEATHER_TEMP_UNITS = 0;


// Setup mag sensor and variables
Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);
bool magPresent = true;
float headingDegrees, prevHeadingDegrees;
const char *headingCardinal;
int last_heading;

/***** NMEA variables *****/
NMEAGPS gps;
gps_fix fix;
struct SAT_DATA {
  int id;
  float elevation;
  float azimuth;
  bool tracked;
};
struct SAT_DATA satData[20];

// Set GPS units (metric/imperial);
int GPS_DISTANCE_UNITS = 0;
int GPS_COORD_UNITS = 0;

// Satellite display settings
// Define Satellite map dimensions and location
#define SATX 63
#define SATY 67
#define SATR 50

int GPS_SAMPLE_RATE[3] = {1, 5, 10};
int currentGPSSampleRate = 0;
int NUMBER_GPS_SAMPLE_RATES = 3;


// Ublox configuration
unsigned char UBLOX_INIT[] PROGMEM = {
  //0xB5,0x62,0x06,0x08,0x06,0x00,0x64,0x00,0x01,0x00,0x01,0x00,0x7A,0x12, //(10Hz)
  0xB5,0x62,0x06,0x08,0x06,0x00,0xC8,0x00,0x01,0x00,0x01,0x00,0xDE,0x6A, //(5Hz)
  // 0xB5,0x62,0x06,0x08,0x06,0x00,0xE8,0x03,0x01,0x00,0x01,0x00,0x01,0x39 //(1Hz)
};

const unsigned char UBLOX_GPS_RATE_WALKING[] PROGMEM = {0xB5,0x62,0x06,0x08,0x06,0x00,0xE8,0x03,0x01,0x00,0x01,0x00,0x01,0x39}; //(1Hz)
const unsigned char UBLOX_GPS_RATE_DRIVING[] PROGMEM = {0xB5,0x62,0x06,0x08,0x06,0x00,0xC8,0x00,0x01,0x00,0x01,0x00,0xDE,0x6A}; //(5Hz)};
const unsigned char UBLOX_GPS_RATE_FLYING[] PROGMEM = {0xB5,0x62,0x06,0x08,0x06,0x00,0x64,0x00,0x01,0x00,0x01,0x00,0x7A,0x12}; //(10Hz)

/***** Interval counters *****/
unsigned long current_millis;
unsigned long last_millis = 0;

/***** BUTTONS *****/
const int buttonOne = 12;              // Uses HW Serial TX pin
int buttonOne_state;                  // the current reading from the input pin
int buttonOne_lastState = HIGH;           // the previous reading from the input pin
const int buttonTwo = 13;              // Uses HW Serial TX pin
int buttonTwo_state;                  // the current reading from the input pin
int buttonTwo_lastState = HIGH;           // the previous reading from the input pin
unsigned long lastDebounceTime = 0;   // the last time the output pin was toggled
unsigned long debounceDelay = 10;     // the debounce time; increase if the output flickers

/***** Configure GUI options *****/
int currentScreen = 0;
int lastScreen = -1;
const int NUMBER_OF_SCREENS = 5;
#define GPS_DATA_SCREEN 0
#define NUM_GPS_DATA_OPTIONS 6

#define WEATHER_SCREEN 1
#define NUM_WEATHER_SCREEN_OPTIONS 4

#define WIFI_INFO_SCREEN 2
#define NUM_WIFI_INFO_SCREEN_OPTIONS 4

#define GPS_SATMAP_SCREEN 3
#define NUM_GPS_SATMAP_OPTIONS 3

#define GPS_SATINFO_SCREEN 4
#define NUM_GPS_SATMAP_OPTIONS 2

bool configPage[NUMBER_OF_SCREENS] = {false, false, false, false, false};
int currentOption = 0;
int lastOption = 0;
bool menuCMD = false;
bool menuWeatherConfig[4] = {false, false, false, false};

// GPX file format configurations
// Seek to fileSize + this position before writing track points.
#define SEEK_TRKPT_BACKWARDS -27
#define GPX_EPILOGUE "\t\t</trkseg>\n\t</trk>\n</gpx>\n"


bool logging = false;
// SdFat sd;
// SdFile logfile;

// #define ESP_getChipId() (ESP.getEfuseMac())

// SSID and PW for Config Portal
String ssid = "ESP32_GPS";
const char* password = "password";

// SSID and PW for your Router
String Router_SSID;
String Router_Pass;

#include <ESP_WiFiManager.h>

char hostString[16] = {0};
char hostIP[16];
ESP_WiFiManager ESP_wifiManager;
// General-purpose text buffer used in formatting.
char buf[32];



// static void programGPS(unsigned char RATE) {
//   for(unsigned int i = 0; i < sizeof(RATE); i++) {                       
//     GPS_PORT.write( pgm_read_byte(RATE+i) );
//   }
// }

void setup() {

  DEBUG_PORT.begin(115200);
  // Configure GPS
  GPS_PORT.begin(GPSBaud);
DEBUG_PORT.println(F("Init."));

  // GPS_PORT.print( F("$PUBX,41,1,0007,0003,115200,0*25\r\n") ); // Increase baud rate
  // programGPS(UBLOX_INIT);
  // for(unsigned int i = 0; i < sizeof(UBLOX_INIT); i++) {                       
  //   GPS_PORT.write( pgm_read_byte(UBLOX_INIT+i) );
  //  }

  pinMode(LED, OUTPUT);
  pinMode(buttonOne, INPUT_PULLUP);
  pinMode(buttonTwo, INPUT_PULLUP);

  // Initialize TFT display
  tft.initR(INITR_144GREENTAB);
  // Blank screen (to black)
  tft.fillScreen(ST7735_BLACK);
  tft.setRotation(3);
  tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
  // Draw title bar
  tft.setTextWrap(false);
  tft.fillRect(0, 0, 128, 11, ST7735_WHITE);
  tft.setCursor(1, 1);
  // tft.setFont(&FreeSans9pt7b);


  // Start mag sensoring
  if (!mag.begin()) {
    magPresent = false;
    DEBUG_PORT.println(F("No magnetometer"));
    tft.setCursor(1, 119);
    tft.print("No magnetometer");
  }
  // else {
  //   sensors_event_t event;
  //   mag.getEvent(&event);

  //   float heading = atan2(event.magnetic.y, event.magnetic.x);
  //   float declinationAngle = 0.22;
  //   heading += declinationAngle;
  //   tft.setCursor(1, 119);
  //   // Correct for when signs are reversed.
  //   if(heading < 0)
  //     heading += 2*PI;

  //   // Check for wrap due to addition of declination.
  //   if(heading > 2*PI)
  //     heading -= 2*PI;

  //   // Convert radians to degrees for readability.
  //   headingDegrees = heading * 180/M_PI;
  //   tft.print(headingDegrees);

  // }

  // Initialize BME280 sensor
  unsigned bmeStatus;
  tft.setCursor(1, 119);
  bmeStatus = bme.begin(0x76);
  if (!bmeStatus) {
    DEBUG_PORT.println(F("BME Fail"));
    tft.print(F("BME Fail"));
  }

  // if (digitalRead(buttonOne) == LOW && digitalRead(buttonTwo) == LOW) {
  //   Router_SSID = ESP_wifiManager.WiFi_SSID();
  //   Router_Pass = ESP_wifiManager.WiFi_Pass();
  //   tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
  //   tft.setCursor(1, 120);
  //   tft.print("Starting WiFi...");
  //   digitalWrite(LED, LOW);
  //   delay(2000);
  //   WiFi.mode(WIFI_STA);
  //   WiFi.begin(Router_SSID.c_str(), Router_Pass.c_str());
  //   delay(5000);
  //   if (WiFi.waitForConnectResult() != WL_CONNECTED) {  
  //     // DEBUG_PORT.println("Connection Failed! Rebooting...");
  //     tft.setCursor(1, 120);
  //     tft.print("Cannot connect to WiFi");
  //     tft.setCursor(114, 120);
  //     tft.print(" ");
  //     // ESP.restart();
  //   }
  //   else {
  //     tft.fillRect(0, 120, 127, 7, ST7735_BLACK);
  //     tft.setCursor(114, 2);
  //     tft.setTextColor(ST7735_BLACK, ST7735_WHITE);
  //     tft.print("W");
  //     tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
  //   }
  //   // Set mDNS hostname
  //   sprintf(hostString, "ESP-%06X", ESP.getChipId());
  //   if (!MDNS.begin(hostString)) {
  //     tft.setCursor(1, 119);
  //     tft.print(F("mDNS failed"));
  //   }
  //   sprintf(hostIP, "%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3] );
  //   digitalWrite(LED, LOW);
  //   delay(20);
  //   digitalWrite(LED, HIGH);
  //   delay(20);
  //   digitalWrite(LED, LOW);
  //   delay(20);
  //   digitalWrite(LED, HIGH);
  //   delay(20);
  //   ArduinoOTA.onStart([]() {
  //     // DEBUG_PORT.println("Start");
  //     digitalWrite(LED, LOW);
  //     tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
  //     tft.setCursor(2, 120);
  //     tft.print(F("Updating firmware: "));
  //     });
  //   ArduinoOTA.onEnd([]() {
  //     // DEBUG_PORT.println("\nEnd");
  //     tft.fillRect(0, 120, 127, 7, ST7735_BLACK);
  //     tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
  //     tft.setCursor(2, 120);
  //     tft.print(F("Firmware updated."));
  //     digitalWrite(LED, HIGH);
  //   });
  //   ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
  //     // DEBUG_PORT.printf("Progress: %u%%\r", (progress / (total / 100)));
  //     tft.setCursor(104, 120);
  //     tft.print(progress / (total / 100));
  //     tft.print(F("% "));
  //   });
  //   ArduinoOTA.onError([](ota_error_t error) {
  //     // DEBUG_PORT.printf("Error[%u]: ", error);
  //     if (error == OTA_AUTH_ERROR) DEBUG_PORT.println("Auth Failed");
  //     else if (error == OTA_BEGIN_ERROR) DEBUG_PORT.println("Begin Failed");
  //     else if (error == OTA_CONNECT_ERROR) DEBUG_PORT.println("Connect Failed");
  //     else if (error == OTA_RECEIVE_ERROR) DEBUG_PORT.println("Receive Failed");
  //     else if (error == OTA_END_ERROR) DEBUG_PORT.println("End Failed");
  //   });
  //   ArduinoOTA.begin();
  // }
  // else {
  //   // Put modem to sleep
  //   WiFi.mode( WIFI_OFF );
  //   WiFi.forceSleepBegin();
  //   delay( 1 );
  // }

  tft.fillRect(0, 120, 128, 8, ST7735_BLACK);
  // Initialize SD Card.
  tft.setCursor(1, 111);
  if (!SD.begin()) {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  // tft.setCursor(1, 120);
  // tft.print(F("Logging: "));
  // if (logging) {
  //   tft.print(F("on"));
  // }
  // else {
  //   tft.print(F("off"));
  // }

  // // Get initial battery voltage
  // check_vbat(); 

} // setup

void loop() { 
  // Set counter for timing
  current_millis = millis();

  while (gps.available( GPS_PORT )) {
    fix = gps.read();
  }

  // read the state of the switch into a local variable:
  int buttonOne_reading = digitalRead(buttonOne);
  int buttonTwo_reading = digitalRead(buttonTwo);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:
  // If the switch changed, due to noise or pressing:
  if (buttonOne_reading != buttonOne_lastState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
  if (buttonTwo_reading != buttonTwo_lastState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (buttonOne_reading != buttonOne_state) {
      buttonOne_state = buttonOne_reading;


      // CMD button pressed
      if (buttonOne_state == LOW) {
        // if (currentScreen == WEATHER_SCREEN) {
          if (configPage[currentScreen] == true) {
            menuCMD = true;
          }
          configPage[currentScreen] = true;
          // tft.setCursor(112, 2);
          // tft.print(F("C"));  
          tft.fillRect(0, 11, 128, 116, ST7735_BLACK);
          // menuCMD = true;
        // }
      }
    }

    if (buttonTwo_reading != buttonTwo_state) {
      buttonTwo_state = buttonTwo_reading;

      if (buttonTwo_state == LOW) {
        // tft.setCursor(112, 2);
        // tft.print(configPage[currentScreen]);
        if (configPage[currentScreen] != true) {
          currentScreen++;
          currentScreen = currentScreen % NUMBER_OF_SCREENS;
          // Clear screen on change.
          tft.fillRect(0, 11, 128, 107, ST7735_BLACK);
        }
        else {
          currentOption++;
          // menuCMD = true;
        }
      }
    }

    // Change screen.
    switch (currentScreen) {
      case GPS_DATA_SCREEN:
        displayGPS(); 
        break;
      case WEATHER_SCREEN:
        displayWeather();
        break;
      case WIFI_INFO_SCREEN:
        displayWifi();
        break;
      case GPS_SATMAP_SCREEN:
        displaySatMap();
        break;
      case GPS_SATINFO_SCREEN:
        displaySatInfo();
        break;
      }
  }



  if (logging) {
    if (fix.valid.location && fix.valid.time) {
      // Write GPS data to SD card.
      // writeFixToLog(fix);
      // digitalWrite(LED, LOW);
      // delay(10);
      // digitalWrite(LED, HIGH);
      // trace_all( DEBUG_PORT, gps, fix );
    }
  }

  // save the reading. Next time through the loop, it'll be the lastButtonState:
  buttonOne_lastState = buttonOne_reading;
  buttonTwo_lastState = buttonTwo_reading;

  // ArduinoOTA.handle();
  
}




  // // Check battery and update display
  // if (current_millis - last_millis >= BATT_CHECK_INTERVAL) {
  //   check_vbat();
  //   last_millis = current_millis;
  // }

/**
 * Check battery voltage and update the display.
 * 
 * Set a threshold of > 0.1v change to prevent jumpy readings.
 */
// void check_vbat() {
//   measured_vbat = analogRead(VBATPIN) * VOLTAGE_ANALOG_CONSTANT;
//   if (last_vbat == 0) {
//     last_vbat = measured_vbat;
//   }

//   tft.setCursor(96, 1);
//   // Set voltage reading color
//   if (measured_vbat >= BATTERY_HIGH_VOLTAGE) {
//     tft.setTextColor(ST7735_BLACK, ST7735_WHITE);
//     BATTERY_LOW = false;
//   }
//   else if (measured_vbat >= BATTERY_MEDIUM_VOLTAGE) {
//     tft.setTextColor(ST7735_ORANGE, ST7735_WHITE);
//     BATTERY_LOW = false;
//   }
//   else {
//     tft.setTextColor(ST7735_RED, ST7735_WHITE);
//     BATTERY_LOW = true;
//   }
//   tft.print(measured_vbat);
//   tft.println("v");
//   last_vbat = measured_vbat;
//   tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
// }
