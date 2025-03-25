#include <Servo.h>

Servo actuator;

const unsigned long updateInterval = 5;  // aggiornamento ogni 5 ms
const int pwmMin = 1100;
const int pwmMax = 1900;

float phase = 0.0;
unsigned long lastMillis = 0;

float frequency = 0.1; // default stato 0 (12 respiri/minuto)
float amplitude = 0.92;
float targetFrequency = 0.1;
float targetAmplitude = 0.92;

bool cheneyStokesMode = false;
int cheneyStokesStep = 0;
unsigned long cheneyPause = 0;

float baseFrequency[]  = {0.1, 0.20, 0.23, 0.25, 0.31, 0.25}; // Hz (respiri/sec)
float baseAmplitude[]  = {0.92, 0.78, 0.72, 0.66, 0.58, 0.30}; // Escursione

// Cheyne-Stokes arrays
float cheneyAmplitudes[] = {0.10,0.20,0.20,0.30,0.30,0.40,0.40,0.50,0.60,0.50,0.40,0.20,0.20,0.10};
float cheneySpeeds[]     = {0.60,0.58,0.59,0.66,0.72,0.84,0.93,0.91,0.89,0.92,0.95,0.83,0.80,0.80};

void setup() {
  actuator.attach(9);
  Serial.begin(9600);
}

// Gestione input seriale per cambiare stati
void serialCheck() {
  if(Serial.available()) {
    char cmd = Serial.read();
    if(cmd >= '0' && cmd <= '5') {
      int state = cmd - '0';
      targetFrequency = baseFrequency[state];
      targetAmplitude = baseAmplitude[state];

      if(state == 5 && !cheneyStokesMode) {
        // Entri in Cheyne-Stokes: imposti step iniziale morbido
        cheneyStokesMode = true;
        cheneyStokesStep = 0;
        cheneyPause = 0;
      } else if (state != 5 && cheneyStokesMode) {
        // Esci da Cheyne-Stokes, mantieni continuità
        cheneyStokesMode = false;
        // NON resettare la fase qui, così eviti scatti
      }

      Serial.print("Stato: ");
      Serial.println(state);
    } 
    else if(cmd == '8') {
      actuator.writeMicroseconds(1500); // stop immediato al centro
      while(true); // ferma esecuzione
    }
  }
}

void loop() {
  serialCheck();  

  unsigned long currentMillis = millis();
  if (currentMillis - lastMillis >= updateInterval) {
    lastMillis = currentMillis;

    // Se in modalità Cheyne-Stokes esegui la sequenza speciale
    if(cheneyStokesMode) {
      performCheyneStokes();
      return;
    }

    // Transizione graduale frequenza/ampiezza
    frequency += (targetFrequency - frequency) * 0.01;
    amplitude += (targetAmplitude - amplitude) * 0.01;

    float omega = 2.0 * PI * frequency;
    float deltaTime = updateInterval / 1000.0;
    float inspirationRatio = 0.4;
    float expirationRatio = 0.6;
    float totalDuration = inspirationRatio + expirationRatio;

    if (sin(phase) >= 0) {
      phase += omega * deltaTime * (totalDuration / inspirationRatio);
    } else {
      phase += omega * deltaTime * (totalDuration / expirationRatio);
    }

    if (phase > 2 * PI) {
      phase -= 2 * PI;
    }

    float sinValue = sin(phase);
    int pwmValue = pwmMin + (pwmMax - pwmMin) * 0.5 * (1 + amplitude * sinValue);
    actuator.writeMicroseconds(pwmValue);
  }
}

// Funzione per Cheyne-Stokes
void performCheyneStokes() {
  if(cheneyPause > 0) {
    cheneyPause--;
    delay(1000);
    return;
  }

  amplitude = cheneyAmplitudes[cheneyStokesStep];
  frequency = cheneySpeeds[cheneyStokesStep];

  float omega = 2.0 * PI * frequency;
  // NON resettare la fase qui per garantire continuità morbida
  // phase = 0.0; // <-- questa linea la puoi commentare o rimuovere

  float localPhase = phase; // fase locale per ciclo completo

  while(localPhase < phase + 2.0 * PI) {
    serialCheck();
    float sinValue = sin(localPhase);
    int pwmValue = pwmMin + (pwmMax - pwmMin) * 0.5 * (1 + amplitude * sinValue);
    actuator.writeMicroseconds(pwmValue);
    localPhase += omega * (updateInterval / 1000.0);
    delay(updateInterval);
  }

  // aggiorna fase globale
  phase = fmod(localPhase, 2.0 * PI);

  cheneyStokesStep++;
  if (cheneyStokesStep >= sizeof(cheneyAmplitudes)/sizeof(float)) {
    cheneyStokesStep = 0;
    cheneyPause = random(12,18); // apnea 12-17 sec
  }
}

