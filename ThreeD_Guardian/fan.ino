/*
  Regulating the fan's speed (by updating its g.duty=0...255; 0 corresponds to zero speed). Fan is disabled in ALARM  mode.
*/

void fan()
{
  if (g.alarm == ALARM)
    return;

  if (g.case_clearing == 1 && (millis() - g.case_t0) / 1000 > g.dt_case)
    // End of case clearing
  {
    g.case_clearing = 0;
    // reverting to the original fan mode:
    g.fan_mode = g.fan_mode_old;
    if (g.fan_mode == 4)
    {
      // The signal that we just did AClear clearing:
      g.aclear_done = 1;
    }
    g.refresh_display = 1;
  }


  if (g.fan_mode == 0 || g.fan_mode == 4 && g.duty > 0)
    // Fan off
  {
    g.duty = 0;
    update_duty();
  }

  else if (g.fan_mode == 1 && millis() - g.fan_t0 > FAN_DT)
    // Fan auto (fan is used to regulate the temperature inside the printer cabinet)
  {
    g.aclear_done = 0;  // only matters when using AClear mode
    g.fan_t0 = millis();
    // Can be positive or negative:
    float dT = g.T - (float)g.T_target;
    if (dT > FAN_HYST || dT < -FAN_HYST)
      // We moved outside the allowed temperature range - fan speed adjustment is required
    {
      g.duty = g.duty + dT * FAN_SCALE;
      if (g.duty > MAX_DUTY)
        g.duty = MAX_DUTY;
      if (g.duty < 0)
        g.duty = 0;

      // Adjusting the fan's speed:
      update_duty();
    }
  }

  else if (g.fan_mode == 2)
    // Fan on at the manual level
  {
    g.aclear_done = 0;  // only matters when using AClear mode
    if (g.duty != g.manual_fan)
    {
      if (g.duty == 0)
        g.open_lid = 1; // Immediately open the lid
      g.duty = g.manual_fan;
      update_duty();
    }
  }

  else if (g.fan_mode == 3)
    // Fan on at 100%
  {
    g.aclear_done = 0;  // only matters when using AClear mode
    if (g.duty < MAX_DUTY)
    {
      if (g.duty == 0)
        g.open_lid = 1; // Immediately open the lid
      g.duty = MAX_DUTY;
      update_duty();
    }
  }


  return;
}

