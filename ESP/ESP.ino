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
int button = 15;
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
// shows which IR sensors are triggered
boolean seated [4] = {false,false,false,false};
// shows how many are counted to be inside  
int inside = 0;
boolean flagRaised = false;
// the values of all sensors
int values [6] = {100,100,1,1,1,1};
// the names of all the sensors
String names [6] = {"sonarOutside","sonarInside","firstIR","secondIR","thirdIR","fourthIR"};
boolean busy = false;
boolean buttonPressed = false;
boolean isPressing = false;

// TIMERS
int globalTimer = 0;
int pictureTimer = 0;
int goingOutsideTimer = 0;
int goingInsideTimer = 0;
int seatedTimer = 0;
int dataTimer = 0;
int timersIR [4] = {0,0,0,0};

// if timers are set
boolean seatedTimerSet = false;
boolean timersIRSet [4] = {false, false, false, false};

// the limit for the sonar sensors
#define LIMIT 40
// the limit of seating duration
#define TOOLONG 300

void setup() {
    pinMode(button, INPUT);
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
    // sensors outside
    SonarSensor(trigPinOutside, echoPinOutside);
    values[0] = distance;
    // sensors inside
    SonarSensor(trigPinInside, echoPinInside);
    values[1] = distance;
    // first IR sensor
    values[2] = digitalRead(firstIR);
    // first IR sensor
    values[3] = digitalRead(secondIR);
    // first IR sensor
    values[4] = digitalRead(thirdIR);
    // first IR sensor
    values[5] = digitalRead(fourthIR);
    // button push
    int buttonState = digitalRead(button);

    // Prints the distance on the Serial Monitor
    Serial.println(globalTimer);
//    Serial.print("DistanceOut: ");
//    Serial.println(sonarOutside);
//    Serial.print("DistanceIn: ");
//    Serial.println(sonarInside);
//    Serial.print("IR: ");
//    Serial.println(digitalRead(12));

    if(buttonState == HIGH && !isPressed) {
        buttonPressed = !buttonPressed;
        isPressed = true;
    } else if(buttonState = LOW {
        isPressed = false;
    }
    
    globalTimer += 1;
  
    // corner case: more people could be seated than detected
    inside = max(inside, nrSeated());

    if((inside > 0 || buttonPressed) && !flagRaised) {
        // if people are present, raise the flag
        oocsi.newMessage("peekaboo_control");
            oocsi.addString("flagUp", "");
            oocsi.sendMessage();
        flagRaised = true;
    } else if (inside < 1 && flagRaised && !buttonPressed) {
        // if no-one is present, lower the flag
        oocsi.newMessage("peekaboo_control");
            oocsi.addString("flagDown", "");
            oocsi.sendMessage();
        flagRaised = false;
        pictureTimer = globalTimer;
    }

    if(globalTimer - pictureTimer == 100 && !flagRaised) {
        // take a picture if the flag is down and 10 seconds have passed
        oocsi.newMessage("peekaboo_control");
            oocsi.addString("triggerPhoto", "");
            oocsi.sendMessage();
    }

    if(inside > 0 && nrSeated() == 0 && !seatedTimerSet) {
      seatedTimer = globalTimer;
      seatedTimerSet = true;
    } else if(inside == 0 || nrSeated() > 0) {
      seatedTimerSet = false;
    }

    // when the seatedTimer runs for X minutes, we can conclude that nobody is present anymore
    // thus reset the nrof people inside
    if (seatedTimerSet && globalTimer - seatedTimer == 1200){
        inside = 0; 
    }

    // when someone is seated for too long, we are going to wiggle the flag
    if(seatedTooLong() && globalTimer % 50 == 0) {
        oocsi.newMessage("peekaboo_control");
          oocsi.addString("flagUp", "");
          oocsi.sendMessage();
    }

    for(int i = 2; i < 6; i++) {
        seated[i-2] = (values[i] == 0);
    }
    if(values[0] < LIMIT) {
        goingInsideTimer = globalTimer;
        dataTimer = globalTimer;
        if(globalTimer-goingOutsideTimer <= 30 && !busy) {
            inside = max(inside-1,0);
            busy = true;
        }
    }

    if(values[1] < LIMIT) {
        goingOutsideTimer = globalTimer;
        dataTimer = globalTimer;
        if(globalTimer-goingInsideTimer <= 30 && !busy) {
            inside += 1;
            busy = true;
        }
    }

    if(values[0] > 40 && values[1] > 40 && globalTimer-dataTimer > 10) {
        busy = false;
    }

    if(globalTimer % 100 == 0) {
        oocsi.newMessage("sensordata");
        for(int i = 1; i < 7; i++) {
            oocsi.addInt("sensorvalue" + i, values[i-1]);
        }
        oocsi.sendMessage();
    }

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

int nrSeated() {
    int areSeated = 0;
    for(int i = 0; i < 4; i++) {
        if(seated[i]) {
            areSeated += 1;
            timersIR[i] = globalTimer;
            timersIRSet[i] = true;
        } else {
            timersIRSet[i] = false;
        }
    }
    return areSeated;
}

boolean seatedTooLong() {
    for(int i = 0; i < 4; i++) {
        if(timersIRSet[i] && globalTimer-timersIR[i] > TOOLONG) {
            return true;
        }
    }
    return false;
}

void processOOCSI() {
    // don't do anything; we are sending only
}
