#ifndef OTAHANDLER_H
#define OTAHANDLER_H

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Update.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define EEPROM_SIZE 64
#define VERSION_ADDRESS 0
#define VERSION_LENGTH 20
#define MAX_RETRIES 3
#define STACK_SIZE 8192
#define OTA_TASK_PRIORITY 3

const char* github_api_url = "https://api.github.com/repos/ibnusina339/NALYTHA-Project/releases/latest";
String firmware_download_url = "";
const String default_current_version = "1.0.0";
String latest_version_global = "";

String getStoredVersion() {
  EEPROM.begin(EEPROM_SIZE);
  char version[VERSION_LENGTH + 1];
  for (int i = 0; i < VERSION_LENGTH; i++) {
    version[i] = char(EEPROM.read(VERSION_ADDRESS + i));
  }
  version[VERSION_LENGTH] = '\0';
  String stored = String(version);
  stored.trim();

  if (stored.length() == 0 || stored == String('\0')) {
    return default_current_version;
  }
  return stored;
}

void setStoredVersion(String newVersion) {
  EEPROM.begin(EEPROM_SIZE);
  for (int i = 0; i < VERSION_LENGTH; i++) {
    if (i < newVersion.length()) {
      EEPROM.write(VERSION_ADDRESS + i, newVersion[i]);
    } else {
      EEPROM.write(VERSION_ADDRESS + i, 0);
    }
  }
  EEPROM.commit();
}

String extractVersionFromFileName(String filename) {
  int vPos = filename.indexOf("-v");
  if (vPos == -1) {
    vPos = filename.indexOf("v");
    if (vPos == -1) return "";
  } else {
    vPos += 1;
  }

  String version = "";
  for (int i = vPos + 1; i < filename.length(); i++) {
    if (isDigit(filename[i]) || filename[i] == '.') {
      version += filename[i];
    } else if (version.length() > 0) {
      break;
    }
  }
  return version;
}

bool getLatestReleaseInfo() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.reconnect();
    vTaskDelay(pdMS_TO_TICKS(5000));
    if (WiFi.status() != WL_CONNECTED) {
      return false;
    }
  }

  for (int retry = 0; retry < MAX_RETRIES; retry++) {
    WiFiClientSecure client;
    client.setInsecure();
    client.setTimeout(10);

    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.addHeader("User-Agent", "ESP32");
    http.addHeader("Accept", "application/vnd.github.v3+json");

    if (!http.begin(client, github_api_url)) {
      vTaskDelay(pdMS_TO_TICKS(1000));
      continue;
    }

    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      DynamicJsonDocument doc(8192);
      DeserializationError error = deserializeJson(doc, payload);

      if (error) {
        http.end();
        vTaskDelay(pdMS_TO_TICKS(1000));
        continue;
      }

      const char* tag_name = doc["tag_name"];
      if (!tag_name) {
        http.end();
        vTaskDelay(pdMS_TO_TICKS(1000));
        continue;
      }

      String version = String(tag_name);
      if (version.startsWith("v")) {
        version = version.substring(1);
      }
      latest_version_global = version;

      bool found_firmware = false;
      for (JsonObject asset : doc["assets"].as<JsonArray>()) {
        const char* name = asset["name"];
        const char* browser_download_url = asset["browser_download_url"];

        if (name && browser_download_url) {
          String assetName = String(name);
          if (assetName.endsWith(".bin") && assetName.indexOf("NAT-A-fw") >= 0) {
            firmware_download_url = String(browser_download_url);
            found_firmware = true;

            String fileVersion = extractVersionFromFileName(assetName);
            if (fileVersion.length() > 0) {
              latest_version_global = fileVersion;
            }
            break;
          }
        }
      }

      if (!found_firmware) {
        firmware_download_url = "https://github.com/ibnusina339/NALYTHA-Project/releases/download/" + String(tag_name) + "/NAT-A-fw-" + latest_version_global + ".bin";
      }

      http.end();
      return true;
    } else {
      http.end();
      if (retry < MAX_RETRIES - 1) {
        vTaskDelay(pdMS_TO_TICKS((retry + 1) * 2000));
      }
    }
  }

  latest_version_global = "1.0.10";
  firmware_download_url = "https://github.com/ibnusina339/NALYTHA-Project/releases/download/v1.0.10/NAT-A-fw-v1.0.10.bin";
  return true;
}

bool checkForUpdate() {
  if (!getLatestReleaseInfo()) {
    return false;
  }


  String stored_version = getStoredVersion();
  return (latest_version_global != stored_version);
}

bool performOTAUpdate() {
  for (int retry = 0; retry < MAX_RETRIES; retry++) {
    WiFiClientSecure client;
    client.setInsecure();
    client.setTimeout(10);

    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    if (!http.begin(client, firmware_download_url)) {
      vTaskDelay(pdMS_TO_TICKS(1000));
      continue;
    }

    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
      int contentLength = http.getSize();

      if (contentLength <= 0) {
        http.end();
        vTaskDelay(pdMS_TO_TICKS(1000));
        continue;
      }

      WiFiClient* stream = http.getStreamPtr();

      if (Update.begin(contentLength)) {
        size_t totalWritten = 0;
        uint8_t buff[512];
        bool timeout = false;
        unsigned long lastDataTime = millis();

        while (http.connected() && totalWritten < contentLength && !timeout) {
          size_t available = stream->available();

          if (available) {
            int bytesRead = stream->readBytes(buff, (available > sizeof(buff)) ? sizeof(buff) : available);
            size_t written = Update.write(buff, bytesRead);
            totalWritten += written;
            lastDataTime = millis();
            vTaskDelay(pdMS_TO_TICKS(1));
          } else {
            if (millis() - lastDataTime > 10000) {
              timeout = true;
            }
            vTaskDelay(pdMS_TO_TICKS(10));
          }
        }

        if (timeout) {
          Update.abort();
          http.end();
          vTaskDelay(pdMS_TO_TICKS(1000));
          continue;
        }

        if (totalWritten == contentLength && Update.end()) {
          if (latest_version_global != "") {
            setStoredVersion(latest_version_global);
          }
          vTaskDelay(pdMS_TO_TICKS(1000));
          ESP.restart();
          return true;
        }
      }
    }

    http.end();

    if (retry < MAX_RETRIES - 1) {
      vTaskDelay(pdMS_TO_TICKS((retry + 1) * 2000));
    }
  }

  return false;
}

void otaUpdateTask(void *parameter) {
  while (true) {
    if (checkForUpdate()) {
      performOTAUpdate();
    }
    vTaskSuspend(NULL);
  }
}

void startOtaUpdateTask(void *parameter) {
  xTaskCreate(
    otaUpdateTask,
    "OTAUpdateTask",
    STACK_SIZE,
    NULL,
    OTA_TASK_PRIORITY,
    NULL
  );
}

void checkAndUpdateFirmware() {
  if (checkForUpdate()) {
    performOTAUpdate();
  }
}

#endif  // OTAHANDLER_H