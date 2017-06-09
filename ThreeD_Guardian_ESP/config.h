// Config file for ThreeD_Guardian_ESP project

// IMPORTANT: you must create file private.h in the current directory, and place the following lines there
// (uncomment the lines, and put the values suitable for your network)
//const char* ssid = "XXX";
//const char* password = "xxxx";
//const char* mqtt_server = "xxxxxx";

//#define DEBUG

#include "private.h"

// The device name for MQTT communications:
#define MQTT "3d_printer"

// External LED - WiFi connection indicator (also used to indicate problems with the serial connection, via flashes):
const byte LED1 = 2;  // D4/2

// Pin for the Panick button (needs an external pullup resistor of 10k to +3.3V):
const byte BUTTON_PIN = 14; // D5

// Thermistor stuff 
const long DT_TH = 100; // raw temperarture measurement interval, ms
const int N_T = 10; // average temperature over this many measurements (so the actual temperature is updated every N_T*DT_TH ms)
const byte TH_PIN = A0; // The analogue pin used:
const float R_PULL = 45900; // Pullup  resistor (Ohms) used with the 50k thermistor on A0 pin
/* NodeMCU devkit v0.9 uses an internal voltage divider based on two resistors - 100k and 220k - to convert the 0...3.3V input voltage range
   to 0...1V range required by the ESP chip (https://github.com/nodemcu/nodemcu-devkit). This creats a pulldown resistor of 320k on A0 pin.
   This needs to be corrected for. If your board doesn't have this issue, put a very large number here (1e9). But then it is your responsibility
   to wire your thermistor in such a way that ESP chip will not get >1V.
*/
const float R_INTERNAL = 320000;
// Parameters for my thermistor (50k, NTC MF58 3950 B from ebay)
const float TH_A = 3.503602e-04; // Two coefficients for conversion of the thermistor resistance to temperature,
const float TH_B = 2.771397e-04; // 1/T = TH_A + TH_B* ln(R), where T is in Kelvins and R is in Ohms
// Analogue raw measurements on pin A0, corresponding to high (connected to +3.3V directly - important because of the internal voltage divider on A0)
// and low (connected to ground directly) situations
// In theory should be 1023 and 0, respectively, but for some reason the real values are slightly different
const int A0_HIGH = 995;
const int A0_LOW = 1;

const long SERIAL_DT = 100;  // How frequently incoming serial communication is to be checked (ms)
const long SERIAL_TIMEOUT = 2000; // Maximum allowed number of ms with no received serial communications; LED starts blinking if it is exceeded
const long LED1_DT = 250; // Blinking interval in ms for LED1 when there is no serial connection
const byte MAX_COMMAND = 20; // Maximum number of commands which can be received in a single serial communication:
byte i_command[MAX_COMMAND]; // starting indexes of the incoming serial commands
byte bytes[MAX_COMMAND]; // number of bytes in each command

long int DT_DEBOUNCE = 200; // Debouncing time for the panic button, in ms

// Magic prefixes to identify serial communications
#define MAGIC_EtoA "E->A" // ESP8266 - > Arduino communications
#define MAGIC_AtoE "A->E" // Arduino -> ESP8266 communications

WiFiClient espClient;
PubSubClient client(espClient);

const int BUF_SIZE = 130;
char packet_old[BUF_SIZE], packet[BUF_SIZE];
char debug[BUF_SIZE];
char str[10];
byte led1;
long t, t0, t_a0;
byte first_send, first_packet;
int i_ser;
char s_char;
float sum_T, T_avr;
int i_T, T_int, T_dec, i_mqtt_T;
byte WiFi_on, MQTT_on;
byte mqtt_init;
volatile byte panic; // Needs to be volatile as it is used by an interrupt function
long int t_serial, t_led1;
byte timeout;
byte blink_state;
