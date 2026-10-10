#include "esp_camera.h"
#include "SD_MMC.h"
#include "FS.h"
#include <WiFi.h>
#include <WebServer.h>

#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

const char *photoPrefix = "/photo_";
int photoNumber = 0;

#define PIR_PIN 13
#define MODE_PIN 12        // Connect to GND at boot for Web Server Mode
#define FLASH_GPIO_NUM 4

unsigned long lastTrigger = 0;
const unsigned long cooldown = 5000;

const char* ap_ssid = "Fotokapan";
const char* ap_password = "12345678";

WebServer server(80);
bool webMode = false;

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(false);
  delay(1000);

  Serial.println("\nESP32-CAM Camera Trap Starting...");

  pinMode(PIR_PIN, INPUT);
  pinMode(MODE_PIN, INPUT_PULLUP);
  pinMode(FLASH_GPIO_NUM, OUTPUT);
  digitalWrite(FLASH_GPIO_NUM, LOW);

  webMode = (digitalRead(MODE_PIN) == LOW);

  if (webMode) {
    Serial.println("MODE_PIN GND: Web Server Mode");
  } else {
    Serial.println("MODE_PIN HIGH: Camera Trap Mode");
  }

  if (!startCamera()) {
    Serial.println("Camera initialization failed!");
    return;
  }

  Serial.println("Initializing SD card...");
  if (!SD_MMC.begin("/sdcard", true)) {
    Serial.println("SD card initialization failed!");
    return;
  }

  if (SD_MMC.cardType() == CARD_NONE) {
    Serial.println("No SD card detected!");
    return;
  }

  Serial.println("SD card initialized.");

  if (webMode) {
    startWebServer();
  } else {
    int lastPhoto = getLastPhotoNumber();
    photoNumber = lastPhoto + 1;

    Serial.printf("Last photo: %d, next photo: photo_%d.jpg\n", lastPhoto, photoNumber);
    Serial.println("Camera trap ready.");
  }
}

void loop() {
  if (webMode) {
    server.handleClient();
    return;
  }

  if (digitalRead(PIR_PIN) == HIGH && millis() - lastTrigger > cooldown) {
    lastTrigger = millis();
    Serial.println("Motion detected.");
    takePhoto();
  }
}

bool startCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;

  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;

  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;

  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;

  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;

  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
    config.fb_location = CAMERA_FB_IN_DRAM;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera error code: 0x%x\n", err);
    return false;
  }

  Serial.println("Camera initialized.");
  return true;
}

void takePhoto() {
  digitalWrite(FLASH_GPIO_NUM, HIGH);
  delay(300);

  for (int i = 0; i < 3; i++) {
    camera_fb_t *discard = esp_camera_fb_get();
    if (discard) {
      esp_camera_fb_return(discard);
    }
    delay(120);
  }

  camera_fb_t *fb = esp_camera_fb_get();

  if (!fb) {
    Serial.println("Photo capture failed!");
    digitalWrite(FLASH_GPIO_NUM, LOW);
    return;
  }

  String photoFileName = String(photoPrefix) + String(photoNumber) + ".jpg";
  Serial.printf("File name: %s\n", photoFileName.c_str());

  File file = SD_MMC.open(photoFileName, FILE_WRITE);

  if (!file) {
    Serial.println("SD card write error!");
  } else {
    file.write(fb->buf, fb->len);
    file.close();

    Serial.printf("Saved: %s, size: %d bytes\n", photoFileName.c_str(), fb->len);
    photoNumber++;
  }

  esp_camera_fb_return(fb);

  delay(200);
  digitalWrite(FLASH_GPIO_NUM, LOW);
}

int extractPhotoNumber(String name) {
  name.toLowerCase();

  int photoIndex = name.indexOf("photo_");
  int jpgIndex = name.indexOf(".jpg");

  if (photoIndex < 0 || jpgIndex < 0 || jpgIndex <= photoIndex + 6) {
    return -1;
  }

  String numStr = name.substring(photoIndex + 6, jpgIndex);
  return numStr.toInt();
}

int getLastPhotoNumber() {
  int maxNum = -1;

  File root = SD_MMC.open("/");
  if (!root) {
    Serial.println("Failed to open SD directory!");
    return -1;
  }

  root.rewindDirectory();

  File file = root.openNextFile();

  while (file) {
    if (!file.isDirectory()) {
      String name = file.name();
      int num = extractPhotoNumber(name);

      if (num > maxNum) {
        maxNum = num;
      }
    }

    file.close();
    file = root.openNextFile();
  }

  root.close();
  return maxNum;
}

void startWebServer() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_password);

  IPAddress IP = WiFi.softAPIP();

  Serial.println("Web server active.");
  Serial.print("WiFi: ");
  Serial.println(ap_ssid);
  Serial.print("Password: ");
  Serial.println(ap_password);
  Serial.print("Address: http://");
  Serial.println(IP);

  server.on("/", handleRoot);
  server.on("/download", handleDownload);
  server.on("/delete", handleDelete);

  server.begin();
}

void handleRoot() {
  String html = "";

  html += "<!DOCTYPE html><html><head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<title>Camera Trap SD Card</title>";
  html += "<style>";
  html += "body{font-family:Arial;background:#111;color:#eee;padding:20px;}";
  html += "a{color:#4da3ff;text-decoration:none;}";
  html += ".card{background:#222;padding:12px;margin:10px 0;border-radius:8px;}";
  html += "</style>";
  html += "</head><body>";

  html += "<h2>Camera Trap SD Card</h2>";
  html += "<p>You can download photos stored on the SD card from here.</p>";

  File root = SD_MMC.open("/");
  if (!root) {
    html += "<p>Failed to open SD card directory.</p>";
    html += "</body></html>";
    server.send(500, "text/html", html);
    return;
  }

  File file = root.openNextFile();
  bool found = false;

  while (file) {
    if (!file.isDirectory()) {
      String name = file.name();

      if (name.endsWith(".jpg") || name.endsWith(".JPG")) {
        found = true;

        html += "<div class='card'>";
        html += name;
        html += " - ";
        html += String(file.size());
        html += " bytes<br><br>";

        html += "<a href='/download?file=";
        html += name;
        html += "'>Download</a>";

        html += " | ";

        html += "<a href='/delete?file=";
        html += name;
        html += "' onclick=\"return confirm('Delete this file?');\">Delete</a>";

        html += "</div>";
      }
    }

    file.close();
    file = root.openNextFile();
  }

  root.close();

  if (!found) {
    html += "<p>No photos found on the SD card.</p>";
  }

  html += "</body></html>";

  server.send(200, "text/html", html);
}

void handleDownload() {
  if (!server.hasArg("file")) {
    server.send(400, "text/plain", "No file specified.");
    return;
  }

  String path = server.arg("file");

  if (!path.startsWith("/")) {
    path = "/" + path;
  }

  File file = SD_MMC.open(path, FILE_READ);

  if (!file) {
    server.send(404, "text/plain", "File not found.");
    return;
  }

  server.streamFile(file, "image/jpeg");
  file.close();
}

void handleDelete() {
  if (!server.hasArg("file")) {
    server.send(400, "text/plain", "No file specified.");
    return;
  }

  String path = server.arg("file");

  if (!path.startsWith("/")) {
    path = "/" + path;
  }

  if (SD_MMC.remove(path)) {
    server.sendHeader("Location", "/");
    server.send(303);
  } else {
    server.send(500, "text/plain", "Failed to delete file.");
  }
}
