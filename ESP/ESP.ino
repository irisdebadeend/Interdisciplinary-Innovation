/**  
 *  DAAU10 MIMIC
 *  @author Iris Verwegen, group U05
 *  @since 22/04/2019
 *  Adapted from TU/e DAAU10 code
 */

// include the OOCSI library
#include "OOCSI.h"

// SSID of your Wifi network.
const char* ssid = "Peekaboo";
// Password of your Wifi network.
const char* password = "boxeslittleboxes";

// name for connecting with OOCSI (unique handle)
const char* OOCSIName = "sensormodule1";
// put the adress of your OOCSI server here, can be URL or IP address string
const char* hostserver = "192.168.4.1";

// OOCSI reference for the entire sketch
OOCSI oocsi = OOCSI();

// pin numbers
int trigPinOutside = 2;
int echoPinOutside = 4;
int trigPinInside = 13;
int echoPinInside = 5;
int firstIR = 12;
int secondIR = 14;
int thirdIR = 18;
int fourthIR = 19;

// defines variables
long duration, distance;
long sonarOutside, sonarInside;

void setup() {
   pinMode(firstIR,INPUT);
   pinMode(secondIR,INPUT);
   pinMode(thirdIR,INPUT);
   pinMode(fourthIR,INPUT);
   pinMode(trigPinOutside, OUTPUT);
   pinMode(echoPinOutside, INPUT);
   pinMode(trigPinInside, OUTPUT);
   pinMode(echoPinInside, INPUT);
   Serial.begin(115200);

   // setting up OOCSI. processOOCSI is the name of the function to call when receiving messages, can be a random function name
   // connect wifi and OOCSI to the server
   oocsi.connect(OOCSIName, hostserver, ssid, password, processOOCSI);
}

void loop() {
    //sensors outside
    SonarSensor(trigPinOutside, echoPinOutside);
    sonarOutside = distance;
    //sensors inside
    SonarSensor(trigPinInside, echoPinInside);
    sonarInside = distance;

    // Prints the distance on the Serial Monitor
//    Serial.print("DistanceOut: ");
//    Serial.println(sonarOutside);
//    Serial.print("DistanceIn: ");
//    Serial.println(sonarInside);
//    Serial.print("IR: ");
//    Serial.println(digitalRead(12));


    // needs to be checked in order for OOCSI to process incoming data.
    oocsi.check();
    delay(200);
}

void SonarSensor(int trigPin, int echoPin) {
  digitalWrite(trigPin,LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin,LOW);
  duration=pulseIn(echoPin, HIGH);
  distance= duration*0.034/2;
  delay(10);
}

void processOOCSI() {
  // don't do anything; we are sending only
}
