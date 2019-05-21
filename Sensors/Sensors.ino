/**  
 *  DBSU10 ProxAlert Hiders
 *  @author Iris Verwegen, group U05
 *  @since 06/02/2019
 */

int trigPinOutside = 4;
int echoPinOutside = 5;
int trigPinInside = 2;
int echoPinInside = 3;
int firstIR = 9;
int secondIR = 10;
int thirdIR = 11;
int fourthIR = 12;


long duration, distance;
long sonarOutside, sonarInside;
void setup() {
  Serial.begin(9600); 
   pinMode(firstIR,INPUT);
   pinMode(secondIR,INPUT);
   pinMode(thirdIR,INPUT);
   pinMode(fourthIR,INPUT);
   pinMode(trigPinOutside, OUTPUT);
   pinMode(echoPinOutside, INPUT);
   pinMode(trigPinInside, OUTPUT);
   pinMode(echoPinInside, INPUT);

}

void loop() {
  //sensors outside
  SonarSensor(trigPinOutside, echoPinOutside);
  sonarOutside = distance;
  //sensors inside
  SonarSensor(trigPinInside, echoPinInside);
  sonarInside = distance;
  

  Serial.print(sonarOutside);
  Serial.print("X");
  Serial.print(sonarInside);
  Serial.print("X");
  Serial.print(digitalRead(9));
  Serial.print("X");
  Serial.print(digitalRead(10));
  Serial.print("X");
  Serial.print(digitalRead(11));
  Serial.print("X");
  Serial.print(digitalRead(12));
  Serial.println("X");
}

void SonarSensor(int trigPin, int echoPin) {
  digitalWrite(trigPin,LOW);
  delayMicroseconds(500);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(500);
  digitalWrite(trigPin,LOW);
  duration=pulseIn(echoPin, HIGH);
  distance =(duration/2)/29.1;
  delay(10);
}
