#include <Wire.h>
#include <SPI.h>
#include <DHT.h>
#include <BH1750.h>
#include <OneWire.h>
#include <DallasTemperature.h>

//#include <clsPCA9555.h>
#include <Ethernet.h>
#include <EEPROM.h>
#include <elapsedMillis.h>
//#include <Adafruit_Sensor.h>
#include <PubSubClient.h>

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield

/*
 * Pins used by the ethernet shield
 * 
D2 - Ethernet interrupt (optional with solder bridge "INT")
D4 - SD SPI CS
D10 - Ethernet SPI CS
D11 - Not connected (but should be SPI MOSI)
D12 - Not connected (but should be SPI MISO)
D13 - SPI SCK
A0 - SD Write Protect
A1 - SD Detect
*/
/*
 * RJ45 connector
 * 1 - 5V
 * 2 - D4 (DHT22_A data)
 * 3 - D6 (PIR data)
 * 4 - 3,3V
 * 5 - SCL \__ i2c for bh1750
 * 6 - SDA / 
 * 7 - D30 (DHT22_B data)
 * 8 - GND
 */
#define OneWirePin    2     // DS18B20 data
#define DHTPIN_A      4     // DHT22_A data first
#define DHTPIN_B      5     // DHT22_B data second
#define PIRPIN        6     // HC-SR501 PIR data
#define CKIPIN        8     // WS2801 CKI (Data clock)
#define SDIPIN        9     // WS2801 SDI (Serial Data)
#define redPin        11    // RGB LED red pin   (pwm)
#define greenPin      12    // RGB LED green pin (pwm)
#define bluePin       7     // RGB LED blue pin  (pwm)
#define RelayStart    22    // Relay Board offset (pin 22 - 29)
#define InputStart    36    // Inputs offset (pin 36 - 43)

#define NUMBUTTONS    8
#define NUMRELAYS     8


#define DHTTYPE       DHT22

#define MQTTName              "ArduinoC2"       // the name this Arduino uses to connect ot the broker, must be unique

#define basevector            "raw/mac/"         // do not exceed 10 chars, see below sram note
#define statusvector          "/Status"                   // do not exceed 10 chars, see below sram note
#define dsTopic               "/Temperature/3"
#define relayTopic            "/Relay"
#define switchTopic           "/Switch"
#define RGBTopic              "/RGB"
#define pirTopic              "/Pir" 
#define dhtaTempTopic         "/Temperature/1" 
#define dhtaHummTopic         "/Humidity/1" 
#define dhtbTempTopic         "/Temperature/2" 
#define dhtbHummTopic         "/Humidity/2" 
#define bh1750Topic           "/LightLevel/1" 
#define openhabTopic          "openhab/time"

#define debounce                  50           // debounce number, depends on arduino speed

#define heartbeat_timeout         61000        // we expect an MQTT message at least every 60 seconds from openhab
#define initialmqttconnecttimeout 35000        // should be 35000

#define MQTTDisconnect            5000         // number of miliseconds between each mqtt reconnect try

#define dsStart                   9500         // Read temp once per second
#define dsRead                    500          // time between request and display

#define bh1750Interval            10000
#define dhtInterval               10000      
#define mqttInterval              10000      

//byte writemac[] = { 0xB0, 0x0B, 0xB0, 0x0B, 0x00, 0x05 };
byte mac[]      = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
char macString[13]; // 12 digit MAC address displayed at startup plus null terminator...

char buff[43]; // general buffer. char array. Assuming topicroot is max 10 chars with null terminator
               // this should not ever exceed 43. #sram

char basetopic[30] = {'<','d','e','f','a','u','l','t','>','\0'};        // base topic (30 chars max to be received from OpenHAB)
char commandtopic[41]; // array size: topicroot/zonevector/commandvector (max 11 + 15 + 11 respectively, plus 2x "/" strings)
  
//byte myMqttBroker[] = { 192,168,178,203 };

int SoffitR;
int SoffitG;
int SoffitB;


long strip_colors[1];                    // Array for the WS2801 strip colors, we only use 1
byte myRelay   = 0xFF;                   // All relays of
byte oldRelay  = 0xFF;                   // holding previous relay status
byte myButton  = 0xFF;                   // No switch pressed
byte oldButton = 0xFF;                   // holding previous switch state
byte currentstate[NUMBUTTONS];           // array with button state


int mqttresponsetime = 0;                // Boot sequence: show OpenHAB response time, i.e. difference in time between
//unsigned long defaultmodetimer;        // used for sleep delay, capsense delay, set mode to default
unsigned long heartbeatsense = 0;        // We expect an MQTT message every 1 minute (60000 millis) at least
                                         // There's an OH rule sending datetime at least every minute.
                                         // We use this as our heartbeat
                                         
boolean gotinfofromserver = false;
bool ReadDHT       = false;
bool ColorChange   = false;
bool pirStatus     = false;
bool dsRunning     = false;
bool mqttConnected = false;
bool standalone    = true;

float humidity1;
float temperature1;
float humidity2;
float temperature2;

// Initialize the BH1750 Lightmeter
BH1750 lightMeter(0x23);
// Initialize the DHT22
DHT dht_a(DHTPIN_A, DHTTYPE);
DHT dht_b(DHTPIN_B, DHTTYPE);
// Initialize the Ethernet client library
OneWire ds(OneWirePin); 
DallasTemperature sensors(&ds);

EthernetClient ethClient;
// Initialzie the MQTT client
PubSubClient mqttClient(ethClient);


// Initialize time for DHT
elapsedMillis timeElapsed;
elapsedMillis mqttTimeElapsed;
elapsedMillis MQTTDisconnectElapsed;
elapsedMillis dsDelayElapsed;
elapsedMillis bh1750DelayElapsed;
elapsedMillis openhabTimeElapsed;



long lastReconnectAttempt = 0;


/******************************************/
/*        Pre-Setup Functions             */
/******************************************/

void(* resetFunc) (void) = 0; //declare reset function @ address 0

// ##################################################
// ##         System Setup, Only run once          ##
// ##################################################
void setup() {
  int i;
  Serial.begin(38400);
  Serial.println("Starting Arduino");

  // pins for the WS2801 shield
  pinMode(SDIPIN,   OUTPUT);
  pinMode(CKIPIN,   OUTPUT);
  // pin for pir
  pinMode(PIRPIN,   INPUT);
  // pins for rgb led
  pinMode(redPin,   OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin,  OUTPUT);
  
  // Initialize the I2C bus (BH1750 library doesn't do this automatically)
  Wire.begin();
  BH1750_Init();
  
  dht_a.begin();  
  
  
  //Clear out the array
  strip_colors[0] = Color(30,10,10);
  post_frame();
  
  setColor(0, 0, 50); // Color blue

  etherInit();
  relayInit();
  switchInit();

  // Initialize MQTT
  mqttClient.setServer(Ethernet.mqttBrokerIP(),1883);
  mqttClient.setCallback(callback);
  setColor(50, 0, 50); // Color blue

  
  // First subscribe to a topic consisting of this arduino's MAC address
  // Connect to Broker, give it arduino as the name
  // Connect to the broker  
  if (mqttClient.connect(MQTTName)) 
    MQTTinit();
  else
    Serial.println("No Connection");

  strip_colors[0] = 0;
  post_frame();
}




// ##################################################
// ##                  Main Loop                   ##
// ##################################################
void loop() {
  mqttRun();           // handle MQTT traffic

  ds18B20Run();        // Read ds18B20 data and write to MQTT topic
  bh1750Run();         // Read BH1750 data and write to MQTT topic
  dht22Run();          // Read DHT22 data and write to MQTT topic

  pirRun();            // Read PIR Data, write to MQTT on change

  switchRelay();       // Switch relays based on MQTT input
  
  readInputs();        // Read inputs and publish status on MQTT
  
  ws2801Run();         // Write data to the WS2801 devices

  standaloneRun();     // What to do if we run standalone
  
  //-------------------------------------------------
}


