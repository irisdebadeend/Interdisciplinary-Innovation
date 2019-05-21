/**  
 *  DAAU10 MIMIC
 *  @author Iris Verwegen
 *  @since 22/04/2019
 *  Adapted from the provided code from the course
 */


import nl.tue.id.oocsi.*;
import processing.serial.*;
import java.util.*;

OOCSI oocsi;
// shows which IR sensors are triggered
boolean[] seated = {false,false,false,false};
// shows how many are counted to be inside  
int inside = 0;
boolean flagRaised = false;
// the values of all sensors
int[] values = {100,100,1,1,1,1};
// the names of all the sensors
String[] names = {"sonarOutside","sonarInside","firstIR","secondIR","thirdIR","fourthIR"};
boolean busy = false;


// TIMERS
int globalTimer = 0;
int pictureTimer = 0;
int goingOutsideTimer = 0;
int goingInsideTimer = 0;
int seatedTimer = 0;
int[] timersIR = {0,0,0,0};

// if timers are set
boolean seatedTimerSet = false;
boolean[] timersIRSet = {false, false, false, false};

// the limit for the sonar sensors
int LIMIT = 40;

//The serial port for the arduino
Serial myPort;



void setup() {
  size(400, 400);
  frameRate(10);
  background(255);
  noStroke();
  myPort = new Serial(this, "COM4", 9600);

  // connect to OOCSI server running on the Peekaboo
  oocsi = new OOCSI(this, "Peekaboo_datagen", "192.168.4.1");

  // ----------------------------------------------------------------
  // CAREFUL: uncommenting this will reset the data on the Peekaboo
  //oocsi.channel("peekaboo_control")
  //  .data("reset", "")
  //  .data("datetime", System.currentTimeMillis()/1000)
  //  .send();
  //exit();
  // ----------------------------------------------------------------
}

void draw() {
  globalTimer += 1;
  
  // corner case: more people could be seated than detected
  inside = max(inside, nrSeated());
  
  if(inside > 0 && !flagRaised) {
    // if people are present, raise the flag
    oocsi.channel("peekaboo_control")
        .data("flagUp", "")
        .send();
    flagRaised = true;
  } else if (inside < 1 && flagRaised) {
    // if no-one is present, lower the flag
    oocsi.channel("peekaboo_control")
        .data("flagDown", "")
        .send();
    flagRaised = false;
    pictureTimer = globalTimer;
  }
  
  if(globalTimer - pictureTimer == 100 && !flagRaised) {
    // take a picture if the flag is down and 10 seconds have passed
    oocsi.channel("peekaboo_control")
      .data("triggerPhoto", "")
      .send();
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
    oocsi.channel("peekaboo_control")
        .data("flagUp", "")
        .send();
  }
}

void serialEvent(Serial myPort) {
  if(myPort.available() > 0) {
    String temp = myPort.readStringUntil('\n');
    if (temp != null){
      String[] data = splitTokens(temp,"X");
      print("data: ");
      for(int i = 0; i < data.length-1; i++) {
        values[i] = parseInt(data[i]);
        print(data[i] + "_");
      }
      println("inside: " + inside);
    
      for(int i = 2; i < 6; i++) {
        seated[i-2] = (values[i] == 0);
      }
      if(values[0] < LIMIT) {
        goingInsideTimer = globalTimer;
        if(globalTimer-goingOutsideTimer <= 30 && !busy) {
          inside = max(inside-1,0);
          busy = true;
        }
      }
      
      if(values[1] < LIMIT) {
        goingOutsideTimer = globalTimer;
        if(globalTimer-goingInsideTimer <= 30 && !busy) {
          inside += 1;
          busy = true;
        }
      }
      
      if(globalTimer-goingInsideTimer > 30 && globalTimer-goingOutsideTimer > 30) {
        busy = false;
      }
      
      if(globalTimer % 600 == 0) {
        oocsi.channel("sensordata")
            .data(names[0], values[0]).data(names[1], values[1])
            .data(names[2], values[2]).data(names[3], values[3])
            .data(names[4], values[4]).data(names[5], values[5])
            .send();
      }
    }
  }
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
    if(timersIRSet[i] && globalTimer-timersIR[i] > 18000) {
      return true;
    }
  }
  return false;
}
