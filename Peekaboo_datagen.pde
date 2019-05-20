import nl.tue.id.oocsi.*;

// ******************************************************
// This example requires a running OOCSI server!
//
// How to do that? Check: Examples > Tools > LocalServer
//
// More information how to run an OOCSI server
// can be found here: https://iddi.github.io/oocsi/)
// ******************************************************

OOCSI oocsi;

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

  // generate some sensor data using hte mouse pointer

  fill(#564765);
  ellipse(mouseX, mouseY, 5, 5);
  fill(#765685);
  ellipse(200 - mouseX/10., (mouseX+mouseY)/100., 5, 5);
  fill(#54ff64);
  ellipse(mouseX, mouseX * (3 + mouseY/5.), 5, 5);

  if (frameCount % 20 == 0) {
    // send to OOCSI ...
    oocsi.channel("sensordata")
      .data("sensorvalue1", mouseX).data("sensorvalue2", mouseY)
      .data("sensorvalue3", 200 - mouseX/10.).data("sensorvalue4", (mouseX+mouseY)/100.)
      .data("sensorvalue5", mouseX * (3 + mouseY/5.))
      .send();
    background(255);
  }
}

void mousePressed() {
  // upper part of screen controls the flag
  if (mouseY < height/2) {

  	// left side click moves the flag UP
    if (mouseX < width/2) {
      oocsi.channel("peekaboo_control")
        .data("flagUp", "")
        .send();
    }
  	// right side click moves the flag UP
    else {
      oocsi.channel("peekaboo_control")
        .data("flagDown", "")
        .send();
    }
  }
  // lower part of screen trigger a photo
  else {
    oocsi.channel("peekaboo_control")
      .data("triggerPhoto", "")
      .send();
  }
}
