/**
 * GPS satellite map state machine and display functions
 */
static void displaySatMap() {
  unsigned long current_millis = millis();
  // Display title bar.
  if (currentScreen != lastScreen) {
    tft.fillRect(0, 1, 112, 10, ST7735_WHITE);
    tft.setTextColor(ST7735_BLACK, ST7735_WHITE);
    tft.setCursor(2, 2);
    tft.print(F("GPS Satellite Map"));
    // tft.fillRect(SATX - SATR, SATY - SATR, SATR * 2, SATR * 2, ST7735_BLACK);
    lastScreen = currentScreen;
  }
  if (configPage[GPS_SATMAP_SCREEN] == true) {
    // tft.fillRect(0, 11, 127, 116, ST7735_BLACK);
    configPage[GPS_SATMAP_SCREEN] = false;  
    currentOption = 0;
    menuCMD = false;
    // gpsSatMapOptions();
  }
  else {
    // Read magnetometer
    if (current_millis - last_millis >= 250) {
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
      tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
      tft.setCursor(2, 121);
      tft.print(headingDegrees, 0);
      tft.print((char)247);
      tft.print(F("  "));
      tft.setCursor(2, 112);
      tft.print(compassDir(headingDegrees));
      tft.print(F("  "));
      displayCompassNeedle();
      drawSatelliteMap(true, true, true, true);
      drawSatellite();
      last_millis = current_millis;
    }
  }
}

/**
 * Draw a circular map that represents the plane upon which satellites can
 * be projected in a top-down 2D view.
 */
void drawSatelliteMap(bool fill, bool outer, bool inner, bool crosshairs) {
  // clear the map
  if (fill) {
  //   tft.fillRect(SATX - SATR, SATY - SATR, SATR * 2, SATR * 2, ST7735_BLACK);
  }
  // Draw outer ring
  if (outer) {
    tft.drawCircle(SATX, SATY, SATR, ST7735_WHITE);
  }
  // Draw inner ring
  if (inner) {
    tft.drawCircle(SATX, SATY, round(SATR/2), ST7735_DARKGREY);
  }
  // Draw crosshairs
  if (crosshairs) {
    tft.drawFastVLine(SATX, SATY - SATR, SATR*2, ST7735_DARKGREY);
    tft.drawFastHLine(SATX - SATR, SATY, SATR*2, ST7735_DARKGREY);
  }
}

/**
 * Display compass needle.
 */
void displayCompassNeedle() {
  int nx, ny, px, py;

  if (abs(prevHeadingDegrees - headingDegrees) >= 2) {
    // Draw inner ring
    tft.drawCircle(SATX, SATY, round(SATR/2), ST7735_DARKGREY);
    // Draw crosshairs
    tft.drawFastVLine(SATX, SATY - SATR, SATR*2, ST7735_DARKGREY);
    tft.drawFastHLine(SATX - SATR, SATY, SATR*2, ST7735_DARKGREY);

    // The X and Y coordinates of the tip of the compass needle.
    nx = round(sin((double)headingDegrees * PI / 180) * (SATR - 1));
    ny = round(cos((double)headingDegrees * PI / 180) * (SATR - 1));
    // Previous needle
    px = round(sin((double)prevHeadingDegrees * PI / 180) * (SATR - 1));
    py = round(cos((double)prevHeadingDegrees * PI / 180) * (SATR - 1));

    tft.drawLine(SATX, SATY, SATX - px, SATY - py, ST7735_BLACK);
    tft.drawLine(SATX, SATY, SATX - nx, SATY - ny, ST7735_RED);
    prevHeadingDegrees = headingDegrees;
  }

}

void drawSatellite() {
  int x, ex, ey;
  // int sat_count = fix.satellites;
  int sat_count = gps.sat_count;
  int j = 0;
  int k = 0;



  for (int i = 0; i < 20; i++) {
    // if (!satData[i].azimuth && !satData[i].elevation) {

    if (satData[i].tracked) {
      // The distance from the center to the satellite.
      x = round(cos(satData[i].elevation * PI / 180) * (SATR - 2));
      // The X and Y coordinates of the satellite on the map.
      ex = round(sin((satData[i].azimuth - last_heading) * PI / 180) * x);
      ey = round(cos((satData[i].azimuth - last_heading) * PI / 180) * x);
      tft.fillCircle(SATX + ex, SATY - ey, 2, ST7735_BLACK);
    }
    // }
  }
 
  for (int i = 0; i < 20; i++) {
    // if (gps.satellites[i].tracked) {
      satData[i].id = gps.satellites[i].id;
      satData[i].elevation = gps.satellites[i].elevation;
      satData[i].azimuth = gps.satellites[i].azimuth;
      satData[i].tracked = gps.satellites[i].tracked;
      if (gps.satellites[i].tracked) {
        j++;
      }
      else if (gps.satellites[i].id) {
        k++;
      }
      // The distance from the center to the satellite.
      x = round(cos(gps.satellites[i].elevation * PI / 180) * (SATR - 2));
      // The X and Y coordinates of the satellite on the map.
      ex = round(sin((gps.satellites[i].azimuth - (int)headingDegrees) * PI / 180) * x);
      ey = round(cos((gps.satellites[i].azimuth - (int)headingDegrees) * PI / 180) * x);
      // if (!gps.satellites[i].azimuth && !gps.satellites[i].elevation) {
        if (gps.satellites[i].tracked) {
          tft.fillCircle(SATX + ex, SATY - ey, 2, ST7735_GREEN);
        }
        else {
          // tft.fillCircle(SATX + ex, SATY - ey, 2, ST7735_RED);
        }
      // }
    // }
  }

  // Clear the rest of the array
  // for (int k = j; k < 20; k++) {
  //   satData[k].id = 0;
  //   satData[k].elevation = 0;
  //   satData[k].azimuth = 0;
  // }

  tft.setCursor(102, 119);
  tft.print(j);
  tft.print(F(":")); 
  tft.print(j + k);
  tft.print(F("  "));
  last_heading = (int)headingDegrees;
}