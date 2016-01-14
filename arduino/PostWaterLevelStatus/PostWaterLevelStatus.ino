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
#include <Adafruit_CC3000.h>
#include <ccspi.h>
#include <SPI.h>
#include <string.h>
#include <Arduino.h>
#include <avr/wdt.h>
#include "utility/debug.h"
#include "utility/netapp.h"

// These are the interrupt and control pins
#define ADAFRUIT_CC3000_IRQ   3  // MUST be an interrupt pin!
// These can be any two pins
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10
// Use hardware SPI for the remaining pins
// On an UNO, SCK = 13, MISO = 12, and MOSI = 11
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                         SPI_CLOCK_DIVIDER); // you can change this clock speed

#define WLAN_SSID       "LYM-41626"           // cannot be longer than 32 characters!
#define WLAN_PASS       "i5Ucawebi5Ucaweb"
// Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY   WLAN_SEC_WPA2

#define IDLE_TIMEOUT_MS  5000      // Amount of time to wait (in milliseconds) with no data 
const unsigned long connectTimeout  = 15L * 1000L; // Max time to wait for server connection

// received before closing the connection.  If you know the server
// you're accessing is quick to respond, you can reduce this value.

// What page to grab!
#define WEBSITE      "wateralarm.xn--mhlemann-65a.ch"
#define WEBPAGE      "/post-status"

#define LEVEL_1 "1"
#define LEVEL_2 "2"
#define LEVEL_3 "3"

#define WIFI_DISABLED 1

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

  Serial.begin(115200);
  Serial.println(F("Booting...\n"));

  Serial.print("Free RAM: "); Serial.println(getFreeRam(), DEC);

  /* Initialise the module */
  Serial.println(F("\nInitializing..."));

  if (WIFI_DISABLED == 0) {
    if (!cc3000.begin())
    {
      Serial.println(F("Couldn't begin()! Check your wiring?"));
      while (1);
    }
  }
  digitalWrite(led_ok, HIGH);
  wdt_reset();

  // Optional SSID scan
  // listSSIDResults();

  Serial.print(F("\nAttempting to connect to ")); Serial.println(WLAN_SSID);

  if (WIFI_DISABLED == 0) {
    if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
      Serial.println(F("Failed!"));
      while (1);
    }

    wdt_reset();

    digitalWrite(led_ok, LOW);
    Serial.println(F("Connected!"));

    /* Wait for DHCP to complete */
    Serial.println(F("Request DHCP"));
    int i;
    for (i = 0; i < 600 && !cc3000.checkDHCP(); i++) {
      delay(100);
    }
    if (i == 600) {
      // Reset after 1 minute
      Serial.println(F("Timeout during DHCP. Resetting"));
      resetFunc();
    }
    wdt_reset();

    /* Display the IP address DNS, Gateway, etc. */
    for (i = 0; i < 60 && !displayConnectionDetails; i++) {
      delay(1000);
    }
    if (i == 60) {
      // Reset after 1 minute
      Serial.println(F("Timeout during showing connection details. Resetting"));
      resetFunc();
    }
    wdt_reset();

    // from https://forums.adafruit.com/viewtopic.php?f=22&t=46901
    unsigned long aucDHCP = 14400;
    unsigned long aucARP = 3600;
    unsigned long aucKeepalive = 10;
    unsigned long aucInactivity = 20;
    if (netapp_timeout_values(&aucDHCP, &aucARP, &aucKeepalive, &aucInactivity) != 0) {
      Serial.println("Error setting inactivity timeout!");
    }
    wdt_reset();
  }

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
  while (ip == 0) {
    if (! cc3000.getHostByName(WEBSITE, &ip)) {
      Serial.println(F("Couldn't resolve!"));
      return 0;
    }
    delay(500);
  }
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

  Adafruit_CC3000_Client www;

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
  unsigned long lastRead = millis();
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
  Serial.println(getFreeRam(), DEC);


  return 1;
}



/**************************************************************************/
/*!
    @brief  Begins an SSID scan and prints out all the visible networks
*/
/**************************************************************************/

void listSSIDResults(void)
{
  uint32_t index;
  uint8_t valid, rssi, sec;
  char ssidname[33];

  if (!cc3000.startSSIDscan(&index)) {
    Serial.println(F("SSID scan failed!"));
    return;
  }

  Serial.print(F("Networks found: ")); Serial.println(index);
  Serial.println(F("================================================"));

  while (index) {
    index--;

    valid = cc3000.getNextSSID(&rssi, &sec, ssidname);

    Serial.print(F("SSID Name    : ")); Serial.print(ssidname);
    Serial.println();
    Serial.print(F("RSSI         : "));
    Serial.println(rssi);
    Serial.print(F("Security Mode: "));
    Serial.println(sec);
    Serial.println();
  }
  Serial.println(F("================================================"));

  cc3000.stopSSIDscan();
}

/**************************************************************************/
/*!
    @brief  Tries to read the IP address and other connection details
*/
/**************************************************************************/
bool displayConnectionDetails(void)
{
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;

  if (!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
  {
    Serial.println(F("Unable to retrieve the IP Address!\r\n"));
    return false;
  }
  else
  {
    Serial.print(F("\nIP Addr: ")); cc3000.printIPdotsRev(ipAddress);
    Serial.print(F("\nNetmask: ")); cc3000.printIPdotsRev(netmask);
    Serial.print(F("\nGateway: ")); cc3000.printIPdotsRev(gateway);
    Serial.print(F("\nDHCPsrv: ")); cc3000.printIPdotsRev(dhcpserv);
    Serial.print(F("\nDNSserv: ")); cc3000.printIPdotsRev(dnsserv);
    Serial.println();
    return true;
  }
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


