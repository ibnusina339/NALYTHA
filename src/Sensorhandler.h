#include <Wire.h>
#include <Adafruit_BME280.h>
#include <TinyGPSPlus.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

// Define pins untuk ESP32-S3 (sesuaikan dengan rangkaian Anda)
#define sdapin 3    // I2C SDA pin untuk ESP32-S3
#define sclpin 4    // I2C SCL pin untuk ESP32-S3
#define rxpin 17    // GPS RX pin
#define txpin 18    // GPS TX pin

class sensorhandler {
private:
  Adafruit_BME280 bme;
  TinyGPSPlus gps;
  HardwareSerial* gpsserial;
  bool useBME, useGPS;
  SemaphoreHandle_t sensorMutex;  // Mutex untuk akses sensor yang aman dalam lingkungan multi-task
  TaskHandle_t gpsTaskHandle;     // Handle untuk task GPS
  
  static void gpsUpdateTask(void* parameter);  // Task untuk memperbarui data GPS di background
  bool gpsDataUpdated;                         // Flag untuk menandai pembaruan data GPS

public:
  sensorhandler(HardwareSerial* gpsserial, bool useBME);
  ~sensorhandler();

  void begin();
  float gethumidity();
  float gettemperature();
  float getpressure();
  float getaltitude();
  float getlatitude();
  float getlongitude();
  int getsatellite();
  String getweather(float latitude, float longitude);
  String gettime();
  String getdate();
};

// Task background untuk memproses data GPS
void sensorhandler::gpsUpdateTask(void* parameter) {
  sensorhandler* handler = (sensorhandler*)parameter;
  
  for (;;) {
    if (handler->gpsserial->available()) {
      // Ambil mutex sebelum mengakses data GPS
      if (xSemaphoreTake(handler->sensorMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        while (handler->gpsserial->available()) {
          handler->gps.encode(handler->gpsserial->read());
        }
        handler->gpsDataUpdated = true;
        xSemaphoreGive(handler->sensorMutex);
      }
    }
    // Beri waktu untuk task lain
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

sensorhandler::sensorhandler(HardwareSerial* gpsserial, bool useBME)
  : gpsserial(gpsserial), useBME(useBME), useGPS(true), gpsDataUpdated(false) {
  
  // Buat mutex untuk koordinasi akses sensor
  sensorMutex = xSemaphoreCreateMutex();
}

sensorhandler::~sensorhandler() {
  // Bersihkan resource saat objek dihapus
  if (gpsTaskHandle != NULL) {
    vTaskDelete(gpsTaskHandle);
  }
  
  if (sensorMutex != NULL) {
    vSemaphoreDelete(sensorMutex);
  }
}

void sensorhandler::begin() {
  if (useBME) {
    Wire.begin(sdapin, sclpin);
    if (!bme.begin(0x76)) {
      Serial.println("BME280 sensor tidak ditemukan, periksa koneksi!");
    }
  }

  if (useGPS) {
    gpsserial->begin(9600, SERIAL_8N1, rxpin, txpin);
    
    // Buat task terpisah untuk memproses data GPS
    xTaskCreate(
      gpsUpdateTask,     // Fungsi task
      "GPS_Update",      // Nama task
      4096,              // Stack size (bytes)
      this,              // Parameter untuk task
      1,                 // Prioritas (1 = rendah)
      &gpsTaskHandle     // Task handle
    );
  }
}

float sensorhandler::gettemperature() {
  if (!useBME) return -1.0;
  
  float temp = -1.0;
  if (xSemaphoreTake(sensorMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    temp = bme.readTemperature();
    xSemaphoreGive(sensorMutex);
  }
  return temp;
}

float sensorhandler::gethumidity() {
  if (!useBME) return -1.0;
  
  float humidity = -1.0;
  if (xSemaphoreTake(sensorMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    humidity = bme.readHumidity();
    xSemaphoreGive(sensorMutex);
  }
  return humidity;
}

float sensorhandler::getpressure() {
  if (!useBME) return -1.0;
  
  float pressure = -1.0;
  if (xSemaphoreTake(sensorMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    pressure = bme.readPressure() / 100.0f;
    xSemaphoreGive(sensorMutex);
  }
  return pressure;
}

float sensorhandler::getaltitude() {
  if (!useBME) return -1.0;
  
  float altitude = -1.0;
  if (xSemaphoreTake(sensorMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    altitude = bme.readAltitude(1013.25); // 1013.25 = tekanan laut standar
    xSemaphoreGive(sensorMutex);
  }
  return altitude;
}

int sensorhandler::getsatellite() {
  int satellites = 0;
  
  if (xSemaphoreTake(sensorMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    satellites = gps.satellites.isValid() ? gps.satellites.value() : 0;
    xSemaphoreGive(sensorMutex);
  }
  
  return satellites;
}

float sensorhandler::getlatitude() {
  float latitude = 0.0;
  
  if (xSemaphoreTake(sensorMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    latitude = gps.location.isValid() ? gps.location.lat() : 0.0;
    xSemaphoreGive(sensorMutex);
  }
  
  return latitude;
}

float sensorhandler::getlongitude() {
  float longitude = 0.0;
  
  if (xSemaphoreTake(sensorMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    longitude = gps.location.isValid() ? gps.location.lng() : 0.0;
    xSemaphoreGive(sensorMutex);
  }
  
  return longitude;
}

String sensorhandler::getweather(float latitude, float longitude) {
  // Implementasi untuk mendapatkan data cuaca
  // Idealnya, fungsi ini akan menggunakan WiFi untuk mengakses API cuaca
  // Contoh placeholder saja, Anda perlu mengimplementasikan sesuai kebutuhan
  return "Data cuaca tidak tersedia";
}

String sensorhandler::gettime() {
  String timeStr = "00:00:00";
  
  if (xSemaphoreTake(sensorMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    if (gps.time.isValid()) {
      char buffer[10];
      sprintf(buffer, "%02d:%02d:%02d", 
              gps.time.hour(), 
              gps.time.minute(), 
              gps.time.second());
      timeStr = String(buffer);
    }
    xSemaphoreGive(sensorMutex);
  }
  
  return timeStr;
}

String sensorhandler::getdate() {
  String dateStr = "00/00/0000";
  
  if (xSemaphoreTake(sensorMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    if (gps.date.isValid()) {
      char buffer[12];
      sprintf(buffer, "%02d,%02d,%04d", 
              gps.date.day(), 
              gps.date.month(), 
              gps.date.year());
      dateStr = String(buffer);
    }
    xSemaphoreGive(sensorMutex);
  }
  
  return dateStr;
}