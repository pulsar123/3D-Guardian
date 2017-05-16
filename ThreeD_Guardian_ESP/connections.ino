void connections()
// Establishing and re-establishing WiFi and MQTT connections
{
  // Taking care of WiFi connection (trying to reconnect when lost)
  if (WiFi_on == 0 && WiFi.status() == WL_CONNECTED)
  {
    WiFi_on = 1;
    // Turning the LED1 on when WiFi is connected:
    led1 = HIGH;
    digitalWrite(LED1, led1);
  }

  if (WiFi_on == 1 && WiFi.status() != WL_CONNECTED)
    // If we just lost WiFi (will have to reconnect MQTT as well):
  {
    WiFi_on = 0;
    MQTT_on = 0;
    mqtt_init = 1;
    WiFi.begin(ssid, password);
    led1 = LOW;
    digitalWrite(LED1, led1);
  }

  if (WiFi_on == 1 && !client.connected())
    // If we just lost MQTT connection (WiFi is still on), or not connected yet after WiFi reconnection:
  {
    MQTT_on = 0;
    mqtt_init = 1;
  }

  // Taking care of MQTT connection (reconnecting when lost and only if WiFi is connected)
  if (WiFi_on == 1 && MQTT_on == 0)
  {
    if (client.connect(MQTT))
    {
      client.subscribe(MQTT"/clear_case/switch");
      client.subscribe(MQTT"/shut_down/switch");
      client.subscribe("openhab/start");
      MQTT_on = 1;
    }
  }
  return;
}
