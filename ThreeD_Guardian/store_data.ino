void store_data()
// Store sensor data to EEPROM regularly (every STORE_DT ms).
{

  if (g.alarm != PROG && g.no_sensors == 0 && g.t - STORE_DT > g.store_t0)
  {
    EEPROM_put();
    g.store_t0 = g.t;
  }

  return;
}


