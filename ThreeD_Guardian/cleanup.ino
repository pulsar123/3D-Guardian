void cleanup()
// Cleanup at the end of every Arduino loop
{
  g.key_pressed = 0;
  g.key_old = g.key;
  g.refresh_display = 0;
  g.duty_perc_old = g.duty_perc;
  for (byte i = 0; i < N_SENSORS; i++)
  {
    sensor[i].old = sensor[i].avr;
    if (sensor[i].on == 0 && millis() - g.t0_init > sensor[i].init_delay)
      sensor[i].on = 1;
  }

  // Refreshing the screen regularly (only for screen>0)
  if (millis() - g.t_refresh > DT_REFRESH && g.screen > 0)
  {
    g.t_refresh = millis();
    g.refresh_display = 1;
  }


  // Once first serial data received from ESP, switch the controller to the mode read from EEPROm at boot time:
  if (g.prog_on == 1 && millis() - g.t_SSR < DT_SSR_MAX)
  {
    g.alarm = g.alarm_ini;
    g.prog_on = 0;
    if (g.alarm == TRAINING)
      training (1);
    else
      training (0);
    g.screen = 0;
    g.exit_menu = 1;
    g.refresh_display = 1;

  }
  // If no serial communication for > DT_SSR_MAX ms, we are switching to the PROG mode:
  // This is ignored for the first PROG_INIT ms after reboot
  #ifndef NO_ESP
  g.t = millis();
  if (g.prog_on == 0  && g.t - g.t_SSR > DT_SSR_MAX && g.t - g.t0_init > PROG_INIT && g.t - g.t0_init < PROG_MAX)
  {
    g.alarm_ini = g.alarm;
    g.alarm = PROG;
    g.prog_on = 1;
    g.prog_led_t0 = g.t;
    // Shutting down the fan:
    //      g.case_clearing = 0;
    //      g.duty = 0;
    //      update_duty();
    lcd.clear();
    lcd.print("Programming");
  }
  #endif

  return;
}

