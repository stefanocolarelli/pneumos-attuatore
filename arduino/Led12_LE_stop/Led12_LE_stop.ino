/*
 * Controllo LED adattivo con modalità operative in linea con i comportamenti del polmone di Pneumos
 * 
 * Configurazione:
 * - Ring LED NEOPIXEL da 12 elementi
 * - Comunicazione seriale a 9600 baud
 * - Modalità operative: 0-5 con colori e effetti predefiniti
 */

#include <FastLED.h>
#define NUM_LEDS 12
#define DATA_PIN 7

CRGB leds[NUM_LEDS];
CRGB modeColors[6] = {
  CRGB(255, 224, 180).nscale8(70),   // 0 - Bianco calmo (100%)
  CRGB(255, 210, 160).nscale8(65),   // 1 - Bianco caldo (90%)
  CRGB(255, 190, 130).nscale8(60),   // 2 - Ambracalda chiara (75%)
  CRGB(255, 170, 110).nscale8(55),   // 3 - Ambracalda media (60%)
  CRGB(255, 150, 90).nscale8(50),    // 4 - Ambracalda intensa (50%)
  CRGB(255, 150, 90).nscale8(80)     // 5 - Stesso colore del 4 con effetti
};

bool isStopped = false; // Flag per lo stato di stop

// Colore per gli effetti "spenti" (10% luminosità)
const CRGB dimColor = CRGB(255, 150, 90).nscale8(25); 

byte currentMode = 0;
byte lastMode = 255;
unsigned long ledEffectUntil[NUM_LEDS] = {0}; // Timer effetti singoli LED 
unsigned long fullEffectUntil = 0; // Timer effetto globale 
byte brightnessLevels[6] = {180, 160, 130, 100, 80, 60};

void setup() {
  //Serial.begin(115200);
  Serial.begin(9600);
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(70);
}

void loop() {
  checkSerialInput();

  if(isStopped) {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    return; // Esci prematuramente dal loop
  }
  
  if(currentMode != lastMode) {
    changeModeHandler(); // Gestione cambio modalità
    lastMode = currentMode;
  }
  
  if(currentMode == 5) {
    handleEmergencyEffects(); // Gestione effetti speciali per modalità 5 (respiro di Chaney Stoke)
  }
}

/**
 * Gestione input seriale
 * 
 * Accetta valori '0'-'5' via seriale e aggiorna la modalità
 * con il segnale 8, spegne tutto
 */
void checkSerialInput() {
  if(Serial.available() > 0) {
    int input = Serial.read();
    Serial.write(input); // Echo del carattere per verifica connessione in python
    Serial.flush();      // Forza invio immediato
    
    if(input >= '0' && input <= '5') {
      currentMode = input - '0';
      isStopped = false; // Riattiva i LED
    }
    else if(input >= 0 && input <= 5) {
      currentMode = input;
      isStopped = false; // Riattiva i LED
    }
    else if(input == '8') {
      isStopped = true; // Disattiva i LED
    }
  }
}

/**
 * Gestore cambio modalità
 * 
 * Aggiorna il ring LED con i parametri della nuova modalità
 */
void changeModeHandler() {
  FastLED.setBrightness(brightnessLevels[currentMode]);
  fill_solid(leds, NUM_LEDS, modeColors[currentMode]);
  memset(ledEffectUntil, 0, sizeof(ledEffectUntil));
  fullEffectUntil = 0;
  FastLED.show();
}

/**
 * Gestione effetti emergenza (modalità 5) in modalità CS / Aria pessima
 * 
 * Genera effetti casuali di tremolio e blackout
 */
void handleEmergencyEffects() {
  unsigned long currentMillis = millis();
  bool updated = false;

  // Effetto tremolio attenuato
  for(int i = 0; i < NUM_LEDS; i++) {
    if(ledEffectUntil[i] != 0 && currentMillis >= ledEffectUntil[i]) {
      leds[i] = modeColors[5]; // Ripristina colore pieno
      ledEffectUntil[i] = 0;
      updated = true;
    }
    
    if(random(1000) < 40) { // 4% di probabilità
      leds[i] = dimColor;   // Usa il colore attenuato
      ledEffectUntil[i] = currentMillis + random(100, 500); // Durata più lunga
      updated = true;
    }
  }

  // Effetto globale attenuato
  if(fullEffectUntil < currentMillis && random(100) < 15) {
    fill_solid(leds, NUM_LEDS, dimColor);
    FastLED.show();
    delay(random(50, 200)); // Transizione più morbida
    updated = true;
    fullEffectUntil = currentMillis + random(500, 1500); // Blackout più radi
  }

  if(updated) FastLED.show();
}