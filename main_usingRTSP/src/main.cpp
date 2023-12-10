#include "main.h"
#include "userdef.h"
#include "wifikeys.h"

/** Camera class */
OV2640 cam;
HTTPClient https;
WiFiClientSecure client;

/** 
 * Called once after reboot/powerup
 */
void setup()
{
	Serial.begin(115200);
	pinMode (LED_GPIO_NUM, OUTPUT);
	analogWrite(LED_GPIO_NUM, 0);
	// Start the serial connection
	Serial.println("\n\n##################################");
	Serial.printf("Internal Total heap %d, internal Free Heap %d\n", ESP.getHeapSize(), ESP.getFreeHeap());
	Serial.printf("SPIRam Total heap %d, SPIRam Free Heap %d\n", ESP.getPsramSize(), ESP.getFreePsram());
	Serial.printf("ChipRevision %d, Cpu Freq %d, SDK Version %s\n", ESP.getChipRevision(), ESP.getCpuFreqMHz(), ESP.getSdkVersion());
	Serial.printf("Flash Size %d, Flash Speed %d\n", ESP.getFlashChipSize(), ESP.getFlashChipSpeed());
	Serial.println("##################################\n\n");

	// Initialize the ESP32 CAM, here we use the AIthinker ESP32 CAM
	delay(100);
	cam.init(esp32cam_aithinker_config);
	delay(100);

	// Connect the WiFi
	//WiFi.mode(WIFI_STA);
	WiFi.begin(wifissid, wifipw);
	Serial.println("Connecting to Wi-Fi...");
	while (WiFi.status() != WL_CONNECTED)
	{
		delay(500);
	}
	// Print information how to contact the camera server
	IPAddress ip = WiFi.localIP();
	Serial.print("\nWiFi connected with IP: ");
	Serial.println(ip);

	String parameter = "?intIP=";
	parameter += ip.toString();
	parameter += "&&port=";
	parameter += CAMPORT;
	parameter += "&&macAddr=";
	uint8_t mac[6];
	char macAddr[19];
	esp_efuse_mac_get_default(mac);
	sprintf(macAddr, "%02X-%02X-%02X-%02X-%02X-%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	parameter += macAddr;
	String serverName = SERVERNAME;
	String URL = serverName + parameter;
	
	client.setInsecure();
	Serial.println("Sending Camera Information to Server...");
	https.begin(client, URL.c_str());
	https.addHeader("Content-Type", "application/x-www-form-unlencoded");
	while(https.GET() != 200){
		ESP.restart();
	} 
	Serial.println("Send Complete Successfully!\n");
	
#ifdef ENABLE_RTSPSERVER
	Serial.print("Stream Link: rtsp://");
	Serial.print(ip);
	Serial.print(":");
	Serial.print(CAMPORT);
	Serial.println("/mjpeg/1\n");
#endif
#ifdef ENABLE_WEBSERVER
	Serial.print("Browser Stream Link: http://");
	Serial.print(ip);
	Serial.println("\n");
	Serial.print("Browser Single Picture Link: http//");
	Serial.print(ip);
	Serial.println("/jpg\n");
#endif
#ifdef ENABLE_WEBSERVER
	// Initialize the HTTP web stream server
	initWebStream();
#endif

#ifdef ENABLE_RTSPSERVER
	// Initialize the RTSP stream server
	initRTSP();
#endif
	analogWrite(LED_GPIO_NUM, 50);
}

void loop()
{
}