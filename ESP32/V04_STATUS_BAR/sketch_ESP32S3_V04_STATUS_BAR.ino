#include <WiFi.h>
#include <ArduinoOTA.h>
#include <time.h>

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <TJpg_Decoder.h>

#include "photo_320x240.h"


// ======================================================
// WIFI
// ======================================================

const char* WIFI_SSID = "Mon";
const char* WIFI_PASSWORD = "Cayxanh82";

const char* OTA_HOSTNAME = "ESP32-S3-LCD";


// ======================================================
// LCD
// ======================================================

#define TFT_CS    10
#define TFT_DC    11
#define TFT_RST   14
#define TFT_MOSI  13
#define TFT_SCLK  12

Adafruit_ST7789 tft(
  TFT_CS,
  TFT_DC,
  TFT_MOSI,
  TFT_SCLK,
  TFT_RST
);


// ======================================================
// JPEG OUTPUT
// ======================================================

bool tft_output(
  int16_t x,
  int16_t y,
  uint16_t w,
  uint16_t h,
  uint16_t *bitmap
) {
  tft.drawRGBBitmap(x, y, bitmap, w, h);
  return true;
}


// ======================================================
// OTA
// ======================================================

bool otaUpdating = false;


// ======================================================
// STATUS BAR
// ======================================================

const int STATUS_BAR_HEIGHT = 24;


// ======================================================
// VE ICON WIFI
// ======================================================

void drawWifiIcon(int x, int y, bool connected) {

  // Cham trung tam
  if (connected) {
    tft.fillCircle(x + 7, y + 16, 2, ST77XX_WHITE);
  } else {
    tft.fillCircle(x + 7, y + 16, 2, ST77XX_RED);
  }

  // Thanh song 1
  tft.drawLine(x + 4, y + 12, x + 7, y + 15, ST77XX_WHITE);
  tft.drawLine(x + 7, y + 15, x + 10, y + 12, ST77XX_WHITE);

  // Thanh song 2
  tft.drawLine(x + 1, y + 9, x + 7, y + 15, ST77XX_WHITE);
  tft.drawLine(x + 7, y + 15, x + 13, y + 9, ST77XX_WHITE);

  // Thanh song 3
  tft.drawLine(x - 2, y + 6, x + 7, y + 15, ST77XX_WHITE);
  tft.drawLine(x + 7, y + 15, x + 16, y + 6, ST77XX_WHITE);

  // Neu mat WiFi, gach cheo
  if (!connected) {
    tft.drawLine(
      x - 2,
      y + 3,
      x + 16,
      y + 20,
      ST77XX_RED
    );
  }
}


// ======================================================
// VE STATUS BAR
// ======================================================

void drawStatusBar() {

  // Nen thanh trang thai
  tft.fillRect(
    0,
    0,
    320,
    STATUS_BAR_HEIGHT,
    ST77XX_BLACK
  );


  // --------------------------------------------------
  // WIFI
  // --------------------------------------------------

  bool wifiConnected =
    (WiFi.status() == WL_CONNECTED);

  drawWifiIcon(
    4,
    1,
    wifiConnected
  );


  // Ten WiFi
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);

  tft.setCursor(22, 4);

  if (wifiConnected) {

    tft.print(WiFi.SSID());

  } else {

    tft.print("WiFi OFF");
  }


  // RSSI
  if (wifiConnected) {
    tft.setCursor(22, 14);

    tft.print(WiFi.RSSI());
    tft.print("dBm");
  }


  // --------------------------------------------------
  // NGUON
  // --------------------------------------------------

  tft.setCursor(170, 8);

  tft.print("USB");


  // --------------------------------------------------
  // DONG HO
  // --------------------------------------------------

  struct tm timeinfo;

  tft.setCursor(263, 8);

  if (getLocalTime(&timeinfo, 10)) {

    char timeString[6];

    strftime(
      timeString,
      sizeof(timeString),
      "%H:%M",
      &timeinfo
    );

    tft.print(timeString);

  } else {

    tft.print("--:--");
  }
}


// ======================================================
// SETUP
// ======================================================

void setup() {

  Serial.begin(115200);

  delay(1000);

  Serial.println();
  Serial.println("================================");
  Serial.println(" ESP32-S3 LCD - V04.1");
  Serial.println(" STATUS BAR");
  Serial.println("================================");


  // --------------------------------------------------
  // LCD
  // --------------------------------------------------

  Serial.println("Khoi tao LCD...");

  tft.init(240, 320);

  tft.setSPISpeed(4000000);

  tft.setRotation(1);

  tft.fillScreen(ST77XX_BLACK);

  tft.invertDisplay(false);

  Serial.println("LCD OK");


  // --------------------------------------------------
  // HIEN ANH
  // --------------------------------------------------

  TJpgDec.setJpgScale(1);

  TJpgDec.setCallback(tft_output);

  Serial.println("Dang hien anh...");

  TJpgDec.drawJpg(
    0,
    0,
    photo_320x240,
    photo_320x240_len
  );

  Serial.println("Hien anh xong!");


  // --------------------------------------------------
  // WIFI
  // --------------------------------------------------

  Serial.println();
  Serial.println("Dang ket noi WiFi...");

  Serial.print("SSID: ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);

  WiFi.setHostname(OTA_HOSTNAME);

  WiFi.setAutoReconnect(true);

  WiFi.begin(
    WIFI_SSID,
    WIFI_PASSWORD
  );


  int dem = 0;

  while (
    WiFi.status() != WL_CONNECTED &&
    dem < 30
  ) {

    delay(500);

    Serial.print(".");

    dem++;
  }

  Serial.println();


  if (WiFi.status() == WL_CONNECTED) {

    Serial.println("WIFI DA KET NOI!");

    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    Serial.print("IP ESP32: ");
    Serial.println(WiFi.localIP());

    Serial.print("RSSI: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");

  } else {

    Serial.println("KHONG KET NOI DUOC WIFI!");
  }


  // --------------------------------------------------
  // NTP - GIO VIET NAM UTC+7
  // --------------------------------------------------

  if (WiFi.status() == WL_CONNECTED) {

    Serial.println();
    Serial.println("Dong bo gio NTP...");

    configTime(
      7 * 3600,   // UTC+7
      0,
      "pool.ntp.org",
      "time.nist.gov"
    );
    struct tm timeinfo;

    if (getLocalTime(&timeinfo, 5000)) {

      Serial.println("DONG BO GIO THANH CONG!");

      Serial.print("Gio: ");

      Serial.println(
        &timeinfo,
        "%d/%m/%Y %H:%M:%S"
      );

    } else {

      Serial.println("KHONG DONG BO DUOC GIO!");
    }
  }


  // --------------------------------------------------
  // OTA
  // --------------------------------------------------

  ArduinoOTA.setHostname(OTA_HOSTNAME);


  ArduinoOTA.onStart([]() {

    otaUpdating = true;

    Serial.println("OTA BAT DAU...");
  });


  ArduinoOTA.onEnd([]() {

    otaUpdating = false;

    Serial.println();

    Serial.println("OTA HOAN TAT!");
  });


  ArduinoOTA.onProgress(
    [](unsigned int progress, unsigned int total) {

      Serial.printf(
        "OTA: %u%%\r",
        (progress * 100) / total
      );
    }
  );


  ArduinoOTA.onError(
    [](ota_error_t error) {

      Serial.printf(
        "\nOTA ERROR[%u]\n",
        error
      );

      otaUpdating = false;
    }
  );


  ArduinoOTA.begin();


  Serial.println();
  Serial.println("OTA DA SAN SANG!");

  Serial.print("Ten OTA: ");
  Serial.println(OTA_HOSTNAME);

  Serial.print("IP OTA: ");
  Serial.println(WiFi.localIP());

  Serial.println("================================");


  // --------------------------------------------------
  // VE STATUS BAR LAN DAU
  // --------------------------------------------------

  drawStatusBar();
}


// ======================================================
// LOOP
// ======================================================

void loop() {

  // Luon xu ly OTA
  ArduinoOTA.handle();


  // --------------------------------------------------
  // CAP NHAT STATUS BAR MOI 1 GIAY
  // --------------------------------------------------

  static unsigned long lastStatusUpdate = 0;

  if (
    millis() - lastStatusUpdate >= 1000
  ) {

    lastStatusUpdate = millis();

    drawStatusBar();
  }


  // --------------------------------------------------
  // SERIAL STATUS MOI 5 GIAY
  // --------------------------------------------------

  static unsigned long lastPrint = 0;

  if (
    millis() - lastPrint >= 5000
  ) {

    lastPrint = millis();


    if (WiFi.status() == WL_CONNECTED) {

      Serial.print(
        "ESP32 online - IP: "
      );

      Serial.println(
        WiFi.localIP()
      );

    } else {

      Serial.println(
        "ESP32 mat WiFi!"
      );
    }
  }
}