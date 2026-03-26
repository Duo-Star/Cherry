#include <WiFi.h>
#include "Freenove_WS2812_Lib_for_ESP32.h"

#define LEDS_COUNT  8
#define LEDS_PIN	48
#define CHANNEL		0

Freenove_ESP32_WS2812 strip = Freenove_ESP32_WS2812(LEDS_COUNT, LEDS_PIN, CHANNEL, TYPE_GRB);

uint8_t m_color[5][3] = { {255, 0, 0}, {0, 255, 0}, {0, 0, 255}, {255, 255, 255}, {0, 0, 0} };
int delayval = 50;

const char*ssid = "123";
const char*password = "123456789";

void setup(){ 
  strip.begin();
	strip.setBrightness(10);	
	Serial.begin(115200);
	Serial.print("wifi开始连接");
	WiFi.begin(ssid,password);
	while(WiFi.status() != WL_CONNECTED){
		delay(1000);
		Serial.print(".");
		}
}

void loop(){
	if(WiFi.status() == WL_CONNECTED){
	  Serial.println("连接成功");
	  Serial.print("IP地址是:");
	  Serial.println(WiFi.localIP());
	  delay(2000);
	  for (int j = 0; j < 5; j++) {
		for (int i = 0; i < LEDS_COUNT; i++) {
			strip.setLedColorData(i, m_color[j][0], m_color[j][1], m_color[j][2]);
			strip.show();
			delay(delayval);
		}
		delay(50);
	}

	}

}