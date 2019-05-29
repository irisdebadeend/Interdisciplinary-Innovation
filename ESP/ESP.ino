/**  
 *  DAAU10 MIMIC
 *  @author Iris Verwegen, group U05
 *  @since 22/04/2019
 *  Adapted from TU/e DAAU10 code
 */

// include the OOCSI library
#include "OOCSI.h"

// SSID of your Wifi network.
const char *ssid = "Peekaboo";
// Password of your Wifi network.
const char *password = "boxeslittleboxes";

// name for connecting with OOCSI (unique handle)
const char *OOCSIName = "sensormodule1";
// put the adress of your OOCSI server here, can be URL or IP address string
const char *hostserver = "192.168.4.1";

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

// define variables
long duration, distance;
// shows which IR sensors are triggered
boolean seated[3] = {false, false, false};
// shows how many are counted to be inside
int inside = 0;
boolean flagRaised = false;
// the values of all sensors
int values[5] = {100, 100, 1, 1, 1};
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
int timersIR[3] = {0, 0, 0};

// if timers are set
boolean seatedTimerSet = false;
boolean timersIRSet[3] = {false, false, false};

// the limit for the sonar sensors
#define LIMIT 50
// the limit of seating duration
#define TOOLONG 6000
// the time it takes to conclude no-one is present
#define NOTPRESENT 3600

void setup()
{
    // define all pins
    pinMode(button, INPUT);
    pinMode(firstIR, INPUT);
    pinMode(secondIR, INPUT);
    pinMode(thirdIR, INPUT);
    pinMode(trigPinOutside, OUTPUT);
    pinMode(echoPinOutside, INPUT);
    pinMode(trigPinInside, OUTPUT);
    pinMode(echoPinInside, INPUT);
    Serial.begin(115200);

    // setting up OOCSI. processOOCSI is the name of the function to call when receiving messages, can be a random function name
    // connect wifi and OOCSI to the server
    oocsi.connect(OOCSIName, hostserver, ssid, password, processOOCSI);
}

void loop()
{
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
    // button push
    int buttonState = digitalRead(button);

    // Prints the timer on the Serial Monitor
    Serial.print("GlobalTimer: ");
    Serial.println(globalTimer);
    Serial.print("OutSensor: ");
    Serial.println(values[0]);
    Serial.print("InSensor: ");
    Serial.println(values[1]);
    Serial.println(inside);
    Serial.println(values[2]);
    Serial.println(values[3]);
    Serial.println(values[4]);
    //    Serial.print("DistanceOut: ");
    //    Serial.println(sonarOutside);
    //    Serial.print("DistanceIn: ");
    //    Serial.println(sonarInside);
    //    Serial.print("IR: ");
    //    Serial.println(digitalRead(12));

    if (buttonState == HIGH && !isPressing)
    {
        // if they want to turn off the camera, they push the button
        buttonPressed = !buttonPressed;
        isPressing = true;
    }
    else if (buttonState == LOW)
    {
        isPressing = false;
    }

    globalTimer += 1;

    // corner case: more people could be seated than detected
    inside = max(inside, nrSeated());

    if (inside > 0 || buttonPressed)
    {
        if (!flagRaised)
        {
            // if people are present, raise the flag
            oocsi.newMessage("peekaboo_control");
            oocsi.addString("flagUp", "");
            oocsi.sendMessage();
            Serial.println("flagUp @ inside > 0");
            flagRaised = true;
        }
    }
    else if (inside < 1 && flagRaised && !buttonPressed)
    {
        // if no-one is present, lower the flag
        oocsi.newMessage("peekaboo_control");
        oocsi.addString("flagDown", "");
        oocsi.sendMessage();
        flagRaised = false;
        pictureTimer = globalTimer;
    }

    if (globalTimer - pictureTimer == 40 && !flagRaised)
    {
        // take a picture if the flag is down and 10 seconds have passed
        oocsi.newMessage("peekaboo_control");
        oocsi.addString("triggerPhoto", "");
        oocsi.sendMessage();
    }

    if (inside > 0 && nrSeated() == 0 && !seatedTimerSet)
    {
        // if someone is inside but no-one is seated, set the seated timer
        seatedTimer = globalTimer;
        seatedTimerSet = true;
    }
    else if (inside == 0 || nrSeated() > 0)
    {
        //if no-one is inside or is seated, disable the timer
        seatedTimerSet = false;
    }

    // when the seatedTimer runs for X minutes, we can conclude that nobody is present anymore
    // thus reset the nrof people inside
    if (seatedTimerSet && globalTimer - seatedTimer == NOTPRESENT)
    {
        inside = 0;
    }

    // when someone is seated for too long, we are going to wiggle the flag
    if (seatedTooLong())
    {
        oocsi.newMessage("peekaboo_control");
        oocsi.addString("flagUp", "");
        oocsi.sendMessage();
        Serial.println("flagUp @ seatedTooLong()");
    }

    // set the seated values based on the values of the IR sensors
    for (int i = 2; i < 6; i++)
    {
        seated[i - 2] = (values[i] == 0);
    }
    if (values[0] < LIMIT)
    {
        // if someone is passing by, look which direction they are going
        goingInsideTimer = globalTimer;
        dataTimer = globalTimer;
        if (globalTimer - goingOutsideTimer <= 30 && !busy)
        {
            // if the goingOutsideTimer has already been set and enough time has passed
            // we conclude that someone is leaving
            inside = max(inside - 1, 0);
            busy = true;
        }
    }

    if (values[1] < LIMIT)
    {
        // if someone is passing by, look which direction they are going
        goingOutsideTimer = globalTimer;
        dataTimer = globalTimer;
        if (globalTimer - goingInsideTimer <= 30 && !busy)
        {
            // if the goingInsideTimer has already been set and enough time has passed
            // we conclude that someone is entering
            inside += 1;
            busy = true;
        }
    }

    if (values[0] > LIMIT && values[1] > LIMIT && globalTimer - dataTimer > 10)
    {
        // to prevent counting errors
        busy = false;
    }

    if (globalTimer % 100 == 0)
    {
        // send the data to oocsi after every 100 loops
        oocsi.newMessage("sensordata");
        oocsi.addInt("sensorvalue1", values[0]);
        oocsi.addInt("sensorvalue2", values[1]);
        oocsi.addInt("sensorvalue3", 5 * values[2]);
        oocsi.addInt("sensorvalue4", 10 * values[3]);
        oocsi.addInt("sensorvalue5", 15 * values[4]);
        oocsi.sendMessage();
    }

    // needs to be checked in order for OOCSI to process incoming data.
    oocsi.check();
    delay(200);
}

// determine the distance towards a sensor
void SonarSensor(int trigPin, int echoPin)
{
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH);
    distance = duration * 0.034 / 2;
    delay(10);
}

// returns the number of people that are currently seated
int nrSeated()
{
    int areSeated = 0;
    for (int i = 0; i < 3; i++)
    {
        if (seated[i])
        {
            // if someone is seated at spot i, add it to the counter and start timer
            areSeated += 1;
            if (!timersIRSet[i])
            {
                timersIR[i] = globalTimer;
            }
            timersIRSet[i] = true;
        }
        else
        {
            timersIRSet[i] = false;
        }
    }
    return areSeated;
}

// returns is someone is seated for too long
boolean seatedTooLong()
{
    for (int i = 0; i < 3; i++)
    {
        if (timersIRSet[i] && (globalTimer - timersIR[i] > TOOLONG))
        {
            // if the timer has bypassed the limit
            return true;
        }
    }
    return false;
}

void processOOCSI()
{
    // don't do anything; we are sending only
}
