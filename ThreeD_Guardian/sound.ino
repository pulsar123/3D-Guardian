void sound()
/*
  Generating a sound signal in case of an alarm, on SOUND_PIN (any digital pin).
  Also controls the red LED, and yellow LED blinking in PROG/no_sensors modes.
*/
{

  if (g.alarm == PROG)
    // We are in Programming mode - flashing yellow LED regularly
  {
    if (g.t - g.prog_led_t0 > PROG_LED_PERIOD)
    {
      g.prog_led_t0 = g.t;
      g.LEDy_state = 1 - g.LEDy_state;
      digitalWrite(LEDY_PIN, g.LEDy_state);
    }
    return;
  }

  if (g.no_sensors)
    // The printer cable is disconnected - series of two yellow LED flashes
  {
    if (g.t - g.nosensors_led_t0 > NOSENSORS_LED_PERIOD)
    {
      g.nosensors_led_t0 = g.t;
      g.nosensors_step++;
      if (g.nosensors_step > 6)
        g.nosensors_step = 1;
      // Setting up a two-flash sequence, with the gap between sequences = 2x gap between the two flashes
      if (g.nosensors_step == 1 || g.nosensors_step == 3)
        g.LEDy_state = 1;
      else if (g.nosensors_step == 2 || g.nosensors_step == 4)
        g.LEDy_state = 0;
      else
        return;
      digitalWrite(LEDY_PIN, g.LEDy_state);
    }
    return;
  }

  if (g.alarm != ALARM && g.alarm != WARNING)
    return;

  // Alarm sound:
  if (g.alarm == ALARM)
    if (g.t - g.sp_t0 > SP_DT)
    {
      g.sp_t0 = g.t;
      // Flipping the speaker's state:
      g.sp_state = 1 - g.sp_state;
      // Sending the new state to the speaker:
      digitalWrite(SOUND_PIN, g.sp_state);
    }

  // Warning sound (chirping):
  if (g.alarm == WARNING)
  {
    if (g.t - g.sp_t0 > CHIRP_PERIOD && g.sp_state == 0)
    {
      g.sp_t0 = g.t;
      g.sp_state = 1;
      digitalWrite(SOUND_PIN, g.sp_state);
    }
    if (g.t - g.sp_t0 > CHIRP_DT && g.sp_state == 1)
    {
      g.sp_state = 0;
      digitalWrite(SOUND_PIN, g.sp_state);
    }
  }

  return;

}

