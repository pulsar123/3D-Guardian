void LED()
// Using LED for warnings - specifically if serial connection is not working (timed out)
{

  // The timeout criterion:
  if (t - t_serial > SERIAL_TIMEOUT)
  {
    if (timeout == 0)
      t_led1 = t;

    if (t - t_led1 > LED1_DT)
      // Every LED1_DT ms, alternate the state of LED1
    {
      t_led1 = t;
      digitalWrite(LED1, blink_state);
      blink_state = 1 - blink_state;
    }

    timeout = 1;
  }

  else
  {
    if (timeout == 1)
      // We just exited  the warning  state, need to recover the prior LED1 state
      digitalWrite(LED1, led1);

    timeout = 0;
  }


  return;
}

