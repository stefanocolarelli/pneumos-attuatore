#include <VarSpeedServo.h> 
 
VarSpeedServo myservo;    // create servo object to control a servo 
int respiro = 1000;
int basso = 0;
int alto = 140;
int val;
int tipo=0;
int speed=30;
 
void setup() {
  Serial.begin(115200);
  myservo.attach(6);  // attaches the servo on pin 9 to the servo object 
} 
 
void loop() {
       if (Serial.available()) { // If data is available to read,
       val = Serial.read(); // read it and store it in val
       Serial.println(val);
       switch (val) {
          case 48:
          tipo=0; //good, tasto 0
          alto=140;
          speed=30;
            break;
          case 49:
          tipo=1; //fair, tasto 1
          alto=140;
           speed=30;
            break;
          case 50:
          tipo=2; //moderate, tasto 2
          alto=130;
            speed=30;
            break;
          case 51:
          tipo=3; //poor, tasto 3
          alto=110;
            speed=60;
            break;
          case 52:
          tipo=4; //very poor, tasto 4
          alto=90;
            speed=60;
            break;
          case 53:
          tipo=5; //extr poor, tasto 5
          alto=80;
            speed=80;
            break;
        }
       delay(10); // Wait 10 milliseconds for next reading
     } //uso lo swtich per creare una lieve irregolarità naturale nel respiro, che può cambiare ad ogni ciclo rispetto ad ogni input seriale
       switch (tipo) {
          case 0:
          respiro=random(1900,2300);
            break;
          case 1:
          respiro=random(1500,1800);
            break;
          case 2:
          respiro=random(1100,1300);
            break;
          case 3:
          respiro=random(600,900);
            break;
          case 4:
          respiro=random(500,700);
            break;
          case 5:
          respiro=random(0,300);
            break;
        }
  myservo.write(alto, speed, true);        // move to 180 degrees, use a speed of 30, wait until move is complete
  delay(respiro);
  myservo.write(basso, speed, true);        // move to 0 degrees, use a speed of 30, wait until move is complete
  delay(respiro);
}
