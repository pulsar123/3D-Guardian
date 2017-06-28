/*
  The system configuration file for 3D Guardian code. Normally nothing needs to be changed here. All the user configurable stuff is in a separate file, user_config.h.
*/

#ifndef _3D_GUARDIAN_H
#define _3D_GUARDIAN_H

// Structure for writing to/reading from EEPROM of a single sensor data in training mode. In guarding mode, this data is only stored in RAM, not in EEPROM.
// The EEPROM data will be updated (written to EEPROM) every STORE_DT ms (currently 1 min)
struct sensor_EEPROM_struc
{
  int min; // Smallest sensor reading from training sessions
  int max; // Largest sensor reading from training sessions
  int zero; // Raw value corresponding to zero current (only for current/resistance sensors)
  unsigned long int sum; // Running sum for sensor readings from training sessions (warning: this can overflow after very long training time; with the current settings, it'll happen after ~100 days of training)
  unsigned long int N; // Number of sensor readings from training sessions
};

// Structure which keeps all sensor data, for each sensor, in memory:
struct sensor_struc
{
  char name[4]; // Short (3-letter) name of the sensor
  byte pin; // Input analogue pin used for the sensor
  byte type; // Type of sensor (0: raw data sensor - CO, smoke, IR etc; 1: thermistor sensor; 2: one thermistor sensor used to control enclosure temperature with a fan; 3: resistivity sensor consisting of current and voltage sensors)
  unsigned long int init_delay; // Initial delay in ms before the sensor starts reading (used mostly for CO/smoke sensors, which need some warm up time)
  // Parameters only used for a resistance sensor (current sensor + voltage sensor):
  float scaler; // V/A scaler (sensitivity) for the current sensor; =0.1 V/A for the 20A model of ACS712
  byte pin2; // Second analogue pin (only used for a resistance sensor, to measure voltage)
  float divider; // Voltage dividing factor, to convert from the heated bed voltage to <5V
  byte on; // The sensor readings enabled (1) / disabled (0)
  sensor_EEPROM_struc train; // Training data for EEPROM
  sensor_EEPROM_struc guard; // Guarding data (only volatile memory copy, no EEPROM copy)
  int avr; // Current value (averaged over SENS_N measurements, each performed every SENS_DT ms)
  int old; // Averaged value from the previous Arduino loop (used to determine when to refresh the screen)
  unsigned long int t0; // Last time the current average was computed, ms
  unsigned int i; // Counter of the sensor readings
  unsigned long int sum; // Current sum of readings
  int train_avr; // Averaged trained value
  int warn_min; // If sensor value goes below this, trigger warning (bad sensor)
  int warn_max; // If sensor value goes above this, trigger warning (approaching alarm state)
  int alarm_max; // If sensor value goes above this, trigger alarm
  int V_crit_raw; // V_CRIT expressed in raw units
};

// Reading user-provided configuration parameters:
#include "user_config.h"

// Motor stuff:
const int N_STEPS = N_FULL_STEPS * N_MICROSTEPS; // Number of microsteps for the full move
const float ACCEL = 1e-12 * 4.0 * N_STEPS / (T_MOTOR*T_MOTOR); // Acceleration used for the motor in microsteps/microsecond^2
const long int T_MOTOR_US = (long int)(T_MOTOR * 1e6 + 0.5); // Full move in microseconds

// EEPROM stuff:
// EEPROM addresses: make sure they don't go beyong the Arduino Uno EEPROM size of 1024! From my experience, one needs to provide at least two bytes
// for EEPROM storage - even for 1-byte types (like char or byte)
const int ADDR_ALARM = 0;  // Last good g.alarm state (can be either TRAINING or GUARDING)
const int ADDR_T_TARGET = ADDR_ALARM + 2;
const int ADDR_FAN_MODE = ADDR_T_TARGET + 2;
const int ADDR_DT_CASE = ADDR_FAN_MODE + 2;
const int ADDR_MANUAL_FAN = ADDR_DT_CASE + 2;

// Address where the training data starts:
const int ADDR_DATA = ADDR_MANUAL_FAN + 2;
// Address were the first copy (autosaved) of the trained data starts at:
const int ADDR_DATA1 = ADDR_DATA + N_SENSORS*sizeof(sensor_EEPROM_struc);
// Address were the second copy (first memory register) of the trained data starts at:
const int ADDR_DATA2 = ADDR_DATA1 + N_SENSORS*sizeof(sensor_EEPROM_struc);
// Address were the third copy (second memory register) of the trained data starts at:
const int ADDR_DATA3 = ADDR_DATA2 + N_SENSORS*sizeof(sensor_EEPROM_struc);


//+++++++++++++++++++ Menu stuff +++++++++++++++++++++++++++++++
// All the runtime menu stuff is in the menu.ino file.

typedef void (*CustomFunc)(byte mode, byte line);

// Structure for dynamical hierarchical menu, to store a single menu item (node). For the description of the variables, see below.
struct MenuItem_struc {
  char *name;
  byte id;
  byte prev_id;
  byte next_id;
  byte up_id;
  byte down_id;
  CustomFunc func;
};

// Index corersponding to "no id" in menu items (don't use this number as a menu item id!):
const byte NONE = 255;

/* Menu functions declarations (all functions used in MenuItem elements below).
   The function is called by menu_core() routine. Each function has two arguments:
  - mode:
   0: when displaying the menu item
   1: first RIGHT click (enters edit mode, or can be the only command, for an immediate action)
   2: UP click (increases the variable value in edit mode)
   3: DOWN click (decreases the variable value in edit mode)
   4: second RIGHT click (ends the edit mode, saves the variable, optionally perform an action on the new value)
  - line: the current line of the LCD screen (0 or 1)
*/
void menu_printer (byte mode, byte line);
void menu_train_onoff (byte mode, byte line);
void menu_update_train (byte mode, byte line);
void menu_clear_case (byte mode, byte line);
void menu_fan_control (byte mode, byte line);
void menu_factory_reset (byte mode, byte line);
void menu_T_target (byte mode, byte line);
void menu_dt_case (byte mode, byte line);
void menu_manual_fan (byte mode, byte line);
void menu_zero_voltage (byte mode, byte line);
//void menu_train_dump (byte mode, byte line);
//void menu_train_load (byte mode, byte line);
void menu_init_all (byte mode, byte line);
void menu_init_one (byte mode, byte line);
void menu_load_data (byte mode, byte line);
void menu_save_data (byte mode, byte line);

// Total number of menu items (listed in the "struct MenuItem_struc MenuItem[N_MENU]" initialization below)
const byte N_MENU = 18;
// Maximum menu depth:
const byte MENU_DEPTH = 3;

/* The list of menu items. It doesn't have to be sorted in any particular order, so it's easy to add new items anywhere or remove old items.
   Important: the actual id's used in the program will be different (sequential starting from 0); don't use the specific ids from this list - ids are
   used here only to establish the relationship between menu elements!
   The structure elements:
   .name: The string which will be shown on the display starting at column position 1 (position 0 is reserved for special characters - EMPTY_SQUARE and FILLED_SQUARE);
   .id: an arbitrary id (0..254) assigned to this menu item. Doesn't need to follow any particular order. Cannot be equal to NONE (255), which is reserved to "no id" situations.
   .prev_id: the id of the previous (displayed above) menu item at the same level; NONE if none.
   .next_id: the id of the next (displayed below) menu item at the same level; NONE if none.
   .up_id: the id of the parent (one level up) menu item; NONE if none.
   .down_id: the id of the first (topmost) child id; NONE if none.
   .func: the action function associated with this menu item. It has one argument (mode; see above for its description).
*/
struct MenuItem_struc MenuItem[N_MENU] = {
  // Top (first) level:
  {.name = "Fan",            .id =  2, .prev_id = NONE, .next_id =    1, .up_id = NONE, .down_id =   7},
  {.name = "Training",       .id =  1, .prev_id =    2, .next_id =   15, .up_id = NONE, .down_id =   4},
  {.name = "Printer",        .id = 15, .prev_id =    1, .next_id =    3, .up_id = NONE, .down_id = NONE, .func = menu_printer},
  {.name = "Config",         .id =  3, .prev_id =   15, .next_id = NONE, .up_id = NONE, .down_id =  10},

  // Second level:
  {.name = "Clear case",     .id =  7, .prev_id = NONE, .next_id =    8, .up_id =    2, .down_id = NONE, .func = menu_clear_case},
  {.name = "Fan mode",       .id =  8, .prev_id =    7, .next_id = NONE, .up_id =    2, .down_id = NONE, .func = menu_fan_control},

  {.name = "On/off",         .id =  4, .prev_id = NONE, .next_id =    5, .up_id =    1, .down_id = NONE, .func = menu_train_onoff},
  {.name = "Update",         .id =  5, .prev_id =    4, .next_id =   12, .up_id =    1, .down_id = NONE, .func = menu_update_train},
  {.name = "Init voltage",   .id = 12, .prev_id =    5, .next_id =   18, .up_id =    1, .down_id = NONE, .func = menu_zero_voltage},
  {.name = "Re-init",        .id = 18, .prev_id =   12, .next_id =   21, .up_id =    1, .down_id =   19},
  {.name = "Load data",      .id = 21, .prev_id =   18, .next_id =   22, .up_id =    1, .down_id = NONE, .func = menu_load_data},
  {.name = "Save data",      .id = 22, .prev_id =   21, .next_id = NONE, .up_id =    1, .down_id = NONE, .func = menu_save_data},
//  {.name = "Train dump",     .id = 16, .prev_id =   18, .next_id =   17, .up_id =    1, .down_id = NONE, .func = menu_train_dump},
//  {.name = "Train load",     .id = 17, .prev_id =   16, .next_id = NONE, .up_id =    1, .down_id = NONE, .func = menu_train_load},

  {.name = "T_target",       .id = 10, .prev_id = NONE, .next_id =   11, .up_id =    3, .down_id = NONE, .func = menu_T_target},
  {.name = "Fan time",       .id = 11, .prev_id =   10, .next_id =   23, .up_id =    3, .down_id = NONE, .func = menu_dt_case},
  {.name = "Manual fan",     .id = 23, .prev_id =   11, .next_id =    6, .up_id =    3, .down_id = NONE, .func = menu_manual_fan},
  {.name = "Reset",          .id =  6, .prev_id =   23, .next_id = NONE, .up_id =    3, .down_id = NONE, .func = menu_factory_reset},

  // Third level
  {.name = "All sensors",    .id = 19, .prev_id = NONE, .next_id =   20, .up_id =   18, .down_id = NONE, .func = menu_init_all},
  {.name = "One sensor",     .id = 20, .prev_id =   19, .next_id = NONE, .up_id =   18, .down_id = NONE, .func = menu_init_one}
};


// See also the g.default_id variable - part of the "g" structure below.
//+++++++++++++++++++++++++++++++++++ end of menu stuff +++++++++++++++++++++++++++++++++++++++++++

// Mode constants :
const char PROG = -3; // Programming mode (first a few seconds after booting, and if the serial connection Arduino - ESP is interrupted for firmware upgrade)
const char TRAINING = -2; // Training mode (no alarms; memorizing the highest sensor values; fan thermostat works)
const char WARNING = -1; // Warning mode (can happen while guarding; indicates non-critical issues - a failed sensor etc.; you can still print; fan thermostat works)
const char GUARDING = 0; // Guarding (normal) mode (monitoring sensors, can trigger alarm or warning; fan thermostat works)
const char ALARM = 1; // Ararm mode (critical issue detected, printer shut down, siren sounds, exaust closed; fan thermostat doesn't work; can only be reset by power cycling the controller)

// Display stuff:
// define some values used by the panel and buttons
const char NOKEY  = 0;
const char RIGHT = 1;
const char UP    = 2;
const char DOWN  = 3;
const char LEFT  = 4;
const char SELECT = 5;
// Use the following three special characters the following way:
//      lcd.write(R_ARROW);
//      lcd.write(byte(0));
//      lcd.write(byte(1));
const byte R_ARROW = 0b01111110;  // Or try 0b00011010 if the other one doesn't work on your display
const byte EMPTY_SQUARE[8] = {
  B00000,
  B11111,
  B10001,
  B10001,
  B10001,
  B11111,
  B00000,
  B00000
};
const byte FILLED_SQUARE[8] = {
  B00000,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B00000,
  B00000
};

// Size of the char array g.buffer:
const int BUF_SIZE = 130;

// Magic prefixes to identify serial communications
#define MAGIC_EtoA "E->A" // ESP8266 - > Arduino communications
#define MAGIC_AtoE "A->E" // Arduino -> ESP8266 communications
// Maximum number of commands which can be received in a single serial communication:
const byte MAX_COMMAND = 10;

// Most global variables belong to one structure - global:
struct global
{
  int addr_tr[N_SENSORS]; // Training data (the actual addresses are computed at runtime in initialize())
  byte default_id = 2; // The id of the menu item on the first line of the top level screen, in the "struct MenuItem_struc MenuItem[N_MENU] =" initialization above
  char alarm; // Current state: 0 (GUARDING) - normal state; 1 (ALARM) - alarm was triggered; -3 (PROG) - programming mode; -2 (TRAINING) - training mode; -1 (WARNING) - warning mode; saved in EEPROM (only if -2 or 0)
  char alarm_ini; // Initial value of alarm read from EEPROM at boot time

  char screen; // Screen number; -1 for the main menu, 0 for the default screen, >0 for alternative screens (accessed via UP/DOWN from teh default screen)
  byte menu_id; // The id of the currently active (highlighted) menu item
  byte menu_top_id; // The id of the top line item in the currently displayed menu
  byte old_id[MENU_DEPTH - 1]; // The parent screen's id, when we descend one level down
  byte old_top_id[MENU_DEPTH - 1]; // The parent screen's top id, when we descend one level down
  byte menu_level; // Menu level (0: top level; >0: lower levels)
  byte edit; // Menu value edit mode; 0: no editing (UP/DOWN scroll the menu); 1: editing (UP/DOWN change the value)
  int new_value; // the unsaved new value of the variable which is being edited
  byte exit_menu; // 1 if exiting the menu to default screen

  char th_case; // Case thermistor index (with the type=2; will control fan to maintain a target case temperature)
  float T; // Case temperature in Celsius

  long int store_t0; // last time data were stored to EEPROM

  byte refresh_display; // 1 if we need to refresh display in the current loop; 0 otherwise

  long int t; // current time in us
  long int t0_init; // Bootup time, us
  byte key; // Currently pressed button (O..5)
  byte key_old; // Key pressed at the previous arduino loop
  byte key_pressed; // 1 if a key was just pressed; 0 otherwise
  byte key_released; // 1 if a key was just released; 0 otherwise
  long int keys_t0; // Last time keys were read
  long int key_t0; // Time when a key was pressed last time;
  long int key_count; // Counting the number of key reads with a key continuously pressed

  long int sp_t0; // Time of the last buzzer state change
  byte sp_state; // The buzzer state (0/1)

  byte fan_mode; // Fan mode: 0: off; 1: auto (thermostat); 2: on; saved in EEPROM
  byte fan_mode_old; // A copy of fan_mode
  byte case_clearing; // A special fan mode: running at maximum duty for g.dt_case seconds (to clear the cabinet of fumes after the printing)
  int dt_case; // Number of seconds for the case_clearing mode; saved in EEPROM
  long int case_t0; // Time point when case clearing was initiated, ms
  int duty; // duty cycle for the fan (0...255; 0 is off, 255 is full speed)
  byte duty_perc; // g.duty expressed as percentage of MAX_DUTY
  byte duty_perc_old; // Previous loop value of duty_perc
  long int fan_t0; // last time fan's speed was checked/changed
  int T_target; // the target enclosure temperature in Celsius; saved in EEPROM
  byte manual_fan; // The fan speed in the manual fan mode (1...254); only makes sense when PWM is used

  byte LEDy_state; // Yellow LED state
  long int prog_led_t0; // Last time the yellow LED state changed (in blinking - PROG - mode)

  long int serial_in_t0; // Last time serial communication was received
  long int serial_out_t0; // Last time serial communication was sent
  byte i_command[MAX_COMMAND]; // starting indexes of the icoming serial commands
  int N_serial; // Counter of serial sends (to figure out when to send long -less frequent - updates)
  byte SSR_temp; // Initially 0; becomes 1 once the first SSR temperature value was received via serial from ESP
  char i_SSR; // Index of the SSR temperature "sensor"
  int T_SSR; // SSR temperature in C received from ESP via serial connection
  long int t_SSR; // Last time SSR temperature was received via serial interface
  byte serial_alt; // experimental
  byte prog_on; // =1 when the PROG mode was initiated

  int motor; // Exhaust motor's next step (1...N_STEPS; motor disabled if 0)
  long int t0_motor; // Moment when the motor got the command to start moving, in us
  long int t_next_step; // Time to make the next step
  int step; // Current microstep (1...N_STEPS)
  long int t_us; // Microseconds timer
  long int t_release; // The time (ms) when the exhaust motor was released

  int resistance_sensor; // The ID of the resistance sensor (type=3)
  byte resistance; // 0: if resistance hasn't been measured yet since reboot/reset (because bed wasn't heated yet); 1: was measured at least once

  byte printer; // 0/1 if the printer is off/on
  byte bad_sensor; // The id of the sensor - biggest offender when a warning is issued

  char buffer[BUF_SIZE];  // Buffer for strings
  char buf1[4]; // Temp buffers
  char buf2[4]; // Temp buffers
  char buf3[4]; // Temp buffers
  char buf4[4]; // Temp buffers
};

struct global g;


// This fixes a weird "unable to find a register to spill" error (solution found in https://github.com/arduino/Arduino/issues/3972 )
void display() __attribute__((__optimize__("O2")));

#endif

