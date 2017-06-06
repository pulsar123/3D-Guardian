/* Everything related to the exhaust outlet control. Using a stepper motor to open the outlet when ordered (with holding torque to keep it in place), and releasing the torque when the outlet needs to be closed (using gravity).

   The exhaust fan speed is the primary (g.duty is calculated in fan()), this motor function is seconday: it reacts to any changes in g.duty.

   If g.duty just became > 0 (meaning we just turned the fan on), this is the signal for the motor to start opening the exhaust lid. (Only if at least DT_RELEASE milliseconds have passed since the last time the lid was released - to allow it to settle down.)

   The lid's stepper motor's move is simple: for the first half of N_STEPS microsteps it accelerates at a constant rate (ACCEL), and for the second half it decelerates with the same constant rate.

   Once the lid is fully open (after T_MOTOR seconds), g.duty becoming zero will trigger the lid's release. DT_RELEASE milliseconds later the lid is ready to be fully open again.
*/

void motor ()
{

  if (g.alarm == PROG)
    return;

  // We need microsecond accuracy timer for the stepper motor control
  g.t_us = micros();

  if (g.motor == 0 && g.duty > 0 && g.t - g.t_release > DT_RELEASE)
    // Initiating the motor as the fan's duty just became larger than zero, at least DT_RELEASE milliseconds after having been released the previous time
  {
    g.motor = 1;
    // Motor will start turning after the specified delay, to ensure there is some negative pressure before opening the inlet and outlet:
    g.t0_motor = g.t_us + MOTOR_DELAY_MS;
    digitalWrite(SLEEP_PIN, HIGH); // Enabling the torque
    delay(1); // Required after waking up
    // The timing for the first microstep (us):
    g.t_next_step = t_motor (1);
  }

  if (g.motor > 0 && g.motor <= N_STEPS)
    // Continue moving (will always finish, regardless of g.duty)
  {
    if (g.t_us >= g.t_next_step)
      // Making the next step
    {
      digitalWrite(STEP_PIN, LOW);
      delayMicroseconds(2);
      digitalWrite(STEP_PIN, HIGH);
      g.motor++;
      g.t_next_step = t_motor (g.motor);
    }
  }

  // !!! Releasing even when not finished moving - to minimize escape through inlet; ideally fan still should be running a bit after this - to be implemented
  //  if (g.motor > N_STEPS && g.duty == 0)
  if (g.duty == 0)
    // g.duty just became zero, after a full movement, so time to release the torque
  {
    g.motor = 0;
    digitalWrite(SLEEP_PIN, LOW); // Disabling the torque
    // Memorizing the release time in milliseconds:
    g.t_release = g.t;
  }

  return;

}

