void sound()
/*
  Generating a sound signal in case of an alarm, on SOUND_PIN (any digital pin).
  Also controls the red LED, and yellow LED blinking in PROG/no_sensors modes.
*/
{

  if (g.alarm == PROG || g.no_sensors)
  {
    if (g.t - g.prog_led_t0 > PROG_LED_PERIOD)
    {
      g.prog_led_t0 = g.t;
      g.LEDy_state = 1 - g.LEDy_state;
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

