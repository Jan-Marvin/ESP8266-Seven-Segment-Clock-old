#include <SevenSegmentSR.h>
#include <EspMQTTClient.h>
#include <OneWire.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Timezone.h>

const char *ssid     = "xxxx";
const char *password = "xxxxxx";

EspMQTTClient client(
  "192.168.1.xxx",
  1883
);
const String main_topic = "clock";

String last_mqtt_msg;
int brightness = 100;
float temp;
bool display_on = 1;
float m = -0.2;
int clock_face = 1;
static unsigned long last_temp_time;

byte DS = D0; // serial data
byte SHCP = D1; // clock pin
byte STCP = D2; // latch pin

byte oneWire_data = D4;
const float temp_offset = -2.106; 
int temp_interval = 20000; // 20 sec


// Shift register
SevenSegmentSR<2> sr(DS,SHCP,STCP);

// NTP
WiFiUDP ntpUDP;

int GTMOffset = 0;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", GTMOffset*60*60, 60*60*1000);

TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     // Central European Summer Time
TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};       // Central European Standard Time
Timezone CE(CEST, CET);

// OneWire
OneWire oneWire(oneWire_data);

float get_temp_refresh() {
  oneWire.reset();
  sr.refresh_display();
  oneWire.skip();
  sr.refresh_display();
  oneWire.write(0xBE);
  sr.refresh_display();

  uint8_t data[9];
  for (uint8_t i = 0; i < 9; i++) {
    sr.refresh_display();
    data[i] = oneWire.read();
    sr.refresh_display();
  }
  int16_t rawTemperature = (((int16_t)data[1]) << 8) | data[0];
  sr.refresh_display();
  float temp = 0.0625 * rawTemperature;
  sr.refresh_display();
  oneWire.reset();
  sr.refresh_display();
  oneWire.skip();
  sr.refresh_display();
  oneWire.write(0x44, 0);
  sr.refresh_display();
  return temp + temp_offset;
}

float get_temp() {
  oneWire.reset();
  oneWire.skip();
  oneWire.write(0xBE);

  uint8_t data[9];
  for (uint8_t i = 0; i < 9; i++) {
    data[i] = oneWire.read();
  }
  int16_t rawTemperature = (((int16_t)data[1]) << 8) | data[0];
  float temp = 0.0625 * rawTemperature;
  oneWire.reset();
  oneWire.skip();
  oneWire.write(0x44, 0);
  return temp + temp_offset;
}

void onConnectionEstablished() {
  client.subscribe(main_topic + "/#", [](const String& topic, const String& payload) {
    String mqtt_msg = payload;
    if (mqtt_msg != "" && mqtt_msg != last_mqtt_msg) {
      if (topic == main_topic + "/brt") {
        brightness = mqtt_msg.toInt();
        sr.set_Brightness(m*brightness+20);
      }
      else if (topic == main_topic + "/display") {
        display_on = mqtt_msg.toInt();
        if (display_on) {
          uint8_t pinValues[] = { B0000000, B00000000 };
          sr.setAll(pinValues);
        }
      }
      else if (topic == main_topic + "/face") {
        clock_face = mqtt_msg.toInt();
      }
      else if (topic == main_topic + "/ref") {
        client.publish(main_topic + "/brt", String(brightness));
        client.publish(main_topic + "/display", String(display_on));
        if (millis() - last_temp_time >= 1000) {
          temp = get_temp();
          last_temp_time = millis();
        }
        client.publish(main_topic + "/temp", String(temp));
        client.publish(main_topic + "/face", String(clock_face));
        client.publish(main_topic + "/ver", "1.0.0");
      }
    }
    last_mqtt_msg = payload;
  });
}

void clock_face1() {
  String time_temp;  
  if (hour() <= 9) {
    time_temp += 0;
  }
  time_temp += hour();
  if (minute() <= 9) {
    time_temp += 0;
  }
  time_temp += minute();
  time_temp += " ";
  if (millis() - last_temp_time >= temp_interval) {
    if (display_on) {
      temp = get_temp_refresh();
    } 
    else {
      temp = get_temp();
    }
    last_temp_time = millis();
  }
  time_temp += String(temp,0);
  sr.set_chars(time_temp, 1);
}

void clock_face2() {
  String time_temp;  
  if (hour() <= 9) {
    time_temp += 0;
  }
  time_temp += hour();
  if (minute() <= 9) {
    time_temp += 0;
  }
  time_temp += minute();

  if (millis() - last_temp_time >= temp_interval) {
    if (display_on) {
      temp = get_temp_refresh();
    } 
    else {
      temp = get_temp();
    }
    last_temp_time = millis();
  }
  
  float t_temp = temp;
  String temp_nr = String(String(t_temp)[String(t_temp).length()-1]);
  if (temp_nr.toInt() >= 5) {
    t_temp += 0.1;
  }
  String final_temp = String(t_temp);
  final_temp.remove(final_temp.length()-3,1);
  final_temp.remove(final_temp.length()-1);
  
  time_temp += final_temp;
  sr.set_chars(time_temp, 2);
}

void clock_face3() {
  String time_temp;  
  if (hour() <= 9) {
    time_temp += 0;
  }
  time_temp += hour();
  if (minute() <= 9) {
    time_temp += 0;
  }
  time_temp += minute();
  time_temp += " ";
  if (millis() - last_temp_time >= temp_interval) {
    if (display_on) {
      temp = get_temp_refresh();
    } 
    else {
      temp = get_temp();
    }
    last_temp_time = millis();
  }
  
  float t_temp = temp;
  String temp_nr = String(String(t_temp)[String(t_temp).length()-1]);
  if (temp_nr.toInt() >= 5) {
    t_temp += 0.1;
  }
  String final_temp = String(t_temp);
  final_temp.remove(final_temp.length()-3,1);
  final_temp.remove(final_temp.length()-1);
  
  time_temp += final_temp;
  sr.set_chars(time_temp, 3);
}

void setup() {
  //Serial.begin(9600);
  WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }
  client.enableHTTPWebUpdater();
  oneWire.reset();
  oneWire.skip();
  oneWire.write(0x44, 0);
  timeClient.begin();
  delay ( 1000 );
  if (timeClient.update()){
     Serial.print ( "Adjust local clock" );
     unsigned long epoch = timeClient.getEpochTime();
     setTime(CE.toLocal(epoch));
  }else{
     Serial.print ( "NTP Update not WORK!!" );
  }
  temp = get_temp();
}

void loop() {
  client.loop();

  if (clock_face == 1) {
    clock_face1();
  }
  else if (clock_face == 2) {
    clock_face2();
  }
  else if (clock_face == 3) {
    clock_face3();
  }

  if (display_on) {
    sr.refresh_display();
  }
}
