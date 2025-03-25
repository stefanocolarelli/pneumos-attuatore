#include <Servo.h>

Servo actuator;

const unsigned long updateInterval = 5;
const int pwmMin = 1100;
const int pwmMax = 1900;

float phase = 0.0;
unsigned long lastMillis = 0;

float frequency = 0.2;
float amplitude = 0.92;
float targetFrequency = 0.2;
float targetAmplitude = 0.92;

bool cheneyStokesMode = false;
int cheneyStokesStep = 0;
unsigned long cheneyPause = 0;
bool actuatorStopped = false;

float baseFrequency[]  = {0.20, 0.30, 0.37, 0.51, 0.62, 0.25}; // Hz
float baseAmplitude[]  = {0.92, 0.80, 0.74, 0.68, 0.60, 0.30};

// Cheyne-Stokes arrays
float cheneyAmplitudes[] = {0.10,0.20,0.20,0.30,0.30,0.40,0.40,0.50,0.60,0.50,0.40,0.20,0.20,0.10};
float cheneySpeeds[]     = {0.60,0.58,0.59,0.66,0.72,0.84,0.93,0.91,0.89,0.92,0.95,0.83,0.80,0.80};

void setup() {
  actuator.attach(9);
  Serial.begin(9600);
}

// Gestione seriale con STOP (segnale "8")
void serialCheck() {
  if(Serial.available()) {
    char cmd = Serial.read();
    if(cmd >= '0' && cmd <= '5') {
      int state = cmd - '0';
      targetFrequency = baseFrequency[state];
      targetAmplitude = baseAmplitude[state];

      if(actuatorStopped) {  // se attuatore era fermo, riattacca
        actuator.attach(9);
        actuatorStopped = false;
      }

      if(state == 5 && !cheneyStokesMode) {
        cheneyStokesMode = true;
        cheneyStokesStep = 0;
        cheneyPause = 0;
      } else if (state != 5 && cheneyStokesMode) {
        cheneyStokesMode = false;
      }

      Serial.print("Stato: ");
      Serial.println(state);
    } 
    else if(cmd == '8') {
      actuatorStopped = true;
      actuator.writeMicroseconds(1500); // pos centrale prima di fermarsi
      delay(100);                       // breve attesa per stabilizzarsi
      actuator.detach();                // interrompe completamente il segnale
      Serial.println("Attuatore in pausa (detach).");
    }
  }
}


void loop() {
  serialCheck();

  if(actuatorStopped) {
    delay(10);
    return;  // Attuatore fermo, aspetta prossimo comando
  }

  unsigned long currentMillis = millis();
  if (currentMillis - lastMillis >= updateInterval) {
    lastMillis = currentMillis;

    if(cheneyStokesMode) {
      performCheyneStokes();
      return;
    }

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

// Cheyne-Stokes con continuità fase e gestione STOP
void performCheyneStokes() {
  if(cheneyPause > 0) {
    cheneyPause--;
    delay(1000);
    return;
  }

  amplitude = cheneyAmplitudes[cheneyStokesStep];
  frequency = cheneySpeeds[cheneyStokesStep];

  float omega = 2.0 * PI * frequency;
  float localPhase = phase;

  while(localPhase < phase + 2.0 * PI) {
    serialCheck();
    if(actuatorStopped || !cheneyStokesMode) {
      phase = fmod(localPhase, 2.0 * PI);
      return;
    }
    float sinValue = sin(localPhase);
    int pwmValue = pwmMin + (pwmMax - pwmMin) * 0.5 * (1 + amplitude * sinValue);
    actuator.writeMicroseconds(pwmValue);
    localPhase += omega * (updateInterval / 1000.0);
    delay(updateInterval);
  }

  phase = fmod(localPhase, 2.0 * PI);

  cheneyStokesStep++;
  if (cheneyStokesStep >= sizeof(cheneyAmplitudes)/sizeof(float)) {
    cheneyStokesStep = 0;
    cheneyPause = random(12,18);
  }
}
