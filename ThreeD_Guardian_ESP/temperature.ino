void temperature()
// Measuring the SSR's temperature, and disabling it if it's too hot
{
  if (millis() - t_a0 > DT_TH)
  {
    float Raw = (float)analogRead(TH_PIN);
    // Detecting a disconnected cable to the SSR unit at boo time:
    if (first_temp && Raw > 800)
      no_cable = 1;
    first_temp = 0;
    float R;
    // Thermistor resistance (assuming it is used with a pullup resistor), in Ohms:
    R = R_PULL * (Raw - A0_LOW) / (A0_HIGH - Raw);
    // Correcting for the internal pulldown resistor on A0 pin, with R_INTERNAL value:
    R = 1.0 / (1.0 / R - 1.0 / R_INTERNAL);
    // Temperature (Celsius):
    float T = 1.0 / (TH_A + TH_B * log(R)) - 273.15;
    sum_T = sum_T + T;
    i_T++;
    t_a0 = millis();
    if (i_T == N_T)
    {
      T_avr = sum_T / N_T;
      T_int = (int)(T_avr + 0.5);
      i_T = 0;
      sum_T = 0.0;

      if (T_int < 0)
        T_int = 0;
      if (T_int > 999)
        T_int = 999;
      sprintf(str, MAGIC_EtoA"T%3d", T_int);
      // Sending the SSR temperature value to Arduino every second:
      Serial.print(str);


#ifdef DEBUG
      // Transmit the A0_LOW and A0_HIGH parameters (raw A0 pin values when it is pulled down and up, respectively)
      client.publish("TEST/1", itoa(analogRead(TH_PIN), debug, 10));
#endif
    }
  }

  return;
}
