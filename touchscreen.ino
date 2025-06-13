#include <SPI.h>
#include <XPT2046_Touchscreen.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "esp_wpa2.h" 
#include <ESPping.h>


// Touchscreen pins
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

const char* ssid = "REDACTED"; 
const char* password = "REDACTED"; 

//IPs for Remote Relay Control
const char* serverIPW_1 = "http://127.0.0.1"; 
const char* serverIPW_2 = "http://127.0.0.2"; 
const char* serverIPL_1 = "http://127.0.0.3"; 
const char* serverIPL_2 = "http://127.0.0.4"; 
const char* serverIPL_3 = "http://127.0.0.5"; 
const char* serverIPL_4 = "http://127.0.0.6"; 
const char* serverIPL_5 = "http://127.0.0.7"; 
const char* serverIPL_6 = "http://127.0.0.8"; 
const char* serverIPL_7 = "http://127.0.0.9"; 
const char* serverIPL_8 = "http://127.0.0.10"; 

SPIClass mySpi = SPIClass(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();
TFT_eSPI_Button key[10];
TFT_eSPI_Button extraBtn1;
TFT_eSPI_Button extraBtn2;

bool extrasVisible = false;
bool visible = false;
unsigned long lastTouchTime = 0;
int lastKeyPressed = -1;
bool onPressed = false;
bool offPressed = false;

int extraBtn1_x, extraBtn1_y, extraBtn1_w, extraBtn1_h;
int extraBtn2_x, extraBtn2_y, extraBtn2_w, extraBtn2_h;

int key_x[10], key_y[10], key_w[10], key_h[10]; // Arrays to store button positions and sizes

void drawButtons() {
  uint16_t bWidth = tft.width() / 4;  // Adjust width to fit 4 buttons per row
  uint16_t bHeight = tft.height() / 5; // Reduced height for smaller buttons

  for (int i = 0; i < 10; i++) {
    uint16_t xCenter = bWidth * (i % 4) + bWidth / 2;
    uint16_t yCenter = bHeight * (i / 4) + bHeight / 2;
    key_x[i] = xCenter - bWidth / 2; // Save x position
    key_y[i] = yCenter - bHeight / 2; // Save y position
    key_w[i] = bWidth; // Save width
    key_h[i] = bHeight; // Save height

    key[i].initButton(&tft, xCenter, yCenter, bWidth, bHeight, TFT_WHITE, TFT_PINK, TFT_WHITE, "", 2);
  }

  key[0].drawButton(false, "NaN"); //Rename according to your loads
  key[1].drawButton(false, "NaN"); //..
  key[2].drawButton(false, "NaN");
  key[3].drawButton(false, "NaN");
  key[4].drawButton(false, "NaN");
  key[5].drawButton(false, "NaN");
  key[6].drawButton(false, "NaN");
  key[7].drawButton(false, "NaN");
  key[8].drawButton(false, "NaN");
  key[9].drawButton(false, "NaN"); 

  int btnWidth = tft.width() / 3;
  int btnHeight = 40;
  int btnY = tft.height() - btnHeight - 10;

  extraBtn1_x = btnWidth;
  extraBtn2_x = 2 * btnWidth;
  extraBtn1_y = extraBtn2_y = btnY;
  extraBtn1_w = extraBtn2_w = btnWidth - 10;
  extraBtn1_h = extraBtn2_h = btnHeight;

  extraBtn1.initButton(&tft, extraBtn1_x, extraBtn1_y, extraBtn1_w, extraBtn1_h, TFT_WHITE, TFT_GREEN, TFT_BLACK, "ON", 2);
  extraBtn2.initButton(&tft, extraBtn2_x, extraBtn2_y, extraBtn2_w, extraBtn2_h, TFT_WHITE, TFT_RED, TFT_BLACK, "OFF", 2);
}


void setup() {
  Serial.begin(115200);
  
  mySpi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(mySpi);
  ts.setRotation(1);

  tft.begin();
  tft.invertDisplay(1);
  tft.setRotation(1);

  tft.fillScreen(TFT_BLACK);
  tft.setFreeFont(&FreeMono9pt7b);

  // Connect to WiFi
  WiFi.mode(WIFI_STA); 
  
 WiFi.begin(ssid,password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("WiFi Connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  
 
drawButtons();
 
}

void sendRequest(String endpoint) {
  if (lastKeyPressed < 0 || lastKeyPressed > 9) return;  
    const char* servers[] = {
    serverIPW_1, serverIPW_2, serverIPL_1, serverIPL_2, serverIPL_3,
    serverIPL_4, serverIPL_5, serverIPL_6, serverIPL_7, serverIPL_8
  };
  HTTPClient http;
  String fullURL = String(servers[lastKeyPressed]) + endpoint;
  http.begin(fullURL);
  Serial.println("Connecting to " + fullURL);
  
  int httpCode = http.GET();        

  if (httpCode > 0) {
    Serial.printf("HTTP GET Response: %d\n", httpCode);
  } else {
    Serial.printf("HTTP GET failed: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end(); 

}
void loop() {
  TS_Point p;
 onPressed=false;
 offPressed=false;
  bool touched = ts.touched();
  int x = -1, y = -1;
for (int i = 0; i < 10; i++) {
  if (key[i].justPressed()) {
    lastKeyPressed = i;
    Serial.print("Key selected: ");
    Serial.println(i);
  }
}

  //Serial.println(lastKeyPressed);
  if (touched) {
    p = ts.getPoint();
    x = map(p.x, 300, 3900, 0, tft.width());
    y = map(p.y, 300, 3900, 0, tft.height());
    lastTouchTime = millis();
  }

  for (int b = 0; b < 10; b++) { 
    if (touched) {
      key[b].press(key[b].contains(x, y));
    } else {
      key[b].press(false);
    }

    if (key[b].justPressed() & !visible) {
      Serial.printf("Button %d pressed\n", b + 1);

      if (b < 2) {
        key[b].drawButton(true, "W_" + String(b + 1));
        visible=true;
      } else {
        key[b].drawButton(true, "L_" + String(b - 1));
        visible=true;
      }

      if (!extrasVisible) {
   

        extraBtn1.drawButton(onPressed); // retain last state
        extraBtn2.drawButton(offPressed);
        extrasVisible = true;
      }
    }

  

    if (key[b].justReleased()) {
      Serial.printf("Button %d released\n", b + 1);

      if (b < 2) {
        key[b].drawButton(false, "W_" + String(b + 1));
      } else {
        key[b].drawButton(false, "L_" + String(b - 1));
      }

    }
  }
  if (extrasVisible && touched) {
    extraBtn1.press(extraBtn1.contains(x, y));
    extraBtn2.press(extraBtn2.contains(x, y));
  } else {
    extraBtn1.press(false);
    extraBtn2.press(false);
  }



  if (extraBtn1.justPressed()&& lastKeyPressed != -1) {
    
    extraBtn1.drawButton(true);
    extraBtn2.drawButton(false);
    sendRequest("/on"); 

    
    onPressed = true;
    offPressed = false;

  }
else if (extraBtn2.justPressed()&& lastKeyPressed != -1) {

    extraBtn2.drawButton(true);
    extraBtn1.drawButton(false);
    sendRequest("/off");
    onPressed = false;
    offPressed = true;

  }

  if (extrasVisible && (millis() - lastTouchTime > 1000)) {
    // Hide extra buttons after 1 second of inactivity
    tft.fillRect(extraBtn1_x - extraBtn1_w / 2, extraBtn1_y - extraBtn1_h / 2, extraBtn1_w, extraBtn1_h, TFT_BLACK);
    tft.fillRect(extraBtn2_x - extraBtn2_w / 2, extraBtn2_y - extraBtn2_h / 2, extraBtn2_w, extraBtn2_h, TFT_BLACK);
    extrasVisible = false;
  }
  for (int b = 0; b < 10; b++) {
  if (visible && (millis() - lastTouchTime > 1000)) {
    tft.fillRect(key_x[b] - key_w[b] / 2, key_y[b] - key_h[b] / 2, key_w[b], key_h[b], TFT_PINK);
    visible = false;
  }}

  delay(20);
    
  
}



