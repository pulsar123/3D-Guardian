/*
  Displaying stuff when needed
*/

void display()
{

/*
if (g.t - t_temp > 100)
{
  t_temp = g.t;
  lcd.clear();
  lcd.print((int)g.alarm);
  lcd.print(",");
  lcd.print(g.prog_on);
  lcd.print(",");
  lcd.print(g.t_SSR);
  return;
}
*/
  if (g.alarm == PROG)
    return;

  if (g.screen != 0 && (millis() - g.key_t0 > SCREEN0_DT || g.exit_menu))
    // Timing out to default screen, after SCREEN0_DT ms without a key press
  {
    g.screen = 0;
    g.refresh_display = 1;
    g.edit = 0;
    g.exit_menu = 0;
  }

  // Menu processing:
  if (g.screen == -1 && g.key_pressed)
  {
    menu_core();
    return;
  }

  // Refreshing specific displayed variables if they changed:
  if (g.screen == 0)
  {
    if (sensor[g.th_case].avr != sensor[g.th_case].old)
    {
      lcd.setCursor(2, 1);
      sprintf(g.buffer, "%2d", sensor[g.th_case].avr);
      lcd.print(g.buffer);
    }
    if (g.duty_perc != g.duty_perc_old)
    {
      lcd.setCursor(13, 1);
      lcd.print(display_fan(g.buf1));
    }
  }



  // Default + alternative screens processing:
  if (g.refresh_display)
  {
    lcd.clear();

    // The first line of default screen:
    if (g.screen == 0)
    {
      if (g.case_clearing == 1 && g.alarm <= 0)
      {
        lcd.print("Clearing");
      }
      else if (g.printer == 0 && g.alarm <= 0)
      {
        lcd.print("Stopped");
      }
      else
      {
        switch (g.alarm)
        {
          case TRAINING:
            lcd.print("Training");
            break;

          case WARNING:
            lcd.print("WARNING:   ");
            lcd.print(sensor[g.bad_sensor].name);
            break;

          case GUARDING:
            lcd.print("Guarding");
            break;

          case ALARM:
            lcd.print("ALARM!!!   ");
            lcd.print(sensor[g.bad_sensor].name);
        }
      }

      lcd.setCursor(0, 1);
      sprintf(g.buffer, "T=%2d/%2dC Fan=%2s", sensor[g.th_case].avr, g.T_target, display_fan(g.buf1));
      lcd.print(g.buffer);
    }
    else if (g.screen > 0 && g.screen <= N_SENSORS)
    {
      int xmin;
      byte i = g.screen - 1;

      if (sensor[i].type == 3 && sensor[i].train.zero < 0)
        // We haven't initialized the resistance (voltage) sensor yet
      {
        sprintf(g.buffer, " Perform      ");
        lcd.print(g.buffer);
        sprintf(g.buffer, "\"Init voltage\"");
        lcd.setCursor(0, 1);
        lcd.print(g.buffer);
      }
      else
      {
        sprintf(g.buffer, "%3s:%3s", sensor[i].name, trunc(g.buf1, sensor[i].avr));
        lcd.print(g.buffer);
        if (g.alarm == GUARDING)
        {
          xmin = sensor[i].guard.min;
          sprintf(g.buffer, "(%3s-%3s)", trunc(g.buf2, xmin), trunc(g.buf3, sensor[i].guard.max));
          lcd.print(g.buffer);
        }
        // To display alarm_max in training mode:
        update_limits_all();
        xmin = sensor[i].train.min;
        sprintf(g.buffer, "tr: %3s(%3s-%3s)", trunc(g.buf1, sensor[i].alarm_max),
                trunc(g.buf2, xmin), trunc(g.buf3, sensor[i].train.max));
        lcd.setCursor(0, 1);
        lcd.print(g.buffer);
      }
    }

    g.refresh_display = 0;
    //!!!
 //       lcd.setCursor(0, 0);
 //   lcd.print((int)g.alarm);
  }


  // Printing the clearing time every second:
  if (g.case_clearing == 1 && g.alarm <= 0 && g.screen == 0 && (millis() - g.case_t0) % 1000 == 0)
  {
    lcd.setCursor(10, 0);
    sprintf(g.buffer, "%3d", (millis() - g.case_t0) / 1000);
    lcd.print(g.buffer);
  }

  return;
}



char * display_fan (char * buf)
{
  if (g.fan_mode > 0)
    if (g.fan_mode==4 && g.aclear_done==1)
      sprintf(buf, "CLR");
      else
      sprintf(buf, "%3d", g.duty_perc);
  else
    sprintf(buf, " --");

  return buf;
}

