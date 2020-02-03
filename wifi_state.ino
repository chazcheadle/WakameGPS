/**
 * WiFi state machine and display functions
 */
static void displayWifi() {
  unsigned long current_millis = millis();
  if (currentScreen != lastScreen) {
  // if (current_millis - last_millis >= 1000) {
    // Display title bar.
    tft.fillRect(0, 1, 112, 10, ST7735_WHITE);
    tft.setTextColor(ST7735_BLACK, ST7735_WHITE);
    tft.setCursor(2, 2);
    tft.print(F("WiFi Info"));
    tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
    lastScreen = currentScreen;
    last_millis = 0;
  }
  if (configPage[WIFI_INFO_SCREEN] == true) {
    // tft.fillRect(0, 11, 127, 116, ST7735_BLACK);
    wifiInfoOptions();
  }
  else {
    if (current_millis - last_millis >= 1000) {
      tft.setCursor(2, 12);
      tft.print(F("STATUS: "));
      if ( WiFi.status() != WL_CONNECTED ) {
        tft.print(F("Not connected"));
        tft.setCursor(2, 21);
        tft.print(F("Press CMD to connect"));
      }
      else {
        tft.print(F("Connected"));
        tft.setCursor(2, 21);
        tft.print(F("SSID: "));
        tft.print(ssid);
        tft.setCursor(2, 30);
        tft.print(F("IP: "));
        tft.print(hostIP);
        tft.setCursor(2, 39);
        tft.print(F("DNS: "));
        tft.print(hostString);
        tft.print(F(".local"));
      }
      last_millis = current_millis;
    }
  }
} // displayWifi

void wifiInfoOptions() {


  if (lastOption != currentOption) {
    // tft.fillRect(0, 11, 127, 116, ST7735_BLACK);
  }

  // Set option marker
  currentOption = currentOption % NUM_WIFI_INFO_SCREEN_OPTIONS;
  for (int i= 0; i < NUM_WIFI_INFO_SCREEN_OPTIONS; i++) {
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
      case 0: // Connect (last saved)
        tft.setCursor(1, 120);
        tft.print("Starting WiFi...");
        digitalWrite(LED, LOW);
        delay(2000);
        // Router_SSID = ESP_wifiManager.WiFi_SSID();
        // Router_Pass = ESP_wifiManager.WiFi_Pass();
        // WiFi.mode(WIFI_STA);
        // WiFi.begin(Router_SSID, Router_Pass);
        // delay(5000);
        // if (WiFi.waitForConnectResult() != WL_CONNECTED) {  
        //   // DEBUG_PORT.println("Connection Failed! Rebooting...");
        //   tft.setCursor(1, 112);
        //   tft.print("Cannot connect to WiFi");
        //   tft.setCursor(1, 120);
        //   tft.print("Start WiFi Manager");
        //   tft.setCursor(114, 120);
        //   tft.print(" ");
        //   // ESP.restart();
        // }
        // else {
        //   tft.fillRect(0, 120, 127, 7, ST7735_BLACK);
        //   tft.setTextColor(ST7735_BLACK, ST7735_WHITE);
        //   tft.setCursor(114, 2);
        //   tft.print("W");
        //   tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
        // }
        // // Set mDNS hostname
        // sprintf(hostString, "ESP-%06X", ESP.getChipId());
        // if (!MDNS.begin(hostString)) {
        //   tft.setCursor(1, 119);
        //   tft.print(F("mDNS failed"));
        // }
        // sprintf(hostIP, "%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3] );
        // digitalWrite(LED, LOW);
        // delay(20);
        // digitalWrite(LED, HIGH);
        // delay(20);
        // digitalWrite(LED, LOW);
        // delay(20);
        // digitalWrite(LED, HIGH);
        // delay(20);
        // ArduinoOTA.onStart([]() {
        // // DEBUG_PORT.println("Start");
        // digitalWrite(LED, LOW);
        // tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
        // tft.setCursor(2, 120);
        // tft.print(F("Updating firmware: "));
        // });
        // ArduinoOTA.onEnd([]() {
        // // DEBUG_PORT.println("\nEnd");
        // tft.fillRect(0, 120, 127, 7, ST7735_BLACK);
        // tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
        // tft.setCursor(2, 120);
        // tft.print(F("Firmware updated."));
        // digitalWrite(LED, HIGH);
        // });
        // ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        // // DEBUG_PORT.printf("Progress: %u%%\r", (progress / (total / 100)));
        // tft.setCursor(104, 120);
        // tft.print(progress / (total / 100));
        // tft.print(F("% "));
        // });
        // ArduinoOTA.onError([](ota_error_t error) {
        // // DEBUG_PORT.printf("Error[%u]: ", error);
        // if (error == OTA_AUTH_ERROR) DEBUG_PORT.println("Auth Failed");
        //   else if (error == OTA_BEGIN_ERROR) DEBUG_PORT.println("Begin Failed");
        //   else if (error == OTA_CONNECT_ERROR) DEBUG_PORT.println("Connect Failed");
        //   else if (error == OTA_RECEIVE_ERROR) DEBUG_PORT.println("Receive Failed");
        //   else if (error == OTA_END_ERROR) DEBUG_PORT.println("End Failed");
        // });
        // ArduinoOTA.begin();
        configPage[WIFI_INFO_SCREEN] = false; 
        currentOption = 0;
        break;
      case 1: // Config Portal
        ESP_wifiManager.startConfigPortal();
        break;
      case 2: // Turn WiFi On/Off
        // Put modem to sleep
        // WiFi.mode( WIFI_OFF );
        // WiFi.forceSleepBegin();
        // delay(1);
        // tft.setTextColor(ST7735_BLACK, ST7735_WHITE);
        // tft.setCursor(114, 2);
        // tft.print(" ");
        // tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
        configPage[WIFI_INFO_SCREEN] = false; 
        currentOption = 0;
        break;
      case 3: // Exit
        tft.fillRect(0, 11, 128, 107, ST7735_BLACK);
        delay(1);
        configPage[WIFI_INFO_SCREEN] = false; 
        currentOption = 0;
        break;
    }
    menuCMD = false;
  }
  lastOption = currentOption;

  if (configPage[WIFI_INFO_SCREEN]) {
    tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
    tft.setCursor(8, 12);
    tft.print(F("Connect to Wifi"));
    tft.setCursor(8, 21);
    tft.print(F("WiFi manager"));
    tft.setCursor(8, 30);
    tft.print(F("Turn WiFi Off"));
    tft.setCursor(8, 39);
    tft.print(F("Exit"));
  }



}