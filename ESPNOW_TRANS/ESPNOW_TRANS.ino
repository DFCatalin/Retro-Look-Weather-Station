#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <MAX44009.h>

#define uS_TO_S_FACTOR 1000000    //Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP  300        //Time ESP32 will go to sleep (in seconds)
#define BAT_PIN        34         //ADC input pin
#define MESURE_CYCLES  10         //Used to average ADC result
#define ADC_TRANS_CONST 3.3/4095*2.12

Adafruit_BMP280 bmp;
MAX44009 light;

const int oneWireBus = 4;  
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = {0xZZ, 0xZZ, 0xZZ, 0xZZ, 0xZZ, 0xZZ};

// Must match the receiver structure
typedef struct struct_message {
  float bmpTemp;
  float dsTemp;
  float bmpPress;
  float batVoltage;
  float luxLight;
} struct_message;

// Create a struct_message called myData
struct_message myData;

constexpr char WIFI_SSID[] = "receiver_SSID";

int32_t getWiFiChannel(const char *ssid) {
  if (int32_t n = WiFi.scanNetworks()) {
      for (uint8_t i=0; i<n; i++) {
          if (!strcmp(ssid, WiFi.SSID(i).c_str())) {
              return WiFi.channel(i);
          }
      }
  }
  return 0;
}

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  Wire.begin();
  Serial.println("Begin setup");
  adcAttachPin(BAT_PIN);  
  analogSetAttenuation(ADC_11db);
  sensors.begin();

  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    while (1);
  }

  if(!light.begin()){
    Serial.println("Could not find a valid MAX44009 sensor, check wiring!");
    while(1);
  }
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  int32_t channel = getWiFiChannel(WIFI_SSID);

  //Change Wi-Fi channel to match with the receiver network
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

 esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
 Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +
  " Seconds");

  int batVSum=0;
  delay(100);
  for (uint8_t i=0; i < MESURE_CYCLES; i++){
    batVSum = batVSum + analogRead(BAT_PIN);
    delay(10);
  }
  Serial.println( ADC_TRANS_CONST * (batVSum/10));
  sensors.requestTemperatures(); 
  myData.batVoltage = ( ADC_TRANS_CONST * (batVSum/10));
  myData.bmpTemp = bmp.readTemperature();
  myData.dsTemp = sensors.getTempCByIndex(0);
  myData.bmpPress = bmp.readPressure();
  myData.luxLight = light.get_lux();
  
  //Send data structure
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
   
  if (result == ESP_OK) Serial.println("Sent with success");
  else                  Serial.println("Error sending the data");
  
  //Put ESP in deep sleep
  esp_deep_sleep_start();
}
 
void loop() {
}
