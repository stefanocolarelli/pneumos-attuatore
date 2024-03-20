#include <Servo.h>

Servo myservo;  // crea oggetto servo

// Definizioni delle variabili
int pausarespiro = 1000; // pausa tra una fase respiratoria all'altra
int basso = 15; // angolo del servomotore per la massima espirazione
int alto = 120; // angolo del servomotore per la massima inspirazione
int indexpd; // dato in arrivo da puredata
int tipo = 6; // tipologia di respiro attivata ad ogni ciclo
int velocita = 13; // velocità di movimento del pistone
float pos = 20; // posizione del servomotore
int sospiro = 6; // cicli prima del sospiro, caratteristica dello stato 0
int cheyne = 0; // cicli prima del respiro periodico (Cheyne-Stokes), stato 5
int contapausa = 0; // controllo attuazione sospiro o respiro periodico

void setup() {
  Serial.begin(9600); // avvio comunicazione seriale con raspberry pi per ottenere il segnale di indice sulla qualità dell'aria, ogni 30 minuti
  myservo.attach(9); // servo collegato al pin 9
}

void loop() {
  if (Serial.available()) { // se disponibile connessione seriale
    indexpd = Serial.read(); // leggi dato seriale da puredata
    
    // Gestione tipo di respiro in base al dato ricevuto
    switch (indexpd) {
      case 0:
      case 48: tipo = 0; break; // AQ good
      case 1:
      case 49: tipo = 1; break; // AQ fair
      case 2:
      case 50: tipo = 2; break; // AQ moderate
      case 3:
      case 51: tipo = 3; break; // AQ poor
      case 4:
      case 52: tipo = 4; break; // AQ very poor
      case 5:
      case 53: tipo = 5; break; // AQ extr poor
      case 6:
      case 54: tipo = 6; break; // speciale, malfunzionamento API
    }
    
    delay(10); // attesa 10ms per prossima lettura
  }

  // Configurazione comportamento respiratorio
  switch (tipo) {
    case 0: configuraRespiro(120,12,random(500,800),10); break;
    case 1: configuraRespiro(110,15,random(350,650),9); break;
    case 2: configuraRespiro(100,15,random(250,550),9); break;
    case 3: configuraRespiro(100,25,random(200,400),9); break;
    case 4: configuraRespiro(100,25,random(100,250),9); break;
    case 5: cheyneStokes(); break;
    case 6: configuraRespiro(120,95,random(500,800),20); break;
  }
  
  // Simulazione movimento respiratorio tranne per la funzione pessima, dove abilita il respiro di Cheyne-Stokes
  if(tipo!=5){
    simulaMovimentoRespiratorio();    
  }
}

void configuraRespiro(int a, int b, int r, int v) {
  alto=a; basso=b; pausarespiro=r; velocita=v;
  // Comportamento speciale in stato ottimo: fa un respiro profondo (più lento) dopo un numero random di cicli che va da 48 cicli a 96 cicli
  if(tipo==0){
    contapausa++;
    if(contapausa>sospiro){
      velocita=16;
      pausarespiro=random(1000,1300);
      sospiro=random(48,96);
      if(contapausa>sospiro+3){
      contapausa=0;
      }
    }
  }
}

void simulaMovimentoRespiratorio() {
  for (pos = basso; pos <= alto; pos += 1) { // va da 0 (minimo) al massimo, fino a 180 potenzialmente. basso è la posizione in basso della sacca, alto è quanto sale
    myservo.write(pos); // metti il servo in posizione "pos"
    delay(velocita-2); // aspetti di "velocita" (in ms) dopo che è arrivato in posizione
  }
  delay(pausarespiro/3); // micropausa post inspirazione
  for (pos = alto; pos >= basso; pos -= 1) { // dal massimo (max 180) fino a 0
    myservo.write(pos);
    // funzione per creare un leggero easing nel movimento del pistone verso il basso
    if(pos>(alto*0.75)){
      delay(velocita+1);
    }else{
      if(pos<(alto/4)){
        delay(velocita+4);
      }else{
        delay(velocita+1);
      }
    }
  }
  delay(pausarespiro*3); // pausa per simulare l'inattività post espirazione
}

void cheyneStokes(){
  // Da ottimizzare, funzione prototipale per il respiro di Cheyne-Stokes
  for(int i=10;i>3;i--){
    configuraRespiro(90-i,40-i,0,i);
    simulaMovimentoRespiratorio(); 
  }
  for(int i=4;i<11;i++){
    configuraRespiro(90-i,40-i,0,i);
    simulaMovimentoRespiratorio(); 
  }
delay(random(1200,1500)); 
}