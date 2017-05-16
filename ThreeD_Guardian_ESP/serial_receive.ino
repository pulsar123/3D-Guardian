// Receiving serial data from Arduino, transmitting it via MQTT:

void mqtt_send (int N, char * topic, int * i)
{
  if (*i + N >= BUF_SIZE - 1 || packet[*i] == '\0')
    return;

  if (first_send || strncmp(&packet[*i], &packet_old[*i], N) != 0)
    // Only publishing topic if the value has changed since the last time:
  {
    // Finding the first non-empty character index j:
    int j;
    for (j = *i; j < *i + N; j++)
    {
      if (packet[j] != ' ')
        break;
    }
    // The new length (without leading spaces):
    int Ns = N - j + *i;
    strncpy (str, &packet[j], Ns);
    str[Ns] = '\0';
    if (MQTT_on)
      client.publish(topic, str);
  }

  *i = *i + N;
  return;
}



void  serial_receive()
/* Here we receive serial messages from Arduino. Each message consists of one or more packets, each packet starting
   with the same 4 "magic" characters (MAGIC_AtoE). The packets can either be short (most frequent ones; only contain
   important status updates) or full length (status updates + full update from all sensors). We need to send all
   the received parameters from all packets to MQTT, but only for the parameters which changed their values since
   the last send.
*/
{
  int i;

  if (t - t0 > SERIAL_DT)
  {
    t0 = t;

    if (Serial.available() > 0)
    {
      // Reading the current string from the serial connection:
      while (Serial.available() > 0 && i_ser < BUF_SIZE - 1)
      {
        // Reading one character at a time from the serial terminal:
        s_char = Serial.read();
        // Placing the characters into the raw serial buffer.
        packet[i_ser] = s_char;

        /* Identifying packets - each starts with the magic substring "A->E" (which stands for Arduino->ESP8266)
           This is to minimize the chance of a false command, and to distinguish multiple commands in one serial receive.
        */
        if (i_ser >= 3 && strncmp(&packet[i_ser - 3], MAGIC_AtoE, 4) == 0)
          // We found the start of a packet.
        {
          if (first_packet == 0)
            // We have received at least one full packet at this point; it's time to process and send it to mqtt
          {
#ifdef DEBUG
            strncpy (debug, packet, i_ser + 1); debug[i_ser + 1] = '\0'; client.publish("TEST/0", debug);
#endif

            // i counts chars in the current packet:
            i = 0;
            mqtt_send(1, MQTT"/shut_down/status", &i);
            mqtt_send(1, MQTT"/clear_case/status", &i);
            mqtt_send(2, MQTT"/out/alarm", &i);
            mqtt_send(3, MQTT"/out/fan_speed", &i);

            if (i < i_ser - 3)
              // We got a full size packet:
            {
              mqtt_send(4, MQTT"/out/sensor1/min", &i);
              mqtt_send(4, MQTT"/out/sensor1/max", &i);
              mqtt_send(4, MQTT"/out/sensor1/cur", &i);
              mqtt_send(4, MQTT"/out/sensor1/alarm", &i);

              mqtt_send(4, MQTT"/out/sensor2/min", &i);
              mqtt_send(4, MQTT"/out/sensor2/max", &i);
              mqtt_send(4, MQTT"/out/sensor2/cur", &i);
              mqtt_send(4, MQTT"/out/sensor2/alarm", &i);

              mqtt_send(4, MQTT"/out/sensor3/min", &i);
              mqtt_send(4, MQTT"/out/sensor3/max", &i);
              mqtt_send(4, MQTT"/out/sensor3/cur", &i);
              mqtt_send(4, MQTT"/out/sensor3/alarm", &i);

              mqtt_send(4, MQTT"/out/sensor4/min", &i);
              mqtt_send(4, MQTT"/out/sensor4/max", &i);
              mqtt_send(4, MQTT"/out/sensor4/cur", &i);
              mqtt_send(4, MQTT"/out/sensor4/alarm", &i);
              
              mqtt_send(4, MQTT"/out/sensor5/min", &i);
              mqtt_send(4, MQTT"/out/sensor5/max", &i);
              mqtt_send(4, MQTT"/out/sensor5/cur", &i);
              mqtt_send(4, MQTT"/out/sensor5/alarm", &i);
              
              mqtt_send(4, MQTT"/out/sensor6/min", &i);
              mqtt_send(4, MQTT"/out/sensor6/max", &i);
              mqtt_send(4, MQTT"/out/sensor6/cur", &i);
              mqtt_send(4, MQTT"/out/sensor6/alarm", &i);
            }
            first_send = 0;
            // The length of the packet is i_ser-3:
            strncpy (packet_old, packet, i_ser - 3);
          }
          else
          {
            first_packet = 0;
          }
          i_ser = -1;
        }

        i_ser++;
      } // while i_ser

      t_serial = t;
    }
  }
  return;
}
