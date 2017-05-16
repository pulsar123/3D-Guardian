void alarm()
/*
  Here we decide whether to trigger an alarm or warning, based on all sensor data.

  Doesn't work in TRANING mode.
*/
{
  byte warning;
  float r, r_max;
  byte i_max;


  if (g.alarm == TRAINING || g.alarm == ALARM)
    return;

  warning = 0;
  r_max = -1.0;

  for (byte i = 0; i < N_SENSORS; i++)
  {
    // Skipping the sensors which are not on yet:
    if (!sensor[i].on)
      continue;

    // Skipping the resistance sensor if it hasn't been initialized yet, or if the bed wasn't heated since rebooting:
    if (sensor[i].type == 3 && (sensor[i].train.zero == -1 || g.resistance == 0))
      continue;

    // Alarm criterion:
    if (sensor[i].avr > sensor[i].alarm_max)
    {
      g.alarm = ALARM;
      // Shutting down the printer:
      digitalWrite(SSR_PIN, 0);
      // Just in case:
      g.sp_t0 = g.t;
      g.sp_state = 0;
      // Shutting down the fan:
      g.duty = 0;
      update_duty();
      g.refresh_display = 1;
      g.bad_sensor = i;
      return;
    }

    // Warning criterion:
    if (sensor[i].avr > sensor[i].warn_max || sensor[i].avr < sensor[i].warn_min)
    {
      warning = 1;
      if (sensor[i].avr > sensor[i].warn_max)
        r = (float)sensor[i].avr / (float)sensor[i].warn_max;
      else
        r = (float)sensor[i].warn_min / (float)sensor[i].avr;
      // Finding the biggest offendor (sensor with the largest relative deviation from normal values):
      if (r > r_max)
      {
        r_max = r;
        i_max = i;
      }
    }
  }

  if (warning == 1 && g.alarm == GUARDING)
    // Warning mode is triggered
  {
    g.alarm = WARNING;
    g.bad_sensor = i_max;
    g.refresh_display = 1;
  }

  if (warning == 0 && g.alarm == WARNING)
    // Exiting the warning mode
  {
    g.alarm = GUARDING;
    g.refresh_display = 1;
  }


  return;
}

