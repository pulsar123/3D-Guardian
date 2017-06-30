void LED()
// Using LED for warnings
{

  if (no_cable)
    // The SSR cable is disconnected - series of two green LED flashes
  {
    if (t - t_led1 > LED1_DT)
    {
      t_led1 = t;
      nocable_step++;
      if (nocable_step > 6)
        nocable_step = 1;
      // Setting up a two-flash sequence, with the gap between sequences = 2x gap between the two flashes
      if (nocable_step == 1 || nocable_step == 3)
        led1 = 1;
      else if (nocable_step == 2 || nocable_step == 4)
        led1 = 0;
      else
        return;
      digitalWrite(LED1, led1);
    }
    return;
  }

  return;
}

