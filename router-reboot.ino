
#include <ESP8266WiFi.h>

const char* ssid     = "yourssid"; // WiFi SSID
const char* password = "yourpassword"; // WiFi password
const char* testHostname = "www.google.com";
IPAddress HostIP;
unsigned int localPort = 80;
const int RELAY_Pin = 13;               //Relay Pin D7 on NodeMCU

#define MINUTES (60L * 1000)
#define SECONDS  1000
const unsigned long  PROBE_DELAY = 5 * MINUTES; //5 minutes check every
const unsigned long  RESET_DELAY = 10 * MINUTES; //10 minutes wait after reset
const unsigned long  RESET_PULSE = 60 * SECONDS; //60 seconds power cycle
int  Nreset_events = 0;
int  failure_counter = 0;
const int fails = 3;

// wifi_counter at 60 with delay 500 is for 30 seconds
// wifi_counter at 120 with delay 500 is for 60 seconds
// wifi_counter at 360 with delay 500 is for 3 minutes
// wifi_counter at 600 with delay 500 is for 5 minutes
// wifi_counter at 1200 with delay 500 is for 10 minutes
// wifi_counter at 1800 with delay 500 is for 15 minutes
int wifi_counter = 1800; //1800

enum {
  TESTING_STATE = 0, FAILURE_STATE = 1, SUCCESS_STATE = 2
};

int CurrentState = TESTING_STATE;

// return the connection type for the AP list
String printConnectionType(int thisType) {
  String con_type = "";
  // read connection type and print out the name:
  switch (thisType) {
    case 255:
      return con_type = "WL_NO_SHIELD";
    case 0:
      return con_type = "WL_IDLE_STATUS";
    case 1:
      return con_type = "WL_NO_SSID_AVAIL";
    case 2:
      return con_type = "WL_SCAN_COMPLETED";
    case 3:
      return con_type = "WL_CONNECTED";
    case 4:
      return con_type = "WL_CONNECT_FAILED";
    case 5:
      return con_type = "WL_CONNECTION_LOST";
    case 6:
      return con_type = "WL_DISCONNECTED";
    default:
      return con_type = "?";
  }
}

void reset_device() {
  // keep track of number of resets
  Nreset_events++;
  Serial.println(String("\nDisconnected... resetting - ") + String(Nreset_events));
  digitalWrite(RELAY_Pin, HIGH);
  delay(RESET_PULSE);
  digitalWrite(RELAY_Pin, LOW);
  Serial.println(String("\nPower restored."));
}


void setup() {
  WiFi.setOutputPower(20.5);
  int connect_counter = 0;

  pinMode(RELAY_Pin, OUTPUT);

  Serial.begin(115200);
  delay(10);
  Serial.println( __FILE__ );
  delay(10);

  // Connecting to a WiFi network
  Serial.println("");
  Serial.print(String("Connecting to ") + ssid);

  WiFi.begin(ssid, password);

  // A workaround is to replace
  // while (WiFi.status() != WL_CONNECTED) {
  // with
  // while (WiFi.localIP().toString() == "0.0.0.0") {

  while (WiFi.localIP().toString() == "0.0.0.0") {
    Serial.print(".");
    delay(500);
    connect_counter++;
    if (connect_counter > wifi_counter) {
      Serial.println("\n");
      Serial.println(printConnectionType(WiFi.status()));
      reset_device();
      delay(RESET_DELAY);
      ESP.restart();
    }
  }
  Serial.println("\n");
  Serial.println(printConnectionType(WiFi.status()));
}

void loop() {
  switch (CurrentState) {

    case TESTING_STATE:
      if (!WiFi.hostByName(testHostname, HostIP)) {
        CurrentState = FAILURE_STATE;
      } else {
        Serial.println("resolve success!");
        CurrentState = SUCCESS_STATE;
      }
      break;

    case FAILURE_STATE:
      failure_counter++;
      Serial.println(failure_counter);
      if (failure_counter >= fails) {
        //Serial.println(failure_counter);
        reset_device();
        delay(RESET_DELAY);
      }
      delay(PROBE_DELAY);
      CurrentState = TESTING_STATE;
      break;

    case SUCCESS_STATE:
      failure_counter = 0;
      delay(PROBE_DELAY);
      CurrentState = TESTING_STATE;
      break;
  }
}

