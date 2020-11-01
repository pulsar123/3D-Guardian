/*
  Reading the 5 buttons on LCD1602 using one analogue input pin BUTTONS_PIN
*/

// read the buttons
void read_LCD_buttons()
{
  //!!!
  //  if (g.alarm == ALARM)
  //    return;

  if (g.alarm == PROG)
    return;


  // Reading buttons only once every KEYS_DT us:
  if (millis() - g.keys_t0 > KEYS_DT)
  {
    g.keys_t0 = millis();

    int adc_key_in = analogRead(BUTTONS_PIN);      // read the value from the sensor
    // Interpreting the analogue read from the LCD shield:
    if (adc_key_in < 50)
      g.key = RIGHT;
    else if (adc_key_in < 195)
      g.key = UP;
    else if (adc_key_in < 380)
      g.key = DOWN;
    else if (adc_key_in < 555)
      g.key = LEFT;
    else if (adc_key_in < 790)
      g.key = SELECT;
    else
    {
      g.key = NOKEY;
    }

    if (g.key == NOKEY)
      // No key is pressed, so just exiting:
    {
      return;
    }

    if (g.key_old == NOKEY)
      // A key has just been pressed:
    {
      g.key_count = 0;
    }
    else
    {
      g.key_count++;
    }

    // A key is considered to be pressed only if it had at least N_KEY_COUNT readings
    // Also implementing key auto-repeat here
    if (g.key_count == N_KEY_COUNT || g.key_count >= N_KEY_START && (g.key_count - N_KEY_START) % N_KEY_REPEAT == 0)
      g.key_pressed = 1;

    if (g.key_pressed)
    {
      // Updating the time as long as a key is being pressed:
      g.key_t0 = millis();

      g.refresh_display = 1;
      switch (g.key)
      {
        case SELECT: // Switching to the main menu
          if (g.screen < 0)
          {
            g.screen = 0;
          }
          else
          {
            g.screen = -1;
            g.menu_level = 0;
            g.menu_id = g.default_id;
            g.menu_top_id = g.default_id;
            g.edit = 0;
          }
          break;

        case UP:
          if (g.screen == 0)
            g.screen = N_SENSORS;
          else if (g.screen > 0 && g.screen < N_SENSORS + 1)
            g.screen--;
          break;

        case DOWN:
          if (g.screen == N_SENSORS)
            g.screen = 0;
          else if (g.screen >= 0 && g.screen < N_SENSORS)
            g.screen++;
          break;

      } // switch key
    }

  }

  return;
}

