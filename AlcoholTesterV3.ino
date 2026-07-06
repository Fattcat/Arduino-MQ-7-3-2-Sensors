#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Nastavenia OLED displeja
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Nastavenia MQ-3 a Hardvéru
#define MQ3_PIN A0
#define ADC_RESOLUTION 1023.0 // Zmeň na 4095.0 ak používaš ESP32 !
#define VOLTAGE_REF 5.0       // Napájacie napätie na A0 (zmeň na 3.3 pre ESP32)
#define RL 200.0              // Odpor na module MQ-3 v kOhm (často býva aj 1.0, skontroluj SMD súčiastku)
#define WARMUP_TIME_SEC 100   // Čas zahrievania v sekundách (odporúča sa min 1-2 minúty)

// Globálne premenné
float R0 = 10.0; // Odpor v čistom vzduchu, bude prepísaný automatickou kalibráciou
bool isReady = false;
unsigned long startTime;

void setup() {
  Serial.begin(9600);
  
  // Inicializácia OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Chyba: OLED SSD1306 nenajdeny"));
    while (true); // Zastavenie programu
  }
  
  pinMode(MQ3_PIN, INPUT);
  
  display.clearDisplay();
  display.setTextColor(WHITE);
  
  startTime = millis();
}

void loop() {
  if (!isReady) {
    handleWarmupAndCalibration();
  } else {
    handleMeasurement();
    delay(500); // Rýchlosť obnovovania pri bežnej prevádzke
  }
}

// Funkcia na vyčistenie ADC od šumu (Oversampling & Priemer)
float getCleanADC() {
  long sum = 0;
  for(int i = 0; i < 20; i++) {
    sum += analogRead(MQ3_PIN);
    delay(5);
  }
  return sum / 20.0;
}

// Funkcia na výpočet aktuálneho odporu senzora (Rs)
float calculateRs(float rawADC) {
  float sensorVoltage = (rawADC / ADC_RESOLUTION) * VOLTAGE_REF;
  // Ochrana pred delením nulou ak je napätie 0
  if (sensorVoltage <= 0) return 99999.0; 
  return RL * ((VOLTAGE_REF - sensorVoltage) / sensorVoltage);
}

// Zahrievanie a následná automatická kalibrácia R0
void handleWarmupAndCalibration() {
  unsigned long elapsedMillis = millis() - startTime;
  unsigned long elapsedSeconds = elapsedMillis / 1000;
  
  if (elapsedSeconds <= WARMUP_TIME_SEC) {
    // Vykreslenie obrazovky pre zahrievanie
    display.clearDisplay();
    
    display.setTextSize(1);
    display.setCursor(15, 0);
    display.print("Zahrievanie MQ-3");
    
    display.setCursor(35, 20);
    display.print(WARMUP_TIME_SEC - elapsedSeconds);
    display.print(" sekund");
    
    // Matematicky korektný progress bar (mapovanie na šírku 108 px pre obrys 110 px)
    int barWidth = map(elapsedSeconds, 0, WARMUP_TIME_SEC, 0, 108);
    display.drawRect(9, 40, 110, 15, WHITE);          // Rámik
    display.fillRect(10, 41, barWidth, 13, WHITE);    // Výplň
    
    display.display();
  } 
  else {
    // Čas uplynul -> Kalibrácia v čistom vzduchu
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(15, 20);
    display.print("Prebieha kalibracia...");
    display.display();
    
    float adcBaseline = getCleanADC();
    R0 = calculateRs(adcBaseline);
    
    // Ochrana, aby R0 nebolo nezmyselne malé v prípade šumu
    if (R0 <= 0.1) R0 = 0.1; 
    
    isReady = true;
    
    display.clearDisplay();
    display.setCursor(10, 20);
    display.print("Kalibracia Uspesna!");
    display.display();
    delay(2000);
  }
}

// Hlavná meracia slučka po zahriatí
void handleMeasurement() {
  float rawADC = getCleanADC();
  float Rs = calculateRs(rawADC);
  
  float ratio = Rs / R0;
  if (ratio <= 0) ratio = 0.01; // Ochrana proti matematickým anomáliám
  
  // Regresný výpočet (vyplýva z MQ-3 datasheetu na alkohol)
  // Parametre 0.4 a -1.43 sú experimentálne osvedčené pre alkohol a MQ-3.
  float mgL = 0.4 * pow(ratio, -1.43); 
  
  // Vo veľmi čistom vzduchu (ratio okolo 1.0) môže vzorec vrátiť malý zbytkový šum
  // Preto všetko pod 0.05 mg/L berieme ako 0 (šum pozadia)
  if (mgL < 0.05) {
    mgL = 0.0;
  }
  
  // Prevod mg/L (BrAC) na Promile v krvi (BAC) - pomer 2100:1
  float promile = mgL * 2.1;
  
  // Zobrazenie na sériový port pre debugging
  Serial.print("Rs: "); Serial.print(Rs);
  Serial.print(" | R0: "); Serial.print(R0);
  Serial.print(" | Ratio: "); Serial.print(ratio);
  Serial.print(" | mg/L: "); Serial.print(mgL, 3);
  Serial.print(" | Promile: "); Serial.println(promile, 2);
  
  // Vykreslenie UI na OLED
  display.clearDisplay();
  
  // Horný pás - Hodnota Promile
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print(promile, 2);
  display.setTextSize(1);
  display.print(" Promile");
  
  // Stredný pás - Hodnota mg/L
  display.setCursor(0, 25);
  display.setTextSize(1);
  display.print("Dych: ");
  display.print(mgL, 3);
  display.print(" mg/L");
  
  // Spodný pás - Slovné hodnotenie
  display.setCursor(0, 48);
  display.setTextSize(1);
  
  if (promile == 0.0) {
    display.print("Triezvy - Mozes soferovat");
  } else if (promile > 0.0 && promile < 0.3) {
    display.print("Zbytkac / Jedno pivo");
  } else if (promile >= 0.3 && promile < 1.0) {
    display.print("Pod vplyvom! Nesadaj za volant!");
  } else {
    display.print("Opityyyy!");
  }
  
  display.display();
}
