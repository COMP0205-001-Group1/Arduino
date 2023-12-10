//#define UPLOADINTERVAL 10 // ms
//#define PREVMILLIS 0 // ms, Initial Value;
#define WIFISSID "<WiFiSSID>"
#define WIFIPW "<WiFiPW>"
#define SERVERNAME "<Domain>"
#define SERVERPORT <Port>
#define SERVERPATH "/upload.php?macAddr="
#define CAMERA_MODEL_AI_THINKER
#include "esp_camera.h"
#include <WiFiClientSecure.h>
#include "camera_pins.h"

const char* ssid = WIFISSID;
const char* password = WIFIPW;
uint8_t mac[6];
char macAddr[19];
String serverName = SERVERNAME;
String serverPath = SERVERPATH;
const int serverPort = SERVERPORT;
//const short timerInterval = UPLOADINTERVAL;
//unsigned short previousMillis = PREVMILLIS;
WiFiClientSecure client;

//void startCameraServer();
//void setupLedFlash(int pin);
void sendIMG();

void setup() {
  String getBody;
  Serial.begin(115200);
  Serial.setDebugOutput(true);  
  esp_efuse_mac_get_default(mac);
  sprintf(macAddr, "%02X-%02X-%02X-%02X-%02X-%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  serverPath += macAddr;
  Serial.println();

  WiFi.mode(WIFI_STA);
  Serial.print("Connectiong to Wi-Fi (SSID: ");
  Serial.print(ssid);
  Serial.println(")");
  WiFi.begin(ssid, password);
  WiFi.setSleep(false);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("WiFi connected. Module IP Address: ");
  Serial.println(WiFi.localIP());
  pinMode (LED_GPIO_NUM, OUTPUT);

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
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG; // for streaming
  

  if(psramFound()){
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_CIF;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    delay(1000);
    ESP.restart();
  }

  Serial.println("Connecting to server: " + serverName);
  client.setInsecure(); //skip certificate validation
  if (client.connect(serverName.c_str(), serverPort)) {
    Serial.println("Connection successful!");    
    digitalWrite(LED_GPIO_NUM, HIGH);
    sendIMG();
  } else {
    getBody = "Connection to " + serverName +  " failed.";
    Serial.println(getBody);
  }
}

void loop() {
  //unsigned long currentMillis = millis();
  //if (currentMillis - previousMillis >= timerInterval) {
  digitalWrite(LED_GPIO_NUM, LOW);
  sendIMG();
    //previousMillis = currentMillis;
  //}
}

void sendIMG() {
  String getAll;
  String getBody;

  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get(); // 사진찍기
  if(!fb) {
    delay(100);
    ESP.restart();
  }
  
  String head = "--RandomNerdTutorials\r\nContent-Disposition: form-data; name=\"imageFile\"; filename=\"esp32-cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
  String tail = "\r\n--RandomNerdTutorials--\r\n";

  uint32_t imageLen = fb->len;
  uint32_t extraLen = head.length() + tail.length();
  uint32_t totalLen = imageLen + extraLen;

  // Post Request 날리는 부분 (File 정보 제시)
  client.println("POST " + serverPath + " HTTP/1.1");
  client.println("Host: " + serverName);
  client.println("Content-Length: " + String(totalLen));
  client.println("Content-Type: multipart/form-data; boundary=RandomNerdTutorials");
  client.println();
  client.print(head);

  // 사진정보 전송
  uint8_t *fbBuf = fb->buf;
  size_t fbLen = fb->len;
  for (size_t n=0; n<fbLen; n=n+1024) {
    if (n+1024 < fbLen) {
      client.write(fbBuf, 1024);
      fbBuf += 1024;
    }
    else if (fbLen%1024>0) {
      size_t remainder = fbLen%1024;
      client.write(fbBuf, remainder);
    }
  }   
  client.print(tail); // 전송 끝
  esp_camera_fb_return(fb);
  
  //int timoutTimer = 10000;
  //long startTimer = millis();
  //boolean state = false;
  
  //while ((startTimer + timoutTimer) > millis()) {
  // Serial.print(".");
  //  delay(100);      
  //  while (client.available()) {
  //    char c = client.read();
  //    if (c == '\n') {
  //      if (getAll.length()==0) { state=true; }
  //      getAll = "";
  //    }
  //    else if (c != '\r') { getAll += String(c); }
  //    if (state==true) { getBody += String(c); }
  //    startTimer = millis();
  //  }
  //  if (getBody.length()>0) { break; }
  //}
}