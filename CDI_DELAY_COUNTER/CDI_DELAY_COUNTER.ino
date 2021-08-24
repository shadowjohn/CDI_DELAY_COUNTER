// 把 NSR 抓到凸台脈衝訊號(4P 裡的  青/黃)，到發現點火發生(4P 裡的 黑/黃) 的 轉速、fire_delay、角度，推到 mqtt 送去網頁看
// Date : 2021-08-22
// Author FB @田峻墉
// Authro 羽山 (https://3wa.tw)

#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
//用來偵測 D1 腳點火訊號 (接 黑/黃) 
//用來偵測 D3 腳抓脈衝凸台的訊號 (接 青/黃)

const int FIRE_SIG = D1; //偵測點火
const int TO_SIG = D3; //凸台，脈衝
volatile unsigned long C=0, rpm=0, RPM_DELAY=0, FIRE_DELAY=0, C_old=0;
volatile unsigned long COUNTS = 0; //每隔 100ms 送一筆資料到 mqtt
volatile float FIRE_DEGREE;
/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "john"
#define WLAN_PASS       "0919566444"

#define AIO_SERVER      "3wa.tw"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "john"
#define AIO_KEY         "123123123"
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish mqtt_mycdicounter = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/mycdicounter");

void ICACHE_RAM_ATTR TO_SIG_FUNC() {  
  //凸台起始偵測、rpm 計算  
  //只要是Rising就是凸台剛出現
  C=micros();
  RPM_DELAY=C-C_old;
  rpm = 60000000UL / RPM_DELAY;
  C_old = C;
}

void ICACHE_RAM_ATTR FIRE_SIG_FUNC() {  
  //點火出現，所以用點火的時間，減去凸台時間，得到 FIRE_DELAY
  FIRE_DELAY = micros()-C;
  FIRE_DEGREE = float(FIRE_DELAY) / float(RPM_DELAY) * 360;
}
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);    
  //讀入點火
  pinMode(FIRE_SIG, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(FIRE_SIG), FIRE_SIG_FUNC, RISING);     
  
  //讀入脈衝
  pinMode(TO_SIG, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(TO_SIG), TO_SIG_FUNC, RISING);     



  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); 
  Serial.println(WiFi.localIP());
  COUNTS = micros();
}

void loop() {  
  //每 100ms 送出一次結果就好~
  if(micros() - COUNTS < 100) return;
  //if(rpm==0) return;
  //各種斷線自動重連
  if (WiFi.status() != WL_CONNECTED) {    
    Serial.print("\nReconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.begin(WLAN_SSID, WLAN_PASS);         
  }      
  if(! mqtt.ping()) {
    mqtt.disconnect();
    MQTT_connect();
  }    
  COUNTS = micros();
  
  //送出資料
  String data = String(rpm)+','+String(FIRE_DELAY)+','+String(FIRE_DEGREE);
  char charBuf[30];
  data.toCharArray(charBuf, 30);
  mqtt_mycdicounter.publish(charBuf);
}
