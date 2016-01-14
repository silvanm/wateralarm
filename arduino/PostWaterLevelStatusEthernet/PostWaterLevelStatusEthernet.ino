/***************************************************
  This is an example for the Adafruit CC3000 Wifi Breakout & Shield

  Designed specifically to work with the Adafruit WiFi products:
  ----> https://www.adafruit.com/products/1469

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/

/*
This example does a test of the TCP client capability:
 * Initialization
 * Optional: SSID scan
 * AP connection
 * DHCP printout
 * DNS lookup
 * Optional: Ping
 * Connect to website and print out webpage contents
 * Disconnect
SmartConfig is still beta and kind of works but is not fully vetted!
It might not work on all networks!
*/

#include <SPI.h>
#include <string.h>
#include <Arduino.h>
#include <Ethernet2.h>
#include <avr/wdt.h>


#define IDLE_TIMEOUT_MS  5000      // Amount of time to wait (in milliseconds) with no data 
const unsigned long connectTimeout  = 15L * 1000L; // Max time to wait for server connection

// received before closing the connection.  If you know the server
// you're accessing is quick to respond, you can reduce this value.

// What page to grab!
#define WEBSITE      "wateralarm.xn--mhlemann-65a.ch"
#define WEBPAGE      "/post-status"

// assign a MAC address for the ethernet controller.
// fill in your address here:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

#define LEVEL_1 "1"
#define LEVEL_2 "2"
#define LEVEL_3 "3"

#define WIFI_DISABLED 0

int led_ok = 7;
int led_warn = 6;
int measurement_current = 9;
int buzzer = 0;
int onboardLED = 13;

int sensor_0 = 1;
int sensor_1 = 2;
int sensor_2 = 3;

int mute_button = A5;

int threshold = 100;

int postToWebFrequency = 200; // status is posted on each change or after {postToWebFrequency} intervals.
int waitAfterEachMeasurement = 2000;

int alarm_muted;

int waterlevel;
int previousWaterlevel;

int statusHasChanged = 0;

int measurementCounter = 0;

// initialize the library instance:
EthernetClient client;

#define BUZZER_ENABLED 1

/**************************************************************************/
/*!
    @brief  Sets up the HW and the CC3000 module (called automatically
            on startup)
*/
/**************************************************************************/

uint32_t ip;

void setup(void)
{
  pinMode(led_warn, OUTPUT);
  pinMode(led_ok, OUTPUT);
  pinMode(measurement_current, OUTPUT);
  pinMode(buzzer, OUTPUT);

  initialize();

  alarm_muted = 0;

}

void(* resetFunc) (void) = 0; //declare reset function @ address 0

void initialize()
{


  measurementCounter = 0;
  digitalWrite(led_warn, HIGH);
  digitalWrite(led_ok, LOW);

  // Enable Watchdog timer https://bigdanzblog.wordpress.com/2014/10/24/arduino-watchdog-timer-wdt-example-code/
  wdt_reset();
  // Timeout after 8 seconds
  delay(2L * 1000L);
  //wdt_enable(WDTO_8S);

  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  Serial.println(F("Booting...\n"));

  /* Initialise the module */
  Serial.println(F("\nInitializing..."));

  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    for (;;)
      ;
  }

  Serial.print("My IP address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print(".");
  }
  Serial.println();

  digitalWrite(led_ok, HIGH);
  wdt_reset();

  // Optional SSID scan
  // listSSIDResults();

  digitalWrite(led_ok, LOW);
  Serial.println(F("Connected!"));



  digitalWrite(led_ok, HIGH);
  digitalWrite(led_warn, LOW);

  statusHasChanged = 1;

}


void loop(void)
{

  measurementCounter++;
  /* Get Water level */
  waterlevel = 0;



  Serial.print("Measurement Counter:");
  Serial.println(measurementCounter);

  // only enable current when necessary (to prevent corrosion)
  digitalWrite(measurement_current, HIGH);
  delay(200);

  Serial.print("Sensor values:");
  int sensor_0_value = analogRead(sensor_0);
  Serial.print(sensor_0_value);
  Serial.print(",");
  if (sensor_0_value > threshold) {
    waterlevel = 1;
  }

  int sensor_1_value = analogRead(sensor_1);
  Serial.print(sensor_1_value);
  Serial.print(",");
  if (sensor_1_value > threshold) {
    waterlevel = 2;
  }

  int sensor_2_value = analogRead(sensor_2);
  Serial.print(sensor_2_value);
  Serial.print(",");
  if (sensor_2_value > threshold) {
    waterlevel = 3;
  }

  digitalWrite(measurement_current, LOW);

  Serial.println("");
  Serial.print(F("Water level:"));
  Serial.println(waterlevel);

  /* Handling of elevated waterlevels */
  if (waterlevel > 0) {
    digitalWrite(led_warn, HIGH);

    Serial.print(F("Alarm muted:"));
    Serial.println(alarm_muted);

    if (BUZZER_ENABLED == 1) {

      buzzBasedOnWaterlevel();

      if (digitalRead(mute_button) == LOW) {
        Serial.println(F("Alarm mute button pressed!"));
        alarm_muted = 1;
      }
    }
  } else {
    alarm_muted = 0;
    waterlevel = 0;
    digitalWrite(led_warn, LOW);
    digitalWrite(buzzer, LOW);
  }

  // if the water level rises, the buzzer is reactivated
  if (previousWaterlevel < waterlevel) {
    alarm_muted = 0;
  }

  wdt_reset();

  // Only post if the status has changed or in regular intervals
  Serial.println(previousWaterlevel);
  Serial.println(waterlevel);

  if (WIFI_DISABLED == 0) {
    if ((measurementCounter % postToWebFrequency == 0) || (previousWaterlevel != waterlevel)) {
      if (!postStatusToWeb()) {
        initialize();
      }
    }
  }
  wdt_reset();
  delay(5000);
  digitalWrite(led_ok, LOW);
  delay(100);
  digitalWrite(led_ok, HIGH);

  previousWaterlevel = waterlevel;
}

void buzzBasedOnWaterlevel()
{
  int i;
  if (alarm_muted == 0) {
    for (i = 0; i < waterlevel; i++) {
      digitalWrite(buzzer, HIGH);
      delay(200);
      digitalWrite(buzzer, LOW);
      delay(500);
    }
  }
  digitalWrite(buzzer, LOW);
}

int postStatusToWeb()
{
  ip = 0;
  // Try looking up the website's IP address
  Serial.println("postStatusToWeb()");
  Serial.print(WEBSITE); Serial.print(F(" -> "));
  wdt_reset();

  /* Try connecting to the website.
     Note: HTTP/1.1 protocol is used to keep the server from closing the connection before all data is read.
  */

  char level[2];
  char am[2];
  char mc[4];

  sprintf(level, "%d", waterlevel);
  sprintf(am, "%d", alarm_muted);
  sprintf(mc, "%d", measurementCounter);
  Serial.println("Connecting to server");

  int long unsigned t = millis();

  client.stop();

  if (client.connect(WEBSITE, 80)) {
    client.print("GET ");
    client.print(WEBPAGE);
    client.print("?waterlevel=");
    client.print(level);
    client.print("&muted=");
    client.print(am);
    client.print("&measurementcounter=");
    client.print(mc);
    client.print(" HTTP/1.1\r\n");
    client.print("Host: ");
    client.print(WEBSITE);
    client.print("\r\n");
    client.println("Connection: close");
    client.print("\r\n");

  }
  else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
  }
  /*
    do {
      www = cc3000.connectTCP(ip, 80);
    } while ((!www.connected()) &&
             ((millis() - t) < connectTimeout));

    if (www.connected()) {
      www.fastrprint(F("GET "));
      www.fastrprint(WEBPAGE);
      www.fastrprint(F("?waterlevel="));
      www.fastrprint(level);
      www.fastrprint(F("&muted="));
      www.fastrprint(am);
      www.fastrprint(F("&measurementcounter="));
      www.fastrprint(mc);
      www.fastrprint(F(" HTTP/1.1\r\n"));
      www.fastrprint(F("Host: ")); www.fastrprint(WEBSITE); www.fastrprint(F("\r\n"));
      www.fastrprint(F("\r\n"));
      www.println();
    } else {
      Serial.println(F("Connection failed"));
      digitalWrite(led_ok, LOW);
      return 0;
    }

    Serial.println(F("-------------------------------------"));

    /* Read data until either the connection is closed, or the idle timeout is reached. */
  /*unsigned long lastRead = millis();
  bool ledOkState = true;
  while (www.connected() && (millis() - lastRead < IDLE_TIMEOUT_MS)) {
    while (www.available()) {
      char c = www.read();
      Serial.print(c);
    }

    ledOkState = !ledOkState;
    digitalWrite(led_ok, ledOkState ? HIGH : LOW);
    Serial.print("R");
  }
  Serial.println(F("-------------------------------------"));
  www.close();

  /* You need to make sure to clean up after yourself or the CC3000 can freak out */
  /* the next time your try to connect ... */
  //Serial.println(getFreeRam(), DEC);


  return 1;
}


/* Blinks the LED a number of times to inform the user about the state */
void ledSignal(int blinkCount) {
  int k;
  for (k = 1; k <= blinkCount; k = k + 1) {
    digitalWrite(onboardLED, HIGH);
    delay(250L);
    digitalWrite(onboardLED, LOW);
    delay(250L);
  }
  // delay a bit more so it is clear we are done with setup
  delay(750L);
}


