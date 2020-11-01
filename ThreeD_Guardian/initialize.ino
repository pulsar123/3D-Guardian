void initialize(byte factory_reset)
/* Initializing all the variables, with the optional factory reset (resetting all the EEPROM data).
*/
{
  int address;

  if (factory_reset)
  {
    g.alarm = TRAINING;
    g.T_target = 50;
    g.fan_mode = 1;
    g.dt_case = 900;
    g.manual_fan = 127;

    // Initial EEPROM values:
    for (byte i = 0; i < N_SENSORS; i++)
      init_sensor(&sensor[i].train);

    // writing initial values to EEPROM:
    EEPROM_put();
    // Writing the same values to the three memory registers:
    for (byte i = 0; i < N_SENSORS; i++)
    {
      EEPROM.put(ADDR_DATA1 + g.addr_tr[i] - ADDR_DATA, sensor[i].train);
      EEPROM.put(ADDR_DATA2 + g.addr_tr[i] - ADDR_DATA, sensor[i].train);
      EEPROM.put(ADDR_DATA3 + g.addr_tr[i] - ADDR_DATA, sensor[i].train);
    }
  }

  else
  {
    // Reading the data from EEPROM:
    EEPROM_get();
    // Autosaving the trained data to a copy in EEPROM at each boot time (only in TRAINING mode):
    if (g.alarm == TRAINING)
      for (byte i = 0; i < N_SENSORS; i++)
      {
        EEPROM.put(ADDR_DATA1 + g.addr_tr[i] - ADDR_DATA, sensor[i].train);
      }
  }

  // Initially (first PROG_INIT ms) the controller is in PROG mode; it switches to the proper mode in cleanup()
  g.alarm_ini = g.alarm;
#ifndef NO_ESP  
  g.alarm = PROG;
  g.prog_on = 1;
  lcd.clear();
  lcd.print("Programming");
#endif

  // Resetting guarding sensor data after each reboot:
  for (byte i = 0; i < N_SENSORS; i++)
    init_sensor(&sensor[i].guard);

  g.th_case = -1;
  g.i_SSR = -1;
  // Initializing sensors (non-EEPROM data):
  for (byte i = 0; i < N_SENSORS; i++)
  {
    // Staggered sensor reading:
    sensor[i].t0 = millis() + i * 100;
    sensor[i].avr = 0;
    sensor[i].old = 0;
    sensor[i].sum = 0;
    sensor[i].i = 0;
    // Initially all sensors are off (they will be turned on after sensor[i].init_delay time in cleanup() )
    sensor[i].on = 0;
    if (sensor[i].type == 2)
      // This thermistor will be used to control fan, to maintain target temperature in the case:
      g.th_case = i;
    // For now only accepting one current sensor (the last current sensor in sensor[] vector):
    if (sensor[i].type == 3)
    {
      g.resistance_sensor = i;
      // Critical voltage (in raw units) to detect whether to make a resistance measurement at this point in time (make measurement if V1,2_raw > V_crit_raw)
      sensor[i].V_crit_raw = (int)(1024.0 * V_CRIT * sensor[i].divider / 5.0 + 0.5);
    }
    if (sensor[i].type == 4)
    {
      // Index of the SSR temperature "sensor":
      g.i_SSR = i;
    }
  }


  update_limits_all();

  if (g.alarm == TRAINING)
    training(1);

  g.fan_mode_old = g.fan_mode;
  g.resistance = 0;


  return;
}
