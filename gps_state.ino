/**
 * GPS state machine and display functions
 */
static void displayGPS() {

  unsigned long current_millis = millis();
  // Display title bar.
  if (currentScreen != lastScreen) {
    tft.fillRect(0, 1, 112, 10, ST7735_WHITE);
    tft.setTextColor(ST7735_BLACK, ST7735_WHITE);
    tft.setCursor(2, 2);
    tft.print(F("GPS Location"));
    lastScreen = currentScreen;
    last_millis = 0;
  }
  if (configPage[GPS_DATA_SCREEN] == true) {
    // configPage[GPS_DATA_SCREEN] = false;
    // tft.fillRect(0, 11, 127, 116, ST7735_BLACK);
    gpsDataOptions();
  }
  else {
    if (current_millis - last_millis >= 1000) {
      tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
      tft.setCursor(2, 12);
      if (fix.valid.time && fix.valid.date) {
        tft.print(fix.dateTime.year);
        tft.print(F("/"));
        tft.print(fix.dateTime.month);
        tft.print(F("/"));
        tft.print(fix.dateTime.date);
        tft.print(F(" "));
        tft.print(fix.dateTime.hours);
        tft.print(F(":"));
        tft.print(fix.dateTime.minutes);
        tft.print(F(":"));
        tft.print(fix.dateTime.seconds);
        tft.print(F(" "));
      }
      else {
        tft.print(F("Acquiring signal"));
      }
        tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
        tft.setCursor(2, 21);
        tft.print(F("Lat: "));
        if (GPS_COORD_UNITS == 0) {
          tft.print(fix.latitude(), 6);
        }
        else {
          tft.print(fix.latitudeDMS.degrees);
          tft.print((char)247);
          tft.print(fix.latitudeDMS.minutes);
          tft.print(F("'"));
          tft.print(fix.latitudeDMS.seconds_whole);
          tft.print(F("."));
          tft.print(fix.longitudeDMS.seconds_frac);
          tft.print(F("\""));
          tft.print(fix.latitudeDMS.NS());
          tft.print(F("  "));
        }
        tft.setCursor(2, 30);
        tft.print(F("Lon: "));
        if (GPS_COORD_UNITS == 0) {
          tft.print(fix.longitude(), 6);
        }
        else {
          tft.print(fix.longitudeDMS.degrees);
          tft.print((char)247);
          tft.print(fix.longitudeDMS.minutes);
          tft.print(F("'"));
          tft.print(fix.longitudeDMS.seconds_whole);
          tft.print(F("."));
          tft.print(fix.longitudeDMS.seconds_frac);
          tft.print(F("\""));
          tft.print(fix.longitudeDMS.EW());
          tft.print(F("  "));
        }

        tft.setCursor(2, 39);
        tft.print(F("Speed: "));
        if (GPS_DISTANCE_UNITS == 0) {
          tft.print(fix.speed_kph(), 0);
          tft.print(F("kph  "));
        }
        else {
          tft.print(fix.speed_mph(), 0);
          tft.print(F("mph  "));
        }

        tft.setCursor(2, 48);
        tft.print(F("Course: "));
        if (fix.valid.heading) {
          tft.print(fix.heading(), 0);
          tft.print((char)247);
          tft.print(F(" / "));
          tft.print(compassDir(fix.heading()));
        }
        else {
          tft.fillRect(54, 48, 125, 8, ST7735_BLACK);
          tft.print(F("n/a"));
        }
        tft.setCursor(2, 57);
        tft.print(F("Alt: "));
        if (GPS_DISTANCE_UNITS == 0) {
          tft.print(fix.altitude(), 1);
          tft.print(F("m  "));
        }
        else {
          tft.print(fix.altitude() * 3.2808, 1);
          tft.print(F("ft  "));
        }

        tft.setCursor(2, 66);
        // tft.print(F("HtGd: "));
        // tft.print(fix.geoidHeight());
        // tft.println(F("m"));
        // tft.setCursor(2, 66);
        tft.print(F("Hdop: "));
        tft.print(fix.hdop/1000.0, 2);
        tft.print(F(" Qual: "));
        tft.print(fix.status);
        tft.print(F(" "));

        last_millis = current_millis;

    }
    // Read magnetometer
    if (current_millis - last_millis >= 500) {
      sensors_event_t event;
      mag.getEvent(&event);

      float heading = atan2(event.magnetic.y, event.magnetic.x);
      float declinationAngle = -0.2197;
      heading += declinationAngle;
      // Correct for when signs are reversed.
      if(heading < 0)
        heading += 2*PI;

      // Check for wrap due to addition of declination.
      if(heading > 2*PI)
        heading -= 2*PI;    // Convert radians to degrees for readability.

      headingDegrees = (int)(heading * 180/M_PI);
      tft.setCursor(2, 75);
      tft.print(F("Heading: "));
      tft.print(headingDegrees, 0);
      tft.print((char)247);
      tft.print(F("  "));
    }
  }
} // displayGPS

static void gpsDataOptions() {

  if (lastOption != currentOption) {
    // tft.fillRect(0, 11, 127, 116, ST7735_BLACK);
  }

  // Set option marker
  currentOption = currentOption % NUM_GPS_DATA_OPTIONS;
  for (int i= 0; i < NUM_GPS_DATA_OPTIONS; i++) {
    tft.setCursor(1, 12 + 9 * i);
    if (currentOption == i) {
      tft.print(F("*"));
    }
    else {
      tft.print(F(" "));
      }
  }

  if (menuCMD == true) {
    // tft.setCursor(112, 2);
    // tft.print(currentOption);
    switch (currentOption) {
      case 0: // Start/stop logging
        tft.setTextColor(ST7735_BLACK, ST7735_WHITE);
        tft.setCursor(120, 2);
        if (logging) {
          logfile.close();
          tft.print(F(" "));
          // Turn off logging to SD card.
          DEBUG_PORT.println(F("Logging stopped"));
          logging = false;
        }
        else {
          tft.print(F("L"));
          createLogFile();
          DEBUG_PORT.println(F("Logging started"));
        }
        tft.fillRect(0, 11, 128, 107, ST7735_BLACK);
        configPage[GPS_DATA_SCREEN] = false;
        currentOption = 0;
        break;
      case 1: // Change distance units
        GPS_DISTANCE_UNITS = !GPS_DISTANCE_UNITS;
        break;
      case 2: // Change coordinate units
        GPS_COORD_UNITS = !GPS_COORD_UNITS;
        break;
      case 3: // Change sampling rate
        currentGPSSampleRate++;
        currentGPSSampleRate = currentGPSSampleRate % NUMBER_GPS_SAMPLE_RATES;
        break;
      case 4: // Save configuration
        tft.fillRect(0, 11, 128, 107, ST7735_BLACK);
        configPage[GPS_DATA_SCREEN] = false;
        currentOption = 0;
        break;
      case 5:
        tft.fillRect(0, 11, 128, 107, ST7735_BLACK);
        delay(1);
        configPage[GPS_DATA_SCREEN] = false; 
        currentOption = 0;
        break;
    }
    menuCMD = false;
  }
  lastOption = currentOption;

  if (configPage[GPS_DATA_SCREEN]) {
    tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
    tft.setCursor(8, 12);
    if (logging) {
      tft.print(F("Stop Logging"));
    }
    else {
      tft.print(F("Start Logging"));
    }

    tft.setCursor(8, 21);
    tft.print(F("Distance units: "));
    if (GPS_DISTANCE_UNITS == 0) {
      tft.print(F("Metric "));
    }
    else {
      tft.print(F("Imperial"));
    }

    tft.setCursor(8, 30);
    tft.print(F("Lat/Lon: "));
    if (GPS_COORD_UNITS == 0) {
      tft.print(F("D.DDD"));
      tft.print((char)247);
    } 
    else {
      tft.print(F("DMS  "));
    }

    tft.setCursor(8, 39);
    tft.print(F("Sample Rate: "));
    tft.print(GPS_SAMPLE_RATE[currentGPSSampleRate]);
    tft.print(F("Hz"));

    tft.setCursor(8, 48);
    tft.print(F("Save"));

    tft.setCursor(8, 55);
    tft.print(F("Exit"));
  }

} // gpsDataOptions

const char nCD  [] PROGMEM = "N";
const char nneCD[] PROGMEM = "NNE";
const char neCD [] PROGMEM = "NE";
const char eneCD[] PROGMEM = "ENE";
const char eCD  [] PROGMEM = "E";
const char eseCD[] PROGMEM = "ESE";
const char seCD [] PROGMEM = "SE";
const char sseCD[] PROGMEM = "SSE";
const char sCD  [] PROGMEM = "S";
const char sswCD[] PROGMEM = "SSW";
const char swCD [] PROGMEM = "SW";
const char wswCD[] PROGMEM = "WSW";
const char wCD  [] PROGMEM = "W";
const char wnwCD[] PROGMEM = "WNW";
const char nwCD [] PROGMEM = "NW";
const char nnwCD[] PROGMEM = "NNW";

const char * const dirStrings[] PROGMEM =
  { nCD, nneCD, neCD, eneCD, eCD, eseCD, seCD, sseCD, 
    sCD, sswCD, swCD, wswCD, wCD, wnwCD, nwCD, nnwCD };

const __FlashStringHelper *compassDir( uint16_t bearing ) // degrees CW from N
{
  const int16_t directions    = sizeof(dirStrings)/sizeof(dirStrings[0]);
  const int16_t degreesPerDir = 360 / directions;
        int8_t  dir           = (bearing + degreesPerDir/2) / degreesPerDir;

  while (dir < 0)
    dir += directions;
  while (dir >= directions)
    dir -= directions;

  return (const __FlashStringHelper *) pgm_read_ptr( &dirStrings[ dir ] );

} // compassDir