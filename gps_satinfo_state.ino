/**
 * GPS satellite info state machine and display functions
 */
static void displaySatInfo() {
  unsigned long current_millis = millis();
  int tracked_sats = 0;
  // Display title bar.
  if (currentScreen != lastScreen) {
    tft.fillRect(0, 1, 112, 10, ST7735_WHITE);
    tft.setTextColor(ST7735_BLACK, ST7735_WHITE);
    tft.setCursor(2, 2);
    tft.print(F("Satellite Info"));
    lastScreen = currentScreen;
  }
  if (configPage[GPS_SATINFO_SCREEN] == true) {
    configPage[GPS_SATINFO_SCREEN] = false;  
    currentOption = 0;
    menuCMD = false;
  }
  else {
    if (current_millis - last_millis >= 3000) {
      tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
      tft.setCursor(2, 21);
      for (int i = 0; i < gps.sat_count; i++) {
        if (gps.satellites[i].tracked) {
          tft.setCursor(2, 21 + tracked_sats * 9);
          tft.print(gps.satellites[i].id);
          tft.print(F(": "));
          tft.print(gps.satellites[i].elevation);
          tft.print((char)247);
          tft.print(F(" / "));
          tft.print(gps.satellites[i].azimuth);
          tft.print((char)247);
          tft.print(F(" "));
          tracked_sats++;
        }
      }
      tft.fillRect(0, 21 + tracked_sats * 9, 127, 127 - 29 + tracked_sats * 9, ST7735_BLACK);
      tft.setCursor(2, 12);
      tft.print(F("Fix/Vis: "));
      tft.print(tracked_sats);
      tft.print(F(" / "));
      tft.print(gps.sat_count);
      tft.print(F(" "));
      last_millis = current_millis;
    }
  }
}