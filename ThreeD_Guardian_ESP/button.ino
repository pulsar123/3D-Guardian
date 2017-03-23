void button()
{
  if (panic == (byte)1)
    // The Panic button was pressed
  {
    panic = (byte)0;
    // Send the SHUTDOWN command to Arduino via serial connection:
    Serial.print(MAGIC_EtoA"S");
  }
  return;
}


void buttonIntr()
// Reading the Panick button, sending serial STOP command to Arduino if pressed
// New: now it is interrupt-driven, and only modifies the volatile variable panic
// (serial communication should take place elsewhere).
{

  panic = (byte)1;

  return;
}

