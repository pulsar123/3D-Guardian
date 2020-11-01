void serial_add_int1 (int x, int * i)
// Adding one int variable to the buffer to be trasmitted via serial connection
{
  if (*i + 1 >= BUF_SIZE - 1)
    return;
  sprintf (&g.buffer[*i], "%1d", x);
  *i = *i + 1;
  return;
}
void serial_add_int2 (int x, int * i)
// Adding one int variable to the buffer to be trasmitted via serial connection
{
  if (*i + 2 >= BUF_SIZE - 1)
    return;
  sprintf (&g.buffer[*i], "%2d", x);
  *i = *i + 2;
  return;
}
void serial_add_int3 (int x, int * i)
// Adding one int variable to the buffer to be trasmitted via serial connection
{
  if (*i + 3 >= BUF_SIZE - 1)
    return;
  sprintf (&g.buffer[*i], "%3d", x);
  *i = *i + 3;
  return;
}
void serial_add_int4 (int x, int * i)
// Adding one int variable to the buffer to be trasmitted via serial connection
{
  if (*i + 4 >= BUF_SIZE - 1)
    return;
  sprintf (&g.buffer[*i], "%4d", x);
  *i = *i + 4;
  return;
}


void serial()
// Module to communicate with the WiFi module ESP8266 via serial connection.
{

  // If no data coming from ESP - serial connection is considered to be broken (now we can use it for reading/writing EEPROM data)
  //  if (g.t - g.t_SSR > DT_SSR_MAX)
  //    return;

  // In DEBUG mode, we use serial to print sensor data regularly, instead of communicating with ESP board

  // Serial reception:
  if (millis() - g.serial_in_t0 > SERIAL_IN_DT)
  {
    g.serial_in_t0 = millis();
    int N_command = 0;
    int i0;

    if (Serial.available() > 0)
    {
      // Reading the current string from the serial connection:
      char s_char;
      int i = 0;
      while (Serial.available() > 0 && i < BUF_SIZE - 1)
      {
        // Reading one character from the serial terminal:
        s_char = Serial.read();
        g.buffer[i] = s_char;

        // Detecting the beginning of a command:
        if (i >= 4 && strncmp(&g.buffer[i - 4], MAGIC_EtoA, 4) == 0)
        {
          N_command++;
          if (N_command > MAX_COMMAND)
            break;
          // The starting index of the actual command:
          g.i_command[N_command - 1] = i;
        }
        i++;
      }

      // In programming mode, ignoring all serial commands except for "T" command:
      // Processing the received commands:
      for (i = 0; i < N_command; i++)
      {
        if (g.alarm != PROG)
        {
          if (strncmp(&g.buffer[g.i_command[i]], "S", 1) == 0)
            // Shutting down the printer (not an alarm):
          {
            g.printer = 1 - g.printer;
            digitalWrite(SSR_PIN, g.printer);
            g.refresh_display = 1;
            g.case_clearing = 0;
          }

          else if (strncmp(&g.buffer[g.i_command[i]], "C", 1) == 0)
            // Clearing the case
          {
            clear_the_case();
            g.refresh_display = 1;
          }

          else if (strncmp(&g.buffer[g.i_command[i]], "M", 1) == 0)
            // Training / Guarding mode toggle
          {
            if (g.alarm == TRAINING)
            {
              training (0);
              g.alarm = GUARDING;
            }
            else
            {
              training (1);
              g.alarm = TRAINING;
            }
            g.refresh_display = 1;
          }
        }

        if (strncmp(&g.buffer[g.i_command[i]], "T", 1) == 0)
          // Getting the SSR temperature (C) value from ESP:
        {
          strncpy(g.buf4, &g.buffer[g.i_command[i] + 1], 3);
          g.buf4[3] = '\0';
          g.T_SSR = atoi(g.buf4);
          g.SSR_temp = 1;
          g.t_SSR = millis();
        }

      }
    }
  }


  // Serial transmission:
  if (g.alarm != PROG && millis() - g.serial_out_t0 > SERIAL_OUT_DT)
  {
    g.serial_out_t0 = millis();
    g.N_serial++;

    // Starting all outgoing messages with this magic prefix:
    sprintf(g.buffer, "%4s", MAGIC_AtoE);
    int i = 4;

    serial_add_int1((int)(1 - g.printer), &i);
    serial_add_int1((int)g.case_clearing, &i);
    serial_add_int2((int)g.alarm, &i);
    serial_add_int3((int)g.duty_perc, &i);

    if (g.N_serial == N_SERIAL_MAX)
      // Every N_SERIAL_MAX we continue with a longer message (report from all sensors)
    {
      g.N_serial = 0;

      for (int j = 0; j < N_SENSORS; j++)
      {
        if (g.alarm == TRAINING)
        {
          serial_add_int4(sensor[j].train.min, &i);
          serial_add_int4(sensor[j].train.max, &i);
        }
        else
        {
          serial_add_int4(sensor[j].guard.min, &i);
          serial_add_int4(sensor[j].guard.max, &i);
        }
        serial_add_int4(sensor[j].avr, &i);
        serial_add_int4(sensor[j].alarm_max, &i);
      }
    }
    g.buffer[i] = '\0';

    // Sending the buffer to ESP controller:
    Serial.print(g.buffer);
  }


  return;
}


