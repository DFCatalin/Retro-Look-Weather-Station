#include "decodes.h"
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <esp_now.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include "html_page.h"

#define I2C_SDA 33
#define I2C_SCL 32

const char* ssid     = "network_name";
const char* password = "network_password";

void printTextDelay(String a, int displayTime, bool exterior = false);

WebServer server(80);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

String formattedDate;
String dayStamp;
String timeStamp;
String timeStampHTML;
float extDSTemp = 0.0;
float extBMPTemp = 0.0;
float extBMPPres = 0.0;
float extbatVoltage = 0.0;
float tmp =0.0;
float pres =0.0;
float extLux = 0.0;

Adafruit_BMP085 bmp;

// HV5812 inputs
int latchPin = 21;
int clockPin = 23;
int dataPin = 22;

 byte leds = 0;
 int counter =0;
 unsigned long startProgTime;
 unsigned long currentMillis;
 bool night_dimming = false;
 bool clk_format = false;

typedef struct struct_message {
  float bmpTemp;
  float dsTemp;
  float bmpPress;
  float batVoltage;
  float luxLight;
} struct_message;

struct_message myData;

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
    
  extDSTemp = myData.dsTemp;
  extBMPTemp = myData.bmpTemp;
  extBMPPres = myData.bmpPress;
  extbatVoltage = myData.batVoltage;
  extLux = myData.luxLight;  
}

void setup() 
{
  Serial.begin(115200);
  Serial.println("Start Setup");
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);  
  pinMode(clockPin, OUTPUT);
  Wire.begin(I2C_SDA, I2C_SCL);
  if (!bmp.begin()) {
  while (1) {}
  }
  Serial.println("WiFi setup begin...");
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  CSerial.print("Station IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Wi-Fi Channel: ");
  Serial.println(WiFi.channel());

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
   esp_now_register_recv_cb(OnDataRecv);

   Serial.println("WiFi setup end.");

   Serial.println("NTP server setup start...");
   timeClient.begin();
   timeClient.setTimeOffset(7200);
   Serial.println("NTP server setup end");

  server.on("/", handleRoot);
  server.on("/readPRES", handlePRES);
  server.on("/readTMP", handleTMP);
  server.on("/readTime", handleTIME);
  server.on("/readPRESE", handlePRESE);
  server.on("/readTMPDS", handleTMPDS);
  server.on("/readTMPBM", handleTMPBM);
  server.on("/readBAT", handleBAT);
  server.on("/readLUX", handleLUX);
  
  //Start server
  server.begin();
  
  startProgTime = millis();
  String AP_IP_ADDR = WiFi.localIP().toString();
  printTextDelay(AP_IP_ADDR,5000);
}

void loop() 
{    
      server.handleClient();
      
      while(!timeClient.update()) {
      timeClient.forceUpdate();
      }

      formattedDate = timeClient.getFormattedDate();
      int splitT = formattedDate.indexOf("T");
      dayStamp = formattedDate.substring(0, splitT);
      timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
      timeStampHTML = timeStamp;
      if(clk_format){ timeStamp.replace(":"," "); clk_format=!clk_format;}
      else          { timeStamp.replace(":","-"); clk_format=!clk_format;}

     
      uint8_t hour = (timeStamp[0]-48)*10 + (timeStamp[1]-48);
 
      if(hour >= 20 || hour <=7 ) night_dimming = true;
      else                        night_dimming = false;
          
      startProgTime = millis();
      printTextDelay(timeStamp,1000);
     
      if(timeStamp[3]=='0' && timeStamp[4]=='0' && timeStamp[6]=='0' && timeStamp[7]=='0'){
        startProgTime = millis();
        printTextDelay(dayStamp,2000);
      }

       if(timeStamp[4]=='0' && timeStamp[6]=='0' && timeStamp[7]=='0'){
        tmp = bmp.readTemperature();
        String tmp_bmp = "ti "+String(tmp, 2)+"xC";
        pres = bmp.readPressure()/100;
        String pres_bmp = "Pr "+String(pres,1)+"hP";
        
        String tmpDS_ext;
        if(extDSTemp != 0.0) tmpDS_ext  = "tD "+String(extDSTemp, 2)+"xC";
        else                 tmpDS_ext  = "tD "+String("Error");

        String tmpBMP_ext;
        if(extBMPTemp != 0.0)   tmpBMP_ext  = "tB "+String(extBMPTemp, 2)+"xC";
        else                   tmpBMP_ext  = "tB "+String("Error");

        String pres_ext;
        if(extBMPPres != 0.0)  pres_ext   = "Pr "+String(extBMPPres/100, 1)+"hP";
        else                   pres_ext   = "Pr "+String("Error");

        String bat_voltage;
        if(extbatVoltage != 0.0)  bat_voltage   = "BAT-" + String(extbatVoltage);
        else                      bat_voltage   = "BAT-" +String("Error");

        String lux_value;
        if(extLux != 0.0)         lux_value  = "INT " + String(extLux,1) +"l";
        else                      lux_value  = "INT " +String("Error");
        
        startProgTime = millis();
        printTextDelay(tmp_bmp,3000);
        startProgTime = millis();
        printTextDelay(pres_bmp,3000);
        startProgTime = millis();
        printTextDelay(tmpDS_ext,3000, true);
        startProgTime = millis();
        printTextDelay(tmpBMP_ext,3000, true);
        startProgTime = millis();
        printTextDelay(pres_ext,3000, true);
        startProgTime = millis();
        printTextDelay(bat_voltage,3000, true);
        startProgTime = millis();
        printTextDelay(lux_value,3000, true);
        startProgTime = millis();
      }


}

void printTextDelay(String a, int displayTime, bool exterior){
      currentMillis = millis();
    while(currentMillis - startProgTime <= displayTime){
      printText(a, exterior);
      clearShiftRegister();
      if(night_dimming) delay(1);
      currentMillis = millis();
    }
  }
void printText(String a, bool exterior){
   int testLength = 0;
   for(int i=0; i< a.length(); i++){
    if(a[i]=='m')  testLength++; 
    if(a[i]>='a' && a[i]<='z' || a[i]>='A' && a[i]<='Z' || a[i]>='0' && a[i]<='9' || a[i] ==' ' 
       || a[i] == '-' || a[i] == '=' ||  a[i] == '°' )
       testLength++;
   }
 
   int startIndex = (10 - testLength) / 2;

   uint8_t negOffset = 0;
   uint8_t posOffset = 0;
  
   for(int i=0; i< a.length(); i++){               
    bool nextComma;
    bool nextDot;
    if(a[i+1] == ',') nextComma=true; 
    else              nextComma=false;
    
    if(a[i+1] == '.') nextDot=true; 
    else              nextDot=false;  

    if(a[i] == 'm' || a[i] == 'M'){
     displayChar(i+startIndex-negOffset+posOffset, 'n', nextComma, nextDot);
     posOffset++;
     displayChar(i+startIndex-negOffset+posOffset, 'n', nextComma, nextDot);
    }
   
    displayChar(i+startIndex-negOffset+posOffset, a[i], nextComma, nextDot);
    if(nextComma || nextDot)  {i+=1; negOffset++;}
    }

    //display E sign, to mark exterior values
    if(exterior) displayChar(10, '&', false, false ); 
    //display M to show that Li-Ion is low on battery
    if(extbatVoltage < BAT_MIN_VOL && extbatVoltage!=0) displayChar(10, '%', false, false );   
  }
  

void displayChar(int digitNumber, char charValue, bool commaAfter, bool dotAfter){
  //Grid selection
  byte lastShfitByte, secondShiftByte, firstShiftByte;
  if(digitNumber < 8) {lastShfitByte = (128 >> digitNumber); secondShiftByte = 0;}
  else if(digitNumber >=8 && digitNumber <= 10) {secondShiftByte = (128 >> (digitNumber - 8)); lastShfitByte = 0;}


  //Char decode
  switch (charValue){
    //Numbers and special chars
    case '1' : secondShiftByte += (N1_hex >> 3); firstShiftByte = (N1_hex << 5); break;
    case '2' : secondShiftByte += (N2_hex >> 3); firstShiftByte = (N2_hex << 5); break;
    case '3' : secondShiftByte += (N3_hex >> 3); firstShiftByte = (N3_hex << 5); break;
    case '4' : secondShiftByte += (N4_hex >> 3); firstShiftByte = (N4_hex << 5); break;
    case '5' : secondShiftByte += (N5_hex >> 3); firstShiftByte = (N5_hex << 5); break;
    case '6' : secondShiftByte += (N6_hex >> 3); firstShiftByte = (N6_hex << 5); break;
    case '7' : secondShiftByte += (N7_hex >> 3); firstShiftByte = (N7_hex << 5); break;
    case '8' : secondShiftByte += (N8_hex >> 3); firstShiftByte = (N8_hex << 5); break;
    case '9' : secondShiftByte += (N9_hex >> 3); firstShiftByte = (N9_hex << 5); break;
    case '0' : secondShiftByte += (N0_hex >> 3); firstShiftByte = (N0_hex << 5); break;
    case '=' : secondShiftByte += (EQ_hex >> 3); firstShiftByte = (EQ_hex << 5); break;
    case '-' : secondShiftByte += (EQ_hex >> 3); firstShiftByte = (EQ_hex << 5); break;
    case ' ' : secondShiftByte += (NON_hex >> 3); firstShiftByte = (NON_hex << 5); break;
    case 'x' : secondShiftByte += (DEG_hex >> 3); firstShiftByte = (DEG_hex << 5); break;
    case '&' : secondShiftByte += (EXT_hex >> 3); firstShiftByte = (EXT_hex << 5); break;
    case '%' : secondShiftByte += (ERR_hex >> 3); firstShiftByte = (ERR_hex << 5); break;

    //Letters
    case 'A' : case 'a' : secondShiftByte += (A_hex >> 3); firstShiftByte = (A_hex << 5); break;
    case 'B' :            secondShiftByte += (B_hex >> 3); firstShiftByte = (B_hex << 5); break;
    case 'b' :            secondShiftByte += (b_hex >> 3); firstShiftByte = (b_hex << 5); break;
    case 'C' : case 'c' : secondShiftByte += (C_hex >> 3); firstShiftByte = (C_hex << 5); break;
    case 'D' : case 'd' : secondShiftByte += (D_hex >> 3); firstShiftByte = (D_hex << 5); break;
    case 'E' : case 'e' : secondShiftByte += (E_hex >> 3); firstShiftByte = (E_hex << 5); break;
    case 'F' : case 'f' : secondShiftByte += (F_hex >> 3); firstShiftByte = (F_hex << 5); break;
    case 'G' : case 'g' : secondShiftByte += (G_hex >> 3); firstShiftByte = (G_hex << 5); break;
    case 'H' :            secondShiftByte += (H_hex >> 3); firstShiftByte = (H_hex << 5); break;
    case 'h' :            secondShiftByte += (h_hex >> 3); firstShiftByte = (h_hex << 5); break;
    case 'I' : case 'i' : secondShiftByte += (I_hex >> 3); firstShiftByte = (I_hex << 5); break;
    case 'J' : case 'j' : secondShiftByte += (J_hex >> 3); firstShiftByte = (J_hex << 5); break;
    case 'L' : case 'l' : secondShiftByte += (L_hex >> 3); firstShiftByte = (L_hex << 5); break;
    case 'N' : case 'n' : secondShiftByte += (n_hex >> 3); firstShiftByte = (n_hex << 5); break;
    case 'O' :            secondShiftByte += (O_hex >> 3); firstShiftByte = (O_hex << 5); break;
    case 'o' :            secondShiftByte += (o_hex >> 3); firstShiftByte = (o_hex << 5); break;
    case 'P' : case 'p' : secondShiftByte += (P_hex >> 3); firstShiftByte = (P_hex << 5); break;
    case 'R' : case 'r' : secondShiftByte += (r_hex >> 3); firstShiftByte = (r_hex << 5); break;
    case 'S' : case 's' : secondShiftByte += (S_hex >> 3); firstShiftByte = (S_hex << 5); break;
    case 'T' : case 't' : secondShiftByte += (T_hex >> 3); firstShiftByte = (T_hex << 5); break;
    case 'U' :            secondShiftByte += (U_hex >> 3); firstShiftByte = (U_hex << 5); break;
    case 'u' :            secondShiftByte += (u_hex >> 3); firstShiftByte = (u_hex << 5); break;
    case 'Y' : case 'y' : secondShiftByte += (y_hex >> 3); firstShiftByte = (y_hex << 5); break;

    //default - print "_"
    default  : secondShiftByte += (    8 >> 3);  firstShiftByte = (    8 << 5); break;
    }

  if(commaAfter)  firstShiftByte  += 16;
  if(dotAfter)    secondShiftByte += 16;
  updateShiftRegister(firstShiftByte, secondShiftByte, lastShfitByte);
} 
void updateShiftRegister(byte first, byte second, byte last){
   digitalWrite(latchPin, LOW);
   shiftOut(dataPin, clockPin, LSBFIRST, first);
   shiftOut(dataPin, clockPin, LSBFIRST, second);
   shiftOut(dataPin, clockPin, LSBFIRST, last);
   digitalWrite(latchPin, HIGH);
}
void clearShiftRegister()
{
   digitalWrite(latchPin, LOW);
   shiftOut(dataPin, clockPin, LSBFIRST, 0);
   shiftOut(dataPin, clockPin, LSBFIRST, 0);
   shiftOut(dataPin, clockPin, LSBFIRST, 0);
   digitalWrite(latchPin, HIGH);
}

void handleRoot() {
 String s = MAIN_page; //Read HTML contents
 server.send(200, "text/html", s); //Send web page
}
void handlePRES() {
 float inPres = bmp.readPressure()/100;  
 String bmpPRES = String(inPres ,2);         
 server.send(200, "text/plane", bmpPRES);
}
void handleTMP() {
 String bmpTemp = String(bmp.readTemperature()); 
 server.send(200, "text/plane", bmpTemp);
}
void handleTIME(){
 server.send(200, "text/plane", dayStamp + " " + timeStampHTML);
}
void handlePRESE(){
 server.send(200, "text/plane", String(extBMPPres/100, 2));
}
void handleTMPDS(){
 server.send(200, "text/plane", String(extDSTemp));
}
void handleTMPBM(){
 server.send(200, "text/plane", String(extBMPTemp));
}
void handleBAT(){
 server.send(200, "text/plane", String(extbatVoltage));
}
void handleLUX(){
 server.send(200, "text/plane", String(extLux));
}
