void training (byte mode)
// Enable (mode=1) / disable (mode=0) training mode
{
  //!!!
  //  if (g.alarm == ALARM)
  //    return;

  if (mode == 1)
    // Switching from guarding to training mode:
  {
    g.LEDy_state = 1;
    digitalWrite(LEDY_PIN, g.LEDy_state);
  }
  else
    // Switching from training to guarding mode:
  {
    g.LEDy_state = 0;
    digitalWrite(LEDY_PIN, g.LEDy_state);
    // Every time we switch to guardian mode, we need to update all limits:
    update_limits_all();
  }
  return;
}


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
    if (sensor[i].on == 0 && g.t - g.t0_init > sensor[i].init_delay)
      sensor[i].on = 1;
  }

  if (g.t - g.t0_init > PROG_INIT)
  {
    // After initial delay, switch the controller to the mode read from EEPROm at boot time (only if serial communication with ESP is already established):
    if (g.prog_on == 1 && g.t - g.t_SSR < DT_SSR_MAX)
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
    if (g.prog_on == 0  && g.t - g.t_SSR > DT_SSR_MAX)
    {
      g.alarm_ini = g.alarm;
      g.alarm = PROG;
      g.prog_on = 1;
      // Shutting down the fan:
      g.case_clearing = 0;
      g.duty = 0;
      update_duty();
      lcd.clear();
      lcd.print("Programming");
    }
  }

  return;
}


void init_sensor (sensor_EEPROM_struc * sensor)
{
  sensor->min = 1023;
  sensor->max = 0;
  sensor->zero = -1;
  sensor->sum = 0;
  sensor->N = 0;
  return;
}


void update_sensor (int i)
// Updating training data for the i-th sensor with the new value , x
{
  int value;


  if (g.alarm == TRAINING)
  {
    sensor[i].train.N++;
    sensor[i].train.sum = sensor[i].train.sum + sensor[i].avr;
    if (sensor[i].avr < sensor[i].train.min)
      sensor[i].train.min = sensor[i].avr;
    if (sensor[i].avr > sensor[i].train.max)
      sensor[i].train.max = sensor[i].avr;
  }
  else
  {
    sensor[i].guard.N++;
    sensor[i].guard.sum = sensor[i].guard.sum + sensor[i].avr;
    if (sensor[i].avr < sensor[i].guard.min)
      sensor[i].guard.min = sensor[i].avr;
    if (sensor[i].avr > sensor[i].guard.max)
      sensor[i].guard.max = sensor[i].avr;
  }

  return;
}


void update_limits_all ()
// Updating all sensors' limits (when switching to guardian mode)
{
  int value;

  for (byte i = 0; i < N_SENSORS; i++)
  {
    // The sensor has never been on yet:
    if (sensor[i].train.min == 1023)
      continue;

    // recomputing the running average trained value:
    sensor[i].train_avr = (int)((float)sensor[i].train.sum / (float)sensor[i].train.N + 0.5);

    if (sensor[i].type == 1 || sensor[i].type == 2 || sensor[i].type == 4)
      // Thermistors (data is in Celsius degrees)
    {
      sensor[i].warn_min = sensor[i].train.min - WARN_DTEMP;
      sensor[i].warn_max = sensor[i].train.max + WARN_DTEMP;
      sensor[i].alarm_max = sensor[i].train.max + ALARM_DTEMP;
    }
    else
      // Other sensors (data is raw, 0...1023)
    {
      // Computing the lower limit for a warning:
      value = WARN_MIN * (float)(sensor[i].train_avr - sensor[i].train.min) + 0.5;
      if (value < WARN_CONST)
        value = WARN_CONST;
      sensor[i].warn_min = sensor[i].train_avr - value;
      if (sensor[i].warn_min < 1)
        sensor[i].warn_min = 1;

      // Computing the higher limit for a warning:
      value = WARN_MAX * (float)(sensor[i].train.max - sensor[i].train_avr) + 0.5;
      if (value < WARN_CONST)
        value = WARN_CONST;
      sensor[i].warn_max = sensor[i].train_avr + value;
      if (sensor[i].warn_max > 1022)
        sensor[i].warn_max = 1022;

      // Computing the higher limit for an alarm:
      value = ALARM_MAX * (float)(sensor[i].train.max - sensor[i].train_avr) + 0.5;
      if (value < ALARM_CONST)
        value = ALARM_CONST;
      sensor[i].alarm_max = sensor[i].train_avr + value;
      if (sensor[i].alarm_max > 1022)
        sensor[i].alarm_max = 1022;
    }

  }

  return;
}


void EEPROM_put()
// Writing all data to EEPROM
{
  if (g.alarm == TRAINING || g.alarm == GUARDING)
    EEPROM.put(ADDR_ALARM, g.alarm);

  EEPROM.put(ADDR_T_TARGET, g.T_target);
  EEPROM.put(ADDR_FAN_MODE, g.fan_mode);
  EEPROM.put(ADDR_DT_CASE, g.dt_case);

  if (g.alarm == TRAINING)
    // Only storing the training data to EEPROM:
  {
    for (byte i = 0; i < N_SENSORS; i++)
      EEPROM.put(g.addr_tr[i], sensor[i].train);
  }
  return;
}

void EEPROM_get()
// Reading all data from EEPROM
{
  EEPROM.get(ADDR_ALARM, g.alarm);
  EEPROM.get(ADDR_T_TARGET, g.T_target);
  EEPROM.get(ADDR_FAN_MODE, g.fan_mode);
  EEPROM.get(ADDR_DT_CASE, g.dt_case);
  for (byte i = 0; i < N_SENSORS; i++)
    EEPROM.get(g.addr_tr[i], sensor[i].train);

  return;
}


char * trunc (char* buf, int x)
// Truncating >999 values to *XX form
{
  if (x < 1000)
    sprintf(buf, "%3d", x);
  else
    sprintf(buf, "*%02d", x - 1000);

  return buf;
}



byte translate (byte old_id)
// Translating from old (unsorted) to new (sequential starting from 0) ids. Only done once at the beginning.
{

  for (byte i = 0; i < N_MENU; i++)
  {
    if (MenuItem[i].id == old_id)
      return i;
  }

  return NONE;
}



void update_duty ()
// Run it after every change of g.duty of the fan
{
  int loc_duty = g.duty;

#ifdef NO_PWM
  // If the fan doesn't have PWM control, g.duty can only be either 0 (fan off) or 255 (fan on):
  if (loc_duty > 0)
    loc_duty = 255;
#endif

  analogWrite(FAN_PIN, loc_duty);
  g.duty_perc = (int)(100 * (float)loc_duty / (float)MAX_DUTY + 0.5);

  return;
}


long int t_motor (int step)
// Time (in us) corresponding to the step "step" (=1...N_STEPS)
{
  long int dt;

  if (step <= N_STEPS / 2)
    // First half of the trip - accelerating with a fixed acceleration (=ACCEL)
  {
    dt = (long int)(sqrt(2.0 * step / ACCEL) + 0.5);
  }
  else if (step < N_STEPS)
    // Second half of the trip - decelerating with a fixed deceleration (=ACCEL)
  {
    dt = T_MOTOR_US - (long int)(sqrt(2.0 * (N_STEPS - step) / ACCEL) + 0.5);
  }
  else
    // The final step
  {
    dt = T_MOTOR_US;
  }


  return g.t0_motor + dt;
}



void clear_the_case ()
// Initiating clearing the case (running fan at full speed, using motor to open the exhaust lid)
{
  g.case_clearing = 1;
  g.case_t0 = g.t;
  g.fan_mode_old = g.fan_mode;
  g.fan_mode = 2;
  return;
}


void train_dump()
// Dumping train data to serial interface
{
  for (byte i = 0; i < N_SENSORS; i++)
  {
    Serial.print(sensor[i].train.min);
    Serial.print(" ");
    Serial.print(sensor[i].train.max);
    Serial.print(" ");
    Serial.print(sensor[i].train.zero);
    Serial.print(" ");
    Serial.print(sensor[i].train.sum);
    Serial.print(" ");
    Serial.println(sensor[i].train.N);
  }

  // Special "end of dump" HEX sequence:
  Serial.print("..");
  return;
}


void train_load()
{
  int i = 0;
  long int x;
  while (Serial.available() > 0)
  {
    for (byte i = 0; i < N_SENSORS; i++)
    {
      if (x = Serial.parseInt() != 0)
        sensor[i].train.min = (int)x;
      if (x = Serial.parseInt() != 0)
        sensor[i].train.max = (int)x;
      if (x = Serial.parseInt() != 0)
        sensor[i].train.zero = (int)x;
      if (x = Serial.parseInt() != 0)
        sensor[i].train.sum = (int)x;
      if (x = Serial.parseInt() != 0)
        sensor[i].train.N = (int)x;
    }
  }

  EEPROM_put();

  /*
    while (Serial.available() > 0 && i < BUF_SIZE - 1)
    {
    // Reading one character from the serial terminal:
    s_char = Serial.read();
    g.buffer[i] = s_char;
    // Looking for the end of dump signal (..):
    if (i > 0 && strncmp(&g.buffer[i - 1], "..", 2) == 0)
    {
      N = i - 1;
      break;
    }
    i++;
    }

    for (byte i = 0; i < N_SENSORS; i++)
    {
    Serial.print(sensor[i].train);
    Serial.println();
    }

    // Special "end of dump" HEX sequence:
    Serial.print("..");
  */
  return;
}


void alarm_actions()
// Everything which needs to be done when an alarm is triggered
{
  // Shutting down the printer:
  digitalWrite(SSR_PIN, 0);
  // Just in case:
  g.sp_t0 = g.t;
  g.sp_state = 0;
  // Shutting down the fan:
  g.case_clearing = 0;
  g.duty = 0;
  update_duty();
  g.refresh_display = 1;

}

