/*
  3D Guardian: Arduino-based environment monitoring / security module for a 3D printer.

  Documentation: http://pulsar124.wikia.com/wiki/3D_Guardian

  This code is for the Arduino Nano part.

  Designed by Sergey Mashchenko

*/

#include <math.h>
#include <stdio.h>
#include <EEPROM.h>
#include <LiquidCrystal.h>
#include "config.h"

/* Notes:
  - Thermistor 1: A1 pullup resistor is 35.98k. Measured in the interval +1 ... +100C, the Steinhart–Hart equation for my "NTC MF58 3950 B 50K" thermistor
   from ebay is
       1/T = 3.372535e-04 + 2.791091e-04*ln(R) - 6.351293e-09*[ln(R)]^3
    The last (cubic) term is statistically insignificant. Dropping it results in no loss of the accuracy, so I'll be using the following simpler equation:
       1/T = 3.503602e-04 + 2.771397e-04*ln(R)
    Here T is in Kelvin, and R is in Ohms. The uncertainty for the two numeric coefficients is 3% and 0.4%, respectively. The std for the temperature
    measurements is 0.4C for the whole interval (1...100C). The measured B coefficient is 3608K (the nominal value is 3950K), the measured resistance at
    25C is 50.9k (the nominal value is 50k).

    The final equation to compute the temperature (in Kelvin) based on the raw measurement at the pin A1, M=0...1023, is
       1/T = 3.503602e-04 + 2.771397e-04*ln(R_A1*M/(1023-M))
    The Celcius temperature is obtained by subtructing 273.15 from T.
*/


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void setup()
{
  // Changing the PWM frequency for the pin 11 (also 3) to 31,372 Hz (for fan control); timer 2
  TCCR2B = (TCCR2B & 0b11111000) | 0x01;

  // Increasing the ADC speed by 8x (https://www.gammon.com.au/adc):
  ADCSRA &= ~(bit (ADPS0) | bit (ADPS1) | bit (ADPS2)); // clear prescaler bits
  ADCSRA |= bit (ADPS2);                               //  16 scaler

  pinMode(SSR_PIN, OUTPUT);
  pinMode(SOUND_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(LEDY_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(SLEEP_PIN, OUTPUT);
  digitalWrite(SLEEP_PIN, LOW); // Initially no holding torque
  delay(1);
  digitalWrite(STEP_PIN, LOW);
  for (byte i = 0; i < N_SENSORS; i++)
  {
    if (sensor[i].type == 0)
      // Analogue input pins for raw sensors (smoke, CO, IR ...)
      pinMode(sensor[i].pin, INPUT);
    else if (sensor[i].type == 3)
      // For resistance "sensor", both current and voltage sensors need to be initialized
    {
      pinMode(sensor[i].pin, INPUT);
      pinMode(sensor[i].pin2, INPUT);
    }
    else
      // Internal pullup resistor for pins used for thermistors:
      pinMode(sensor[i].pin, INPUT_PULLUP);
  }

  // Initially the printer is on (can be turned off in case of an alarm, from WiFi or menu):
  digitalWrite(SSR_PIN, 1);
  g.printer = 1;

  // Computing EEPROM addresses for sensor data:
  g.addr_tr[0] = ADDR_DATA;
  for (byte i = 1; i < N_SENSORS; i++)
    g.addr_tr[i] = g.addr_tr[i - 1] + sizeof(sensor_EEPROM_struc);

  g.t = millis();
  g.t0_init = g.t;

  // Case fan stuff (initially off):
  g.duty = 0;
  g.duty_perc_old = 0;
  update_duty();
  g.fan_t0 = g.t;
  g.case_clearing = 0;
  g.case_t0 = g.t;

  // Stepper motor (exhaust opening) stuff:
  g.motor = 0;
  g.t_release = -DT_RELEASE;
  g.step = 0;

  // Buzzer stuff (initially off):
  g.sp_t0 = g.t;
  g.sp_state = 0;

  // Keypad stuff:
  g.keys_t0 = g.t;
  g.key_pressed = 0;
  g.key_released = 0;
  g.key = NOKEY;
  g.key_old = NOKEY;
  g.key_t0 = g.t;
  g.key_count = 0;

  // Initiating the LCD library (LCD1602: 16 characters x 2 lines):
  lcd.begin(16, 2);
  g.refresh_display = 1;
  g.screen = 0;
  g.exit_menu = 0;
  g.menu_id = g.default_id;
  g.menu_top_id = g.default_id;
  g.menu_level = 0;
  g.edit = 0;
  g.new_value = 0;
  for (byte k = 0; k < MENU_DEPTH - 1; k++)
  {
    g.old_id[k] = NONE;
    g.old_top_id[k] = NONE;
  }
  lcd.createChar(0, EMPTY_SQUARE);
  lcd.createChar(1, FILLED_SQUARE);

  // Checking if EEPROM was never used:
  if (EEPROM.read(2) == 255 && EEPROM.read(3) == 255)
    // Initializing with a factory reset (setting EEPROM values to the initial state):
    initialize(1);
  else
    // Initializing, with EEPROM data read from the device
    initialize(0);

  g.store_t0 = g.t;
  g.serial_in_t0 = g.t;
  g.serial_out_t0 = g.t;
  g.N_serial = N_SERIAL_MAX - 1;
  g.bad_sensor = 0;
  g.SSR_temp = 0;
  g.T_SSR = 0;
  g.t_SSR = g.t - DT_SSR_MAX - 1;
  g.prog_led_t0 = g.t;
  t_last_serial = g.t - DT_SSR_MAX - 1;
  // The serial connection to ESP8266 controller, for the WiFi interface:
  // (In DEBUG mode it is used to print sensor data to PC instead)
  Serial.begin(115200);

  // Initializing the menu items (this changes all ids in MenuItem[] structures)
  menu_init();

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void loop()
{
  g.t = millis();

  // Getting inputs:
  read_sensors(); // Reading all the analogue sensors
  read_LCD_buttons(); // Reading key inputs, and performing menu actions if requested
  serial(); // Experimental module to communicate with the WiFi module ESP8266 via serial connection

  // Processing:
  alarm(); // Deciding if to trigger an alarm or warning:
  store_data();  // Store training sensor data regularly to EEPROM

  // Actions:
  sound(); // Making sounds (with a piezo buzzer) and flashing red LED

  fan(); // Controlling the exhaust fan
  motor(); // Controlling the exhaust lid motor
  display(); // Displaying stuff


  // End of the loop cleanup:
  cleanup();
}

