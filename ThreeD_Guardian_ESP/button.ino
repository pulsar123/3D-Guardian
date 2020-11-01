void button()
{
  #ifdef NO_INTERRUPTS
  if (digitalRead(BUTTON_PIN) == LOW && millis()>t_panic+DT_DEBOUNCE)  
    // The Panic button was pressed
  {
    t_panic = millis();
    panic = (byte)1;
    // Send the SHUTDOWN command to Arduino via serial connection:
    Serial.print(MAGIC_EtoA"S");
  }

  #else
  if (panic == (byte)1)
  
    // The Panic button was pressed
  {
    panic = (byte)0;
    // Send the SHUTDOWN command to Arduino via serial connection:
    Serial.print(MAGIC_EtoA"S");
  }
  #endif

  return;
}

#ifndef NO_INTERRUPTS
void buttonIntr()
// Reading the Panick button, sending serial STOP command to Arduino if pressed
// New: now it is interrupt-driven, and only modifies the volatile variable panic
// (serial communication should take place elsewhere).
{

  panic = (byte)1;

  return;
}
#endif
