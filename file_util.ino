/**
 * SD card utilities and logging functions
 */
// Create new log file
static void createLogFile() {

  if (!sdPresent) {
    if (SD.begin(SD_CS)) {
      sdPresent = true;
      DEBUG_PORT.println("Start SD reader");
    }
    else {
      DEBUG_PORT.println("Failed to initialize SD card reader");
      tft.print(F("No SD reader"));
    }
  }

  // Pick a numbered filename, 00 to 99.
  char filename[15] = "data_##.txt";

  for (uint8_t i=0; i<100; i++) {
    filename[5] = '0' + i/10;
    filename[6] = '0' + i%10;
    DEBUG_PORT.print(F("check: "));
    DEBUG_PORT.println(filename);
    if (!SD.exists(filename)) {
      // Use this one!
      DEBUG_PORT.print(F("Creating: "));
      DEBUG_PORT.println(filename);
      break;
    }
  }
  tft.setCursor(1, 111);
  DEBUG_PORT.print(F("Attempt to create file: "));
  DEBUG_PORT.println(filename);
  logfile = SD.open(filename, FILE_WRITE);
  if (!logfile) {
    tft.print("Cannot create file");
    DEBUG_PORT.println(F("Unable to create file"));
    delay(1000);
    logging = false;
  }
  else {
    logging = true;
    // tft.print(filename);
    logfile.print(F(
      "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
      "<gpx version=\"1.0\">\n"
      "\t<trk>\n\t\t<trkseg>\n"));
    logfile.print(F(GPX_EPILOGUE));
  }
} // createLogFile

// static void writeFloat(float v, fs::FS &fs, int precision) {
//   obufstream ob(buf, sizeof(buf));
//   ob << setprecision(precision) << v;
//   file.print(buf);
// } // writeFloat

// static void writeFormattedDatetime(gps_fix fix, fs::FS &fs) {
//   sprintf(
//       buf,
//       "20%0d-%02d-%02dT%02d:%02d:%02d.%03dZ",
//       fix.dateTime.year,
//       fix.dateTime.month,
//       fix.dateTime.day,
//       fix.dateTime.hours,
//       fix.dateTime.minutes,
//       fix.dateTime.seconds,
//       fix.dateTime_cs);
//   file.print(buf);
// } //writeFormattedDatetime

static void writeFixToLog(gps_fix fix) {
  logfile.seek(logfile.size() + SEEK_TRKPT_BACKWARDS, SeekSet);

  logfile.print(F("\t\t\t<trkpt "));

  // logfile.print(F("lat=\""));
  // writeFloat(fix.latitude(), logfile, 7);
  // logfile.print(F("\" lon=\""));
  // writeFloat(fix.longitude(), logfile, 7);
  // logfile.print(F("\">"));

  // logfile.print(F("<time>"));
  // writeFormattedDatetime(fix, logfile);
  // logfile.print(F("</time>"));

  // logfile.print(F("<ele>")); // meters
  // writeFloat(fix.altitude(), logfile, 1);
  // logfile.print(F("</ele>"));

  // if (fix.speed_mps != TinyGPS::GPS_INVALID_F_SPEED) {
  //   logfile.print(F("<speed>"));
  //   writeFloat(fix.speed_mps, logfile, 1);
  //   logfile.print(F("</speed>"));
  // }
  // if (fix.course_deg != TinyGPS::GPS_INVALID_F_ANGLE) {
  //   logfile.print(F("<course>"));
  //   writeFloat(fix.course_deg, logfile, 1);
  //   logfile.print(F("</course>"));
  // }

  // logfile.print(F("<sat>"));
  // logfile.print(fix.satellites);
  // logfile.print(F("</sat>"));

  // if (fix.valid.hdop) {
  //   logfile.print(F("<hdop>"));
  //   writeFloat(fix.hdop/1000.0, logfile, 2);
  //   logfile.print(F("</hdop>"));
  // }

  logfile.print(F("</trkpt>\n"));

  logfile.print(F(GPX_EPILOGUE));
  logfile.flush();
} // writeFixToLog


