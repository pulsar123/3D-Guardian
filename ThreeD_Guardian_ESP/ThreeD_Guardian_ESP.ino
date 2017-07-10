/*
  ESP8266 WiFi module for the Arduino-based 3D Guardian (safety device for 3D printers).

  Documentation:  http://pulsar124.wikia.com/wiki/3D_Guardian

  Based on a ESP8266 example from PubSubClient library (https://github.com/knolleary/pubsubclient)

  Using non-blocking functions for WiFi and MQTT connections. The only functionalities which work without WiFi and/or MQTT
  are the thermal protection of the Solid State Relay, and the Panick button. If Panick button is pressed,
  a serial command to shut down the printer will be sent to Arduino over the serial interface.

  To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select "NodeMCU 0.9" in "Tools -> Board"

*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "config.h"

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void setup()
{
  pinMode(LED1, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  // Panic button is handled by an interrupt function button():
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonIntr, FALLING); // FALLING as we only care about the initial button press moment
  led1 = LOW;
  digitalWrite(LED1, led1); //WiFi indicator (external LED)
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  t = millis();
  t0 = t;
  t_a0 = t;
  t_serial = t;
//  t_beacon = t;
  t_led1 = t;
  first_send = 1;
  first_packet = 1;
  i_ser = 0;
  s_char = '\0';
  sum_T = 0.0;
  i_T = 0;
  T_avr = 0;
  WiFi_on = 0;
  MQTT_on = 0;
  mqtt_init = 1;
  i_mqtt_T = 0;
  panic = 0;
  timeout = 0;
  blink_state = 0;
  first_temp = 1;
  no_cable = 0;
  nocable_step = 0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void loop()
{
  connections();

  t = millis();

  // Checking the SSR temperature, and send it to Arduino over serial:
  temperature();

  // Receiving serial data from Arduino, transmitting it via MQTT:
  serial_receive();

  // Stuff related to the panic button:
  button();

  // MQTT communications:
  mqtt();

  // LED warning:
  LED();

}
