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
boolean[] seated = {false,false,false,false};
int inside = 0;
boolean flagRaised = false;
boolean picture = true;
int[] values = new int[6];
String[] names = {"sonarOutside","sonarInside","firstIR","secondIR","thirdIR","fourthIR"};
int[] timersIR = {0,0,0,0};
boolean[] timersIRSet = {false, false, false, false};
boolean timerSet = false;
int globalTimer = 0;
int pictureTimer = 0;
int goingOutsideTimer = 0;
int goingInsideTimer = 0;
int seatedTimer = 0;
int LIMIT = 100;

//The serial port for the arduino
Serial myPort;



void setup() {
  size(400, 400);
  frameRate(10);
  background(255);
  noStroke();

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
  inside = max(inside, nrSeated());
  if(inside > 0 && !flagRaised) {
    oocsi.channel("peekaboo_control")
        .data("flagUp", "")
        .send();
    flagRaised = true;
  } else if (inside < 1 && flagRaised) {
    oocsi.channel("peekaboo_control")
        .data("flagDown", "")
        .send();
    flagRaised = false;
    pictureTimer = globalTimer;
  }
  if(globalTimer - pictureTimer == 100 && !flagRaised) {
    oocsi.channel("peekaboo_control")
      .data("triggerPhoto", "")
      .send();
  }
  
  if(inside > 0 && nrSeated() == 0 && !timerSet) {
    seatedTimer = globalTimer;
    timerSet = true;
  } else if(inside == 0 || nrSeated() > 0) {
    timerSet = false;
  }
  // when the seatedTimer runs for X minutes, we can conclude that nobody is present anymore
  // thus reset the nrof people inside
  if (timerSet && globalTimer - seatedTimer == 1200){
    inside = 0; 
  }
}

void serialEvent(Serial myPort) {
  String temp = myPort.readStringUntil('\n');
  if (temp != null){
    String[] data = splitTokens(temp,"X");
    for(int i = 0; i < data.length-1; i++) {
      values[i] = parseInt(data[i]);
      print(data[i] + "_");
    }
    println();
  }
  for(int i = 2; i < 6; i++) {
    seated[i-2] = (values[i] == 0);
  }
  if(values[0] < LIMIT) {
    goingInsideTimer = globalTimer;
    if(globalTimer-goingOutsideTimer <= 30) {
      inside = max(inside-1,0);
    }
  }
  if(values[1] < LIMIT) {
    goingOutsideTimer = globalTimer;
    if(globalTimer-goingInsideTimer <= 30) {
      inside += 1;
    }
  }

  oocsi.channel("sensordata")
      .data(names[0], values[0]).data(names[1], values[1])
      .data(names[2], values[2]).data(names[3], values[3])
      .data(names[4], values[4]).data(names[5], values[5])
      .send();
}

int nrSeated() {
  int areSeated = 0;
  for(boolean seat:seated) {
    if(seat) areSeated += 1;
  }
  return areSeated;
}
