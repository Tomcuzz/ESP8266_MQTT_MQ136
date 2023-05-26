#include "EspMQTTClient.h"

const char *wifi_ssid = "yyyyy"; // Wifi SSID
const char *wifi_pass = "xxxxx"; // Wifi Password
const char *mqtt_server = "192.168.1.2"; // MQTT Server host
const char *mqtt_user = "user"; // MQTT User
const char *mqtt_pass = "pass"; // Mqtt Password
const char *mqtt_id = "bathroom_h2s";
const char *mqtt_will_topic = "h2s_sensor/bathroom/status";
const char *mqtt_post_topic = "h2s_sensor/bathroom/value";
const char *mqtt_hass_topic = "homeassistant/sensor/bathroom_h2s/config";
const char *otp_pass = "otp_pass"; //OTP Password

const char *hass_config = "{\"name\":\"bathroom_h2s\", \"unique_id\":\"bathroom_h2s\", \"device\":{\"identifiers\":[\"bathroom_h2s\"], \"manufacturer\":\"tcousin\", \"model\":\"Gass Sensor V1\", \"name\":\"bathroom_h2s\"}, \"availability\":{\"topic\":\"h2s_sensor/bathroom/status\", \"payload_available\":\"online\", \"payload_not_available\":\"offline\"}, \"icon\":\"mdi:scent\", \"state_topic\":\"h2s_sensor/bathroom/value\", \"device_class\":\"gas\", \"unit_of_measurement\":\"\"}";


EspMQTTClient client(
  wifi_ssid,
  wifi_pass,
  mqtt_server,
  mqtt_user,
  mqtt_pass,
  mqtt_id
);

int i = 0;

void setup() {
  Serial.begin(115200);
  Serial.print("Starting...");

  client.setMaxPacketSize(512);
  client.enableDebuggingMessages();
  client.enableLastWillMessage(mqtt_will_topic, "offline", true);
  client.enableHTTPWebUpdater(otp_pass);
  client.enableOTA(otp_pass);
  client.enableMQTTPersistence();
}

void onConnectionEstablished() {
  Serial.print("MQTT Connected...");

  client.publish(mqtt_will_topic, "online", true);
  client.publish(mqtt_hass_topic, hass_config);
  
  client.subscribe("homeassistant/status", [](const String & payload) {
    client.publish(mqtt_hass_topic, hass_config);
  });
}

void loop() {
  client.loop();

  i++;
  if (i>100) {
    client.publish(mqtt_hass_topic, hass_config);
    i = 0;
  }

  float h = analogRead(A0);
  if (isnan(h))
  {
    Serial.println("Failed to read from MQ136 sensor!");
    return;
  }
  
  String val = String(h/1023*100);
  Serial.print("Gas Level: ");
  Serial.println(val);
  client.publish(mqtt_post_topic, val);
  
  delay(500);
  Serial.println("Waiting...");
  
  // thingspeak needs minimum 15 sec delay between updates.
  delay(1500);
}
