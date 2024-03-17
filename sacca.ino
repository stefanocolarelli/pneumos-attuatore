#include <Servo.h>

Servo myservo;    // crea oggetto servo
int respiro = 1000;
int basso = 15;
int alto = 120;
int val;
int tipo=6;
int speed=30;

int velocita=13;
float pos = 20;
int pausalunga=random(6,7); // la prima pausa lunga per il sospiro spontaneo dello stato 0
int contapausa=0;
 
void setup() {
  Serial.begin(9600);
  //Serial.println(pausalunga);
  myservo.attach(9);  // servo attached sul pin 9
} 
 
void loop() {
       if (Serial.available()) { // se la connessione seriale è dispoibile
       val = Serial.read(); // leggi il dato seriale e salvalo in val
       //Serial.println(val);
       switch (val) {
          case 0:
          case 48: {
          tipo=0; //good, tasto 0
            break;}
          case 1:
          case 49: {
          tipo=1; //fair, tasto 1
            break;}
          case 2:
          case 50: {
          tipo=2; //moderate, tasto 2
            break;}
          case 3:
          case 51: {
          tipo=3; //poor, tasto 3
            break;}
          case 4:
          case 52: {
          tipo=4; //very poor, tasto 4
            break;}
          case 5:
          case 53: {
          tipo=5; //extr poor, tasto 5
            break;}
          case 6:
          case 54: {
          tipo=6; //speciale in caso di malfunzionamento API, tasto 6
            break;}
        }
       delay(10); // attendi 10ms per la prox lettura
     }
       switch (tipo) {
          case 0:
          alto=120;
          basso=12;
          respiro=random(500,800);
          //una volta ogni 2 minuti, porta doppio ciclo a velocità 24
          velocita=10;
          contapausa++;
          //Serial.println(contapausa);
          if(contapausa>pausalunga){
            velocita=16;
            respiro=random(1000,1300);
            pausalunga=random(48,96);
            if(contapausa>pausalunga+3){
            contapausa=0;
            }
          }
            break;
          case 1:
          alto=110;
          basso=15;
          respiro=random(350,650);
          velocita=9;
            break;
          case 2:
          alto=100;
          basso=15;
          respiro=random(250,550);
          velocita=9;
            break;
          case 3:
          alto=100;
          basso=25;
          respiro=random(200,400);
          velocita=9;
            break;
          case 4:
          alto=100;
          basso=25;
          respiro=0;
          respiro=random(100,250);
          velocita=9;
            break;
          case 5:
          alto=90;
          basso=25;
          respiro=0;
          velocita=9;
            break;
          case 6:
          alto=120;
          basso=95;
          respiro=random(500,800);
          //una volta ogni 2 minuti, porta doppio ciclo a velocità 24
          velocita=20;
          contapausa++;
            break;
        }
    for (pos = basso; pos <= alto; pos += 1) { // va da 0 (minimo) al massimo, fino a 180 potenzialmente. basso è la posizione in basso della sacca, alto è quanto sale
    //Serial.println(pos);
    myservo.write(pos);              // metti il servo in posizione "pos"
    delay(velocita-2);                       // aspetti di "velocita" (in ms) dopo che è arrivato in posizione
  }
  delay(respiro/2); // micropausa post inspirazione
    for (pos = alto; pos >= basso; pos -= 1) { // dal massimo (max 180) fino a 0
      //Serial.print(pos);
      myservo.write(pos);
      if(pos>(alto*0.75)){
        delay(velocita+1);
        //Serial.println("-2");
      }else{
        if(pos<(alto/4)){
          delay(velocita+4);
          //Serial.println("+2");
        }else{
          delay(velocita+1);
          //Serial.println("+0");
        }
      }
    }
  delay(respiro*2); // pausa raddoppiata, per simulare l'inattività post espirazione
}