/*
  File containing all user-configurable input parameters. It is read once, inside config.h file.
*/

// If defined, the serial interfaced is used to continuously print out the sensor data (instead of communicating with the ESP board).
// The physical serial connection between the two boards should be disabled.
//#define DEBUG

// +++++++++++++++++++++++++  Pins  +++++++++++++++++++++++++++++
// Assumes we are using Arduino Nano
// ATTENTION: in Arduino Nano, pins A6 and A7 cannot be used as digital pins (only as analogue input pins)
// The pins 0 and 1 (Rx0 and Tx1) are reserved for serial communications with the WiFi module (esp8266 nodemcu devkit v0.9, or similar).
// Don't use the Arduino's 3.3V out voltage for ESP - it doen't have the power to drive the ESP module. Instead, get the "esp8266 nodemcu devkit v0.9"
// module (3.50$ on ebay) and power it by +5V you can take from the Arduino Nano. (So both Arduino and ESP will be powered by a single micro USB cable.)

const byte SOUND_PIN = 2; // Speaker pin (digital out); needs some elements to be usable for a piezo buzzer control
const byte FAN_PIN = 3; // Pin to control the exhaust fan in the printer's enclosure (connect it to the control - PWM - wire of a 4-wire fan)

// Pins used by the display (LCD 1602);
// This is the one I'm using: https://www.dfrobot.com/wiki/index.php/Arduino_LCD_KeyPad_Shield_(SKU:_DFR0009)
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// Stepper driver pins (exhaust outlet control); in A4988 driver, connect RESET pin to the adjacent SLEEP pin, and use 100uF capacitor across the motor voltage pins
// ( https://www.pololu.com/product/1182 )
// The DIR pin of the driver has to be connected to either +5V or Ground, for the desired direction of the motor move (from the default state - exhaust closed, to exhaust open)
const byte SLEEP_PIN = 10; // Control of Sleep function of the stepper motor driver (A4988, EasyDriver and similar); LOW: disable motor; HIGH: enable motor; needs 1ms delay after waking up
const byte STEP_PIN = 11; // Control of Step for the motor driver

const byte SSR_PIN = 12; // Pin to control the solid state relay in the external power module (to shut down the printer in case of alarm)
const byte LEDY_PIN = 13; // Yellow LED (training mode) pin (TX1), via 220 Ohm

// Analogue pins:
const byte BUTTONS_PIN = A0; // The analog input pin for the buttons on the display
// The remaining seven analogue pins (A1 ... A7) of Arduino Nano are assigned to up to seven environment sensors below (in "Sensor stuff")

/* Sensor stuff (between one and six sensors with Arduino Nano)
   Types are as follows:
     - 0: raw data sensors (range 0...1023): smoke (MQ-2), CO (MQ-8), etc.
     - 1: alarm thermistors (using Celsius degrees)
     - 2: enclosure thermistor (used both for alarms and for the thermostate - keeps the temperature inside the printer cabinet constant by operating the fan)
     - 3: resistance sensor (current sensor + voltage sensor) - for heated bed or hot end
   Attention: type=3 sensors will not be enabled until they are manually trained from the menu, "Training"/"Init current" (detecting the raw current sensor measurement for zero current)
   Currently only one resistance sensor is supported. Resistance sensors use two analogue pins (for current and voltage sensing, .pin and .pin2, respectively).
   Voltage divider for the voltage sensing part should be such that the heated bed voltage (say, 15V) would be converted to Arduino-safe voltage (<5V), accounting for potential spikes
   in the measured voltage. Don't use smoothing capacitors here as we use very fast (~14 us) voltage/current sensing. E.g., if your voltage is 15V, use R1=39k and R2=10k (R1 is connected
   to +Vin), which will divide by ~5x, so 15V becomes ~3V (=626 in raw units).
*/

const byte N_SENSORS = 7; // Number of sensors
// Basic sensor data (they will be used in this order):
struct sensor_struc sensor[N_SENSORS] = {
  {name: "Smo", pin: A2, type: 0, init_delay: 600000}, // Smoke sensor (MQ-2; raw data)
  {name: "CO ", pin: A3, type: 0, init_delay: 600000}, // CO sensor (MQ-8; raw data)
  {name: "Cur", pin: A7, type: 0, init_delay: 0}, // current sensor (ACS712; raw data)
  {name: "Vol", pin: A6, type: 0, init_delay: 0}, // current sensor (ACS712; raw data)
  {name: "Th1", pin: A1, type: 2, init_delay: 0}, // Enclosure thermistor (Celcius data)
  {name: "Th2", pin: A4, type: 1, init_delay: 0}, // Motherboard thermistor (Celcius data)
  {name: "PSU", pin: A5, type: 1, init_delay: 0} // PSU thermistor (Celcius data)
//  {name: "Bed", pin: A7, type: 3, init_delay: 0, scaler: 0.1, pin2: A6, divider: 0.3} // Heated bed resistance sensor (ACS712 sensor for the current, voltage divider for the voltage)
};
const int SENS_DT = 10; //  Read sensors every SENS_DT ms
const int SENS_N = 100; // Compute the current sensor value by averaging over this many measurements
const float V_CRIT = 6; // Critical voltage (Volts) used in resistance sensors. Should be ~50% of the normal heated bed / hot end voltage
const float R_BED = 1.1; // Resistance of the bed (Ohms)

// Alarm and warning constants (only used in GUARDING mode)
// For type=0 and type=3 sensors:
// (Both following alarm criteria have to be satisfied before an alarm is triggered)
const float ALARM_MAX = 2.0; // If the deviation of the raw sensor value above the trained average value (avr) is larger than ALARM_MAX*(sensor_max-avr), trigger an alarm (also see the next parameter)
const int ALARM_CONST = 10; // Alarm is only triggered if the raw sansor value is at least ALARM_CONST raw units above the trained average value
// Warnings are used to indicate a possible failed sensor, or as a precursor to an alarm:
const float WARN_MAX = 1.5; // If the deviation of the raw sensor value above the trained average value (avr) is larger than WARN_MAX*(sensor_max-avr), trigger a warning
const float WARN_MIN = 2.0; // If the deviation of the raw sensor value below the trained average value (avr) is larger than WARN_MIN*(avr-sensor_min), trigger a warning
const int WARN_CONST = 5; // Similar to ALARM_CONST; to be triggered, a warning needs the raw sansor value being at least ALARM_CONST raw units above or below the trained average value
// For thermistor sensors (type=1/2):
const int ALARM_DTEMP = 20; // If thermistor temperature goes above training max temperature plus this factor (C), trigger alarm
const int WARN_DTEMP = 10; // If thermistor temperature goes below trained min minus this factor (C) or above trained max temperature plus this factor (C), trigger warning

const long int STORE_DT = 60000; // How often to store sensor data to EEPROM, ms

// Display/keys stuff:
const long int SCREEN0_DT = 15000; // Time out after a key press to default screen, ms
const long int KEYS_DT = 10; //  Read keys every this many ms
const long int N_KEY_COUNT = 10; // Accept a key press if it was continuously read this many times
const long int N_KEY_START = 100; // After this many continuous reads of a key, start the key auto-repeating mode
const long int N_KEY_REPEAT = 20; // Number of key reads for each repeat

// Temperature stuff:
// The range of allowed target temperatures inside the enclosure (sensor with the type = 2), Celsius:
const int T_TARGET_MIN = 10;
const int T_TARGET_MAX = 80;
// This is a simplified approach - assumes that all the thermistors we use are identical, and that all the analogue pins the thermistors
// are connected to have identical pullup resistance values.
// The parameters for the thermistor(s):
const float R_A1 = 35980; // Resistance of the internal pullup resistor, Ohm (you have to measure it for your Arduino yourself)
const float TH_A = 3.503602e-04; // Two coefficients for conversion of the thermistor resistance to temperature,
const float TH_B = 2.771397e-04; // 1/T = TH_A + TH_B* ln(R), where T is in Kelvins and R is in Ohms

// Sound (piezo buzzer) stuff (also applies to the red LED):
const long int SP_DT = 500; // Half-period of the buzzer signal, ms; also used for the red LED blinking
const long int CHIRP_DT = 25; // Duration of a chirp sound (ms); used in WARNING mode; also used for red LED "chirping"
const long int CHIRP_PERIOD = 10000; // Time interval (ms) between chirps, in WARNING mode

// Exhaust fan stuff (controlled via PWM at 31 kHz)
//  If defined, it is assumed that the fan doesn't have PWM  control, and can only be truned on (g.duty=255) and off (g.duty=0).
//  In this case FAN_PIN turns on/off the fan via a MOSFET
//#define NO_PWM
const int DT_CASE_MIN = 10; // Shortest allowed time for fan to run (seconds) during case clearing
const int DT_CASE_MAX = 990; // Longest allowed time for fan to run (seconds) during case clearing
const float FAN_SCALE = 10.0; // Sensitivity of the fan; change in fan's duty cycle for each 1C temperature difference, at each cycle
const byte MAX_DUTY = 255; // Maxium allowed fan's duty cycle; can be 0...255
const long int FAN_DT = SENS_DT * SENS_N; // Time interval to update the fan's speed = time interval for temperature recalculations
const float FAN_HYST = 0.5; // Fan's hysteresis, in Celcius (temperature is now allowed to drift beyond T_target+-FAN_HYST)

// Stepper motor (exhaust outlet open/close) stuff
// Number of full steps to take when going from the passive default (closed) to the active open state. For the stepper motors
// with 200 steps per rotation, given the desired rotation angle A in degrees, N_FULL_STEPS = (A/360) * 200
const int N_FULL_STEPS = 44;
// Number of microsteps per step (hardwired  - see https://www.pololu.com/product/1182 ; for 16 microsteps on A4988, put 10k pullup resistors
// on MS1, 2, and 3). Higher numbers are better for the mechanics (less vibrations), but for small T_MOTOR you can start loosing steps
const int N_MICROSTEPS = 16;
const float T_MOTOR = 0.5; // How many seconds a full (closed->opened) move takes
const long int DT_RELEASE = 3000; // Number of milliseconds it takes the exhaust lid to settle down after having being released

// Serial (ESP8266) stuff
const long int SERIAL_IN_DT = 100; // How frequently incoming serial communication is to be checked (ms)
const long int SERIAL_OUT_DT = 500; // How frequently outgoing short serial communication is to be sent (ms)
const int N_SERIAL_MAX = 10; // Every this many short serial out communications send one long (full sensor update) one

