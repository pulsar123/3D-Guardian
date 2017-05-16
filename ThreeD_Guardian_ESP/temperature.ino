void temperature()
// Measuring the SSR's temperature, and disabling it if it's too hot
{
  if (t - t_a0 > DT_TH)
  {
    float Raw = (float)analogRead(TH_PIN);
    float R;
    // Thermistor resistance (assuming it is used with a pullup resistor), in Ohms:
    R = R_PULL * (Raw - A0_LOW) / (A0_HIGH - Raw);
    // Correcting for the internal pulldown resistor on A0 pin, with R_INETRNAL value:
    R = 1.0 / (1.0 / R - 1.0 / R_INTERNAL);
    // Temperature (Celsius):
    float T = 1.0 / (TH_A + TH_B * log(R)) - 273.15;
    sum_T = sum_T + T;
    i_T++;
    t_a0 = t;
    if (i_T == N_T)
    {
      T_avr = sum_T / N_T;
      int T1 = (int)(T_avr * 10.0 + 0.5);
      T_int = T1 / 10;
      i_T = 0;
      sum_T = 0.0;
      if (bad_temp == 0 && T_avr > T_MAX)
        // We exceeded the critical SSR temperature; disabling SSR until the controller is rebooted:
      {
        bad_temp = 1;
        // Send the SHUTDOWN command to Arduino via serial connection:
        Serial.print(MAGIC_EtoA"S");
        if (MQTT_on)
          client.publish(MQTT"/out/alarm", "1");
      }

      if (MQTT_on)
      {
        i_mqtt_T++;
        // Sending current temperature every 30 seconds:
        if (i_mqtt_T == 30)
        {
          // We don't care about negative temperature values (usually if thermistor is disconnected):
          if (T_int < 0)
            T_int = 0;
          i_mqtt_T = 0;
          sprintf(str, "%d", T_int);
          client.publish(MQTT"/SSR_temp", str);
        }
      }

#ifdef DEBUG
      // Transmit the A0_LOW and A0_HIGH parameters (raw A0 pin values when it is pulled down and up, respectively)
//      client.publish("TEST/1", itoa(analogRead(TH_PIN)));
#endif
    }
  }

  return;
}
