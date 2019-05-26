// defines pins numbers
int trigPinOutside = 2;
int echoPinOutside = 4;
int trigPinInside = 13;
int echoPinInside = 5;
int firstIR = 12;

// defines variables
long duration, distance;
long sonarOutside, sonarInside;

void setup() {
   pinMode(trigPinOutside, OUTPUT);
   pinMode(echoPinOutside, INPUT);
   pinMode(trigPinInside, OUTPUT);
   pinMode(echoPinInside, INPUT);
   pinMode(firstIR,INPUT);
   Serial.begin(9600); // Starts the serial communication
}

void loop() {
    //sensors outside
    SonarSensor(trigPinOutside, echoPinOutside);
    sonarOutside = distance;
    //sensors inside
    SonarSensor(trigPinInside, echoPinInside);
    sonarInside = distance;

  // Prints the distance on the Serial Monitor
  Serial.print("DistanceOut: ");
  Serial.println(sonarOutside);
  Serial.print("DistanceIn: ");
  Serial.println(sonarInside);
  Serial.print("IR: ");
  Serial.println(digitalRead(12));
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
