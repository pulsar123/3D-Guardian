// Menu functions

void menu_init ()
// Initializing menu items. This changes all ids in MenuItem[] array of structures. In the updated array,
// id = array index. Also updates any other variables which explicitely use old ids.
{
  // Translating all ids (except for the .id element) in MenuItem structures:
  for (byte i = 0; i < N_MENU; i++)
  {
    MenuItem[i].prev_id = translate(MenuItem[i].prev_id);
    MenuItem[i].next_id = translate(MenuItem[i].next_id);
    MenuItem[i].up_id = translate(MenuItem[i].up_id);
    MenuItem[i].down_id = translate(MenuItem[i].down_id);
  }

  // Updating the default menu id:
  g.default_id = translate(g.default_id);

  // Now we can translate the .id as well:
  for (byte i = 0; i < N_MENU; i++)
    MenuItem[i].id = i;
  // At this point, the .id elemnt in MenuItem[] array is identical to the corresponding index in the array, with all relationships between menu items preserved.

  return;
}


void menu_core ()
/*
  Core menu function. To be called from display(), only when g.screen<0 and g.key_pressed = 1.
*/
{
  byte line;

  // Determening the current display line:
  if (g.menu_id == g.menu_top_id)
    line = 0;
  else
    line = 1;

  if (g.key == DOWN)
  {
    if (g.edit)
      // Decreasing the value of the edit variable
    {
      MenuItem[g.menu_id].func(3, line);
      return;
    }
    else
      // Navigating down at the same level
    {
      if (MenuItem[g.menu_id].next_id != NONE)
      {
        // Explicitely assuming the display has two lines:
        if (g.menu_id != g.menu_top_id)
          // Scrolling down one line:
          g.menu_top_id = MenuItem[g.menu_top_id].next_id;
        // Moving the cursor down one line:
        g.menu_id = MenuItem[g.menu_id].next_id;
      }
    }
  }
  else if (g.key == UP)
  {
    if (g.edit)
      // Increasing the value of the edit variable
    {
      MenuItem[g.menu_id].func(2, line);
      return;
    }
    else
      // Navigating up at the same level
    {
      if (MenuItem[g.menu_id].prev_id != NONE)
      {
        // Explicitely assuming the display has two lines:
        if (g.menu_id == g.menu_top_id)
          // Scrolling up one line:
          g.menu_top_id = MenuItem[g.menu_top_id].prev_id;
        // Moving the cursor up one line:
        g.menu_id = MenuItem[g.menu_id].prev_id;
      }
    }
  }
  else if (g.key == RIGHT)
  {
    if (g.edit)
      // Accepting the edited value + optionally perforning an action on the new value, and exiting the edit mode
    {
      MenuItem[g.menu_id].func(4, line);
    }
    else
    {
      if (MenuItem[g.menu_id].down_id != NONE)
        // Moving one level down, at the g.menu_id point
      {
        // Memorizing old values, to recover from when coming back with LEFT command
        g.old_top_id[g.menu_level] = g.menu_top_id;
        g.old_id[g.menu_level] = g.menu_id;
        // One level deeper:
        g.menu_level++;
        g.menu_top_id = MenuItem[g.menu_id].down_id;
        g.menu_id = MenuItem[g.menu_id].down_id;
      }
      else
        // Processing the menu action
      {
        if (MenuItem[g.menu_id].func != NULL)
        {
          lcd.setCursor(0, line);
          // Making the square on the left side solid (signal that we can edit the value now):
          lcd.write(byte(1));
          // Entering the edit mode, or performing an immediate action:
          MenuItem[g.menu_id].func(1, line);
          return;
        }
      }
    }
  }
  else if (g.key == LEFT)
  {
    g.edit = 0;
    if (MenuItem[g.menu_id].up_id != NONE)
      // Moving one level up
    {
      if (g.old_id != NONE && g.old_top_id != NONE)
      {
        // One level higher:
        g.menu_level--;
        g.menu_top_id = g.old_top_id[g.menu_level];
        g.menu_id = g.old_id[g.menu_level];
      }
      else
        // Just in case (this shouldn't happen)
      {
        g.menu_top_id = MenuItem[g.menu_id].up_id;
        g.menu_id = MenuItem[g.menu_id].up_id;
      }
    }
    else
    {
      g.exit_menu = 1;
      return;
    }
  }

  // Displaying the updated menu screen:
  lcd.clear();
  byte id1 = g.menu_top_id;
  for (line = 0; line < 2; line++)
  {
    lcd.setCursor(0, line);
    if (id1 == g.menu_id)
      lcd.write(byte(0));  // Empty square character marks the current item position
    else
      lcd.write(" ");
    lcd.print(MenuItem[id1].name);
    //lcd.print(temp);
    if (MenuItem[id1].down_id != NONE)
      // Displaying right arrow at the end of the line if the item has sub-items
    {
      lcd.setCursor(15, line);
      lcd.write(R_ARROW);
    }
    else
    {
      if (MenuItem[id1].func != NULL && id1 == g.menu_id)
      {
        // Using the action function (mode=0) to display the current value/state on the right side of the current line:
        MenuItem[id1].func(0, line);
      }
    }
    id1 = MenuItem[id1].next_id;
    if (id1 == NONE)
      break;
  }

  return;
}


//++++++++++++++++++++++++++++++++++ Menu actions +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void menu_train_onoff (byte mode, byte line)
/*

    Action function control: mode=
   0: when displaying the menu item
   1: first RIGHT click (enters edit mode, or can be the only command, for an immediate action)
   2: UP click (increases the variable value in edit mode)
   3: DOWN click (decreases the variable value in edit mode)
   4: second RIGHT click (ends the edit mode, saves the variable, optionally perform an action on the new value)

*/
{

  // Immediate action version:
  if (mode == 1)
  {
    if (g.alarm == TRAINING)
    {
      // If any sensor has never been trained yet (because of the warmup delay), ignore the request, and stay in training mode
      for (byte i = 0; i < N_SENSORS; i++)
        if (sensor[i].train.max == 0)
          return;
      g.alarm = GUARDING;
    }
    else
      g.alarm = TRAINING;

    if (g.alarm == TRAINING || g.alarm == GUARDING)
      EEPROM.put(ADDR_ALARM, g.alarm);
    // Performing an action on the newly accepted value:
    if (g.alarm == TRAINING)
      training (1);
    else
      training (0);

    g.exit_menu = 1;
  }
  return;
}


void menu_update_train (byte mode, byte line)
// A typical "instant action on RIGHT click" action function
// Use the current guarding data to update the training data
{
  if (mode == 1)
  {
    for (byte i = 0; i < N_SENSORS; i++)
    {
      if (sensor[i].guard.min < sensor[i].train.min)
        sensor[i].train.min = sensor[i].guard.min;
      if (sensor[i].guard.max > sensor[i].train.max)
        sensor[i].train.max = sensor[i].guard.max;
      sensor[i].train.N = sensor[i].train.N + sensor[i].guard.N;
      sensor[i].train.sum = sensor[i].train.sum + sensor[i].guard.sum;
      EEPROM.put(g.addr_tr[i], sensor[i].train);
    }
    g.exit_menu = 1;
    // recomputing all warning and alarm limits:
    update_limits_all ();
  }
  return;
}


void menu_clear_case (byte mode, byte line)
// Initiating case clearing (fan will run at maximum speed for g.dt_case seconds)
{
  if (mode == 1 && g.alarm != ALARM)
  {
    clear_the_case();
    // Instantly going back to the default screen:
    g.exit_menu = 1;
  }
  return;
}


void menu_fan_control (byte mode, byte line)
// A typical "change variable" action function.
{
  const byte POS = 10;

  lcd.setCursor(POS, line);

  switch (mode)
  {
    case 0:
      g.new_value = g.fan_mode;
      break;

    case 1:
      g.edit = 1;
      break;

    case 2:
      if (g.new_value < 3)
        g.new_value++;
      break;

    case 3:
      if (g.new_value > 0)
        g.new_value--;
      break;

    case 4:
      g.edit = 0;
      g.fan_mode = g.new_value;
      EEPROM.put(ADDR_FAN_MODE, g.fan_mode);
      g.exit_menu = 1;
      return;
  }

  //lcd.print(g.new_value);

  if (g.new_value == 0)
    lcd.print("   Off");
  else if (g.new_value == 1)
    lcd.print("  Auto");
  else if (g.new_value == 2)
    lcd.print("Manual");
  else
    lcd.print("    On");

  return;
}



void menu_factory_reset (byte mode, byte line)
{
  const byte POS = 13;

  lcd.setCursor(POS, line);
  if (mode == 3)
    mode = 2;

  switch (mode)
  {
    case 0:
      g.new_value = 0;
      break;

    case 1:
      g.edit = 1;
      break;

    case 2:
      g.new_value = 1 - g.new_value;
      break;

    case 4:
      g.edit = 0;
      // Performing the factory reset:
      initialize(1);
      g.exit_menu = 1;
      return;
  }

  if (g.new_value == 0)
    lcd.print(" No");
  else
    lcd.print("Yes");

  return;
}


void menu_T_target (byte mode, byte line)
{
  const byte POS = 14;
  lcd.setCursor(POS, line);
  switch (mode)
  {
    case 0:
      g.new_value = g.T_target;
      break;

    case 1:
      g.edit = 1;
      break;

    case 2:
      if (g.new_value < T_TARGET_MAX)
        g.new_value++;
      break;

    case 3:
      if (g.new_value > T_TARGET_MIN)
        g.new_value--;
      break;

    case 4:
      g.edit = 0;
      g.T_target = g.new_value;
      EEPROM.put(ADDR_T_TARGET, g.T_target);
      return;
  }

  lcd.print(g.new_value);


  return;
}


void menu_printer (byte mode, byte line)
// Manually turning the printer on or off (doesn't work if the printer was shut down after an alarm)
{
  const byte POS = 12;
  lcd.setCursor(POS, line);
  switch (mode)
  {
    case 0:
      g.new_value = g.printer;
      break;

    case 1:
      g.edit = 1;
      break;

    case 2:
      if (g.new_value < 1)
        g.new_value++;
      break;

    case 3:
      if (g.new_value > 0)
        g.new_value--;
      break;

    case 4:
      g.edit = 0;
      g.printer = g.new_value;
      digitalWrite(SSR_PIN, g.printer);
      g.exit_menu = 1;
      return;
  }

  if (g.new_value == 0)
    lcd.print("Off");
  else
    lcd.print(" On");

  return;
}


void menu_dt_case (byte mode, byte line)
{
  const byte POS = 13;
  lcd.setCursor(POS, line);
  switch (mode)
  {
    case 0:
      g.new_value = g.dt_case/60;
      break;

    case 1:
      g.edit = 1;
      break;

    case 2:
      if (g.new_value < DT_CASE_MAX)
        g.new_value = g.new_value + 1;
      break;

    case 3:
      if (g.new_value > DT_CASE_MIN)
        g.new_value = g.new_value - 1;
      break;

    case 4:
      g.edit = 0;
      g.dt_case = g.new_value*60;
      EEPROM.put(ADDR_DT_CASE, g.dt_case);
      return;
  }

  sprintf(g.buffer, "%2d", g.new_value);
  lcd.print(g.buffer);

  return;
}



void menu_manual_fan (byte mode, byte line)
{
  const byte POS = 13;
  lcd.setCursor(POS, line);
  switch (mode)
  {
    case 0:
      g.new_value = (int)(100 * (float)g.manual_fan / (float)255 + 0.5);
      break;

    case 1:
      g.edit = 1;
      break;

    case 2:
      if (g.new_value < 99)
        g.new_value++;
      break;

    case 3:
      if (g.new_value > 1)
        g.new_value--;;
      break;

    case 4:
      g.edit = 0;
      // Converting the percentage (0...100%) to a byte range (0...255):
      g.manual_fan = (int)(((float)g.new_value) * 2.55 + 0.5);
      EEPROM.put(ADDR_MANUAL_FAN, g.manual_fan);
      return;
  }

  sprintf(g.buffer, "%3d", g.new_value);
  lcd.print(g.buffer);

  return;
}




void menu_zero_voltage (byte mode, byte line)
// Use the current average value of the type=3 (resistance) sensor to update sensor[g.resistance_sensor].train.zero
// Run this during initial training, when there is no voltage to the load (heated bed)
{
  if (g.alarm != TRAINING)
    return;

  const byte POS = 4;

  lcd.setCursor(POS, line);

  switch (mode)
  {
    case 0:
      // Displaying the current average voltage value (raw):
      g.new_value = sensor[g.resistance_sensor].avr;
      break;

    case 1:
      int i = g.resistance_sensor;
      if (sensor[i].train.zero < 0)
      {
        // Assigning a positive value to *.zero for the voltage (resistance) sensor is the signal that the resistance sensor is enabled and can start normal training:
        // We have to re-initialize the sensor now as it has bad (voltage, not resistance) data
        init_sensor(&sensor[i].train);
        init_sensor(&sensor[i].guard);
        sensor[i].train.zero = g.new_value;
        sensor[i].avr = 0;
        sensor[i].old = 0;
        sensor[i].sum = 0;
        sensor[i].i = 0;
        sensor[i].alarm_max = 0;
      }
      g.exit_menu = 1;
      break;
  }

  sprintf(g.buffer, "%4d(%4d)", g.new_value, sensor[g.resistance_sensor].train.zero);
  lcd.print(g.buffer);
  return;
}



void menu_init_all (byte mode, byte line)
// Re-initialize all sensors training data (only when in training mode)
{
  if (g.alarm != TRAINING)
    return;

  const byte POS = 13;

  lcd.setCursor(POS, line);
  if (mode == 3)
    mode = 2;

  switch (mode)
  {
    case 0:
      g.new_value = 0;
      break;

    case 1:
      g.edit = 1;
      break;

    case 2:
      g.new_value = 1 - g.new_value;
      break;

    case 4:
      g.edit = 0;
      g.exit_menu = 1;
      if (g.new_value == 1)
      {
        int zero = sensor[g.resistance_sensor].train.zero;
        for (byte i = 0; i < N_SENSORS; i++)
        {
          init_sensor(&sensor[i].train);
          // We preserve the old value of the zero voltage calibration (only for the resistance sensor):
          if (i == g.resistance_sensor)
            sensor[i].train.zero = zero;
          // writing initial values to EEPROM:
          EEPROM.put(g.addr_tr[i], sensor[i].train);
        }
      }
      return;
  }

  if (g.new_value == 0)
    lcd.print(" No");
  else
    lcd.print("Yes");

  return;
}


void menu_init_one (byte mode, byte line)
// Re-initialize training data for a single sensor (only when in training mode)
{
  int zero;

  if (g.alarm != TRAINING)
    return;

  const byte POS = 13;

  lcd.setCursor(POS, line);

  switch (mode)
  {
    case 0:
      g.new_value = 0;
      break;

    case 1:
      g.edit = 1;
      break;

    case 3:
      if (g.new_value < N_SENSORS - 1)
        g.new_value++;
      break;

    case 2:
      if (g.new_value > 0)
        g.new_value--;
      break;

    case 4:
      g.edit = 0;
      g.exit_menu = 1;
      if (g.new_value == g.resistance_sensor)
        zero = sensor[g.resistance_sensor].train.zero;
      init_sensor(&sensor[g.new_value].train);
      // We preserve the old value of the zero voltage calibration (only for the resistance sensor):
      if (g.new_value == g.resistance_sensor)
        sensor[g.new_value].train.zero = zero;
      // writing initial values to EEPROM:
      EEPROM.put(g.addr_tr[g.new_value], sensor[g.new_value].train);
      return;
  }

  lcd.print(sensor[g.new_value].name);

  return;
}


void menu_load_data (byte mode, byte line)
// Load training data from EEPROm copy (either auto-saved data, or two custom registers)
{

  const byte POS = 12;

  lcd.setCursor(POS, line);

  switch (mode)
  {
    case 0:
      g.new_value = 0;
      break;

    case 1:
      g.edit = 1;
      break;

    case 3:
      if (g.new_value < 2)
        g.new_value++;
      break;

    case 2:
      if (g.new_value > 0)
        g.new_value--;
      break;

    case 4:
      g.edit = 0;
      g.exit_menu = 1;
      for (byte i = 0; i < N_SENSORS; i++)
      {
        // Reading the training data from one of the three EEPROM registers:
        if (g.new_value == 0)
          EEPROM.get(ADDR_DATA1 + g.addr_tr[i] - ADDR_DATA, sensor[i].train);
        else if (g.new_value == 1)
          EEPROM.get(ADDR_DATA2 + g.addr_tr[i] - ADDR_DATA, sensor[i].train);
        else
          EEPROM.get(ADDR_DATA3 + g.addr_tr[i] - ADDR_DATA, sensor[i].train);
        // Writing the loaded data to EEPROM:
        EEPROM.put(g.addr_tr[i], sensor[i].train);
      }
      return;
  }

  if (g.new_value == 0)
    lcd.print("Auto");
  else if (g.new_value == 1)
    lcd.print("Reg1");
  else
    lcd.print("Reg2");

  return;
}


void menu_save_data (byte mode, byte line)
// Save training data to one of the two custome registers in EEPROM
{

  const byte POS = 12;

  lcd.setCursor(POS, line);
  if (mode == 3)
    mode = 2;

  switch (mode)
  {
    case 0:
      g.new_value = 0;
      break;

    case 1:
      g.edit = 1;
      break;

    case 2:
      g.new_value = 1 - g.new_value;
      break;

    case 4:
      g.edit = 0;
      g.exit_menu = 1;
      for (byte i = 0; i < N_SENSORS; i++)
      {
        // Saving the training data to one of the two EEPROM registers:
        if (g.new_value == 0)
          EEPROM.put(ADDR_DATA2 + g.addr_tr[i] - ADDR_DATA, sensor[i].train);
        else
          EEPROM.put(ADDR_DATA3 + g.addr_tr[i] - ADDR_DATA, sensor[i].train);
      }
      return;
  }

  if (g.new_value == 0)
    lcd.print("Reg1");
  else
    lcd.print("Reg2");

  return;
}


