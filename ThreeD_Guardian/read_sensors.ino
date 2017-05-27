void read_sensors ()
/*
  Reading inputs from all sensors, and updating all limiting values
*/
{
  int x;
  long int y;
  float T;
  int V1_raw, V2_raw, V_crit_raw, Vmax_raw, I_raw;

  // Going over all sensors in a loop, and reading the data if its due
  for (byte i = 0; i < N_SENSORS; i++)
  {
    // Only reading each sensor once every SENS_DT ms. The *.on flag is used to establish an initial "warmup" delay for specific senors
    if (sensor[i].on && g.t - sensor[i].t0 > SENS_DT)
    {
      sensor[i].t0 = g.t;

      // Sensor type=4 is not real - these are SSR temperature values (C) received from ESP via serial every second
      if (sensor[i].type != 4)
      {
        if (sensor[i].type == 3)
          // First voltage measurement, for resistance sensor
          V1_raw = analogRead(sensor[i].pin2);

        // For resistance sensor this is the raw current measurement:
        x = analogRead(sensor[i].pin);

        if (sensor[i].type == 1 || sensor[i].type == 2)
          // Inverting the raw thermistor data to have it behave like other sensors (higher value means higher temperature -  more dangerous)
          x = 1023 - x;
        else if (sensor[i].type == 3)
          // Second voltage measurement, for resistance sensor
          V2_raw = analogRead(sensor[i].pin2);

        if (sensor[i].type == 3)
          // Computing inverse resistance based on two raw voltage (V1_raw, V2_raw) and one current (x) measurements
          // For details see http://pulsar124.wikia.com/wiki/3D_Guardian#Resistance_sensor
        {
          // If the resistance sensor hasn't been trained yet, we store the raw voltage as teh sensor value - to be used for zero voltage calibration from Menu:
          if (sensor[i].train.zero < 0)
          {
            x = (int)((V1_raw + V2_raw) / 2.0 + 0.5);
          }
          else
          {
            // Fixing the voltage offset:
            V1_raw = V1_raw - sensor[i].train.zero;
            V2_raw = V2_raw - sensor[i].train.zero;
            if (V1_raw < sensor[i].V_crit_raw || V2_raw < sensor[i].V_crit_raw)
              // Either first or second voltage measurement is bad (<V_crit), so skipping this resistance measurement
              continue;
            if (V1_raw > V2_raw)
              Vmax_raw = V1_raw;
            else
              Vmax_raw = V2_raw;
            I_raw = x - ZERO_CURRENT_RAW;
            // Inverse resistance (1/Ohms) normalized by the expected bed resistance (R_BED), converted to raw units (by *512)
            x = (int)(512 * R_BED * sensor[i].divider / sensor[i].scaler * (float)I_raw / (float)Vmax_raw + 0.5);
            // Resistance was measured, so warnings/alarms are now allowed for resistance sensor:
            g.resistance = 1;
          }
#ifdef DEBUG
          Serial.print("R ");
          Serial.print(V1_raw);
          Serial.print(" ");
          Serial.print(I_raw);
          Serial.print(" ");
          Serial.print(V2_raw);
          Serial.print(" ");
          Serial.print(Vmax_raw);
          Serial.print(" ");
          Serial.println(x);
#endif
        }

        sensor[i].sum = sensor[i].sum + x;
      } // if sensor type != 4

      sensor[i].i++;

      // Once every SENS_N measurements (normally every SNES_N*SENS_DT ms) we recompute the averaged current value for this sensor:
      if (sensor[i].i == SENS_N)
      {
        if (sensor[i].type == 4 && g.SSR_temp == 1)
        {
          sensor[i].avr = g.T_SSR;
          if (g.T_SSR > CRAZY_TEMP)
          {
            g.alarm = ALARM;
            g.bad_sensor = i;
            alarm_actions();
            return;
          }
        }
        else
        {
          float M = (float)sensor[i].sum / (float)SENS_N;

          // This is a simplified approach - assumes that all the thermistors we use are identical, and that all the analogue pins the thermistors
          // are connected to have identical pullup resistance values.
          if (sensor[i].type == 1 || sensor[i].type == 2)
          {
            // Temperature in Celcius:
            if (M == 0.0)
              T = -99;
            else if (M == 1023.0)
              T = 999;
            else
              T = 1.0 / (TH_A + TH_B * log((1023 - M) * R_A1 / M)) - 273.15;
            if (sensor[i].type == 2)
              // Case temperature:
              g.T = T;
            M = T;
            if (T > CRAZY_TEMP)
            {
              g.alarm = ALARM;
              g.bad_sensor = i;
              alarm_actions();
              return;
            }
          }

          // Adding 0.5 for proper roundoff:
          sensor[i].avr = M + 0.5;
          if (sensor[i].avr < 0)
            sensor[i].avr = 0;
          else if (sensor[i].avr > 1023)
            sensor[i].avr = 1023;

          if (sensor[i].type == 3 && sensor[i].train.zero > 0)
          {
            // Converting to a raw quantity which grows (non-linearly) with the averaged bed resistance.
            // Specifically, it is 1023*(1-R_BED/R/2). In these units, infinitive resistance is 1023,
            // and raw 0 corresponds to the lowest measurable resistance of R_BED/2.
            // Normal raw value should be ~512.
            sensor[i].avr = 1023 - sensor[i].avr;
          }
        }

        update_sensor(i);
        sensor[i].i = 0;
        sensor[i].sum = 0;

#ifdef DEBUG
        /*
                if (i == 0)
                {
                  Serial.print(g.t);
                  Serial.print(" ");
                }
                Serial.print(sensor[i].avr);
                Serial.print(" ");
                if (i == N_SENSORS - 1)
                  Serial.println("");
        */
#endif

      }
    } // if (sensor[i].on && g.t - sensor[i].t0 > SENS_DT)
  } // for loop

  return;
}

