/**
 * Weather state machine and display functions
 */
static void displayWeather() {
  unsigned long current_millis = millis();
  if (currentScreen != lastScreen) {
    // Display title bar.
    tft.fillRect(0, 1, 112, 10, ST7735_WHITE);
    tft.setTextColor(ST7735_BLACK, ST7735_WHITE);
    tft.setCursor(2, 2);
    tft.print(F("Weather Data"));
    lastScreen = currentScreen;
    last_millis = 0;
  }
  if (configPage[WEATHER_SCREEN] == true) {
    // tft.fillRect(0, 11, 127, 116, ST7735_BLACK);
    weatherOptions();
  }
  else {
    if (current_millis - last_millis >= 1000) {
      // tft.fillRect(0, 11, 127, 107, ST7735_BLACK);
      tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
      tft.setCursor(2, 12);
      tft.print(F("Temp: "));
      if (WEATHER_TEMP_UNITS == 0) {
        tft.print(bme.readTemperature());
        tft.print((char)247);
        tft.print(F("C "));
      }
      else {
        tft.print((bme.readTemperature() * 1.8) + 32);
        tft.print((char)247);
        tft.print(F("F "));
      }
      tft.setCursor(2, 21);
      tft.print(F("Pressure: "));
      tft.print(bme.readPressure() / 100, 2);
      tft.print(F("hPa "));

      tft.setCursor(2, 30);
      tft.print(F("Humidity: "));
      tft.print(bme.readHumidity(), 0);
      tft.print(F("%"));
      last_millis = current_millis;
    }
  }
} // displayWeather

static void weatherOptions() {

  if (lastOption != currentOption) {
    // tft.fillRect(0, 11, 127, 112, ST7735_BLACK);
  }

  // Set option marker
  currentOption = currentOption % NUM_WEATHER_SCREEN_OPTIONS;
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
    switch (currentOption) {
      case 0: // Change temperature units
        WEATHER_TEMP_UNITS = !WEATHER_TEMP_UNITS;
        break;
      case 1: // Set sea level pressure
        tft.fillRect(0, 11, 128, 107, ST7735_BLACK);
        configPage[WEATHER_SCREEN] = false;  
        currentOption = 0;
        break;
      case 2: // Save configuration
        tft.fillRect(0, 11, 128, 107, ST7735_BLACK);
        configPage[WEATHER_SCREEN] = false;  
        currentOption = 0;
        break;
      case 3: // Exit
        tft.fillRect(0, 11, 128, 107, ST7735_BLACK);
        configPage[WEATHER_SCREEN] = false;  
        currentOption = 0;
        break;
    }
    menuCMD = false;
  }
  lastOption = currentOption;

  if (configPage[WEATHER_SCREEN]) {
    tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
    tft.setCursor(8, 12);
    tft.print(F("Units: "));
    if (WEATHER_TEMP_UNITS == 0) {
      tft.print(F("Celcius   "));
    }
    else {
      tft.print(F("Fahrenheit"));
    }

    tft.setCursor(8, 21);
    tft.print(F("Sea Lvl hPa: "));
    tft.print(SEALEVELPRESSURE_HPA);
    tft.print(F("hPa"));

    tft.setCursor(8, 30);
    tft.print(F("Save"));

    tft.setCursor(8, 39);
    tft.print(F("Exit"));
  }

} // weatherOptions