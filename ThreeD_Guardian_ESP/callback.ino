void callback(char* topic, byte* payload, unsigned int length)
// The function processing incoming MQTT commands.
{
  // Clear Case command:
  if (strcmp(topic, MQTT"/clear_case/switch") == 0)
  {
    if ((char)payload[0] == '1')
    {
      // Send the command to Arduino via serial connection:
      Serial.print(MAGIC_EtoA"C");
    }
  }

  // Shut down command. The signal can come either from MQTT, or from the button interrupt function (modifying panic variable)
  if (strcmp(topic, MQTT"/shut_down/switch") == 0)
  {
    if ((char)payload[0] == '1')
    {
      // Send the command to Arduino via serial connection:
      Serial.print(MAGIC_EtoA"S");
    }
  }

  // Training/Guarding mode toggle:
  if (strcmp(topic, MQTT"/training/switch") == 0)
  {
    if ((char)payload[0] == '1')
    {
      // Send the command to Arduino via serial connection:
      Serial.print(MAGIC_EtoA"M");
    }
  }
  // Refresh everything command:
  if (strcmp(topic, "openhab/start") == 0)
  {
    if ((char)payload[0] == '1')
      first_send = 1;
  }
  return;
}

