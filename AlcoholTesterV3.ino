#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- HARDVÉROVÁ KONFIGURÁCIA ---
#define MQ3_PIN A0
#define ADC_RESOLUTION 1023.0 // PRE ESP32 ZMEŇ NA 4095.0
#define VOLTAGE_REF 5.0       // PRE ESP32 ZMEŇ NA 3.3 (Ak máš delič napätia!)
#define RL 10.0               // Odpor záťažového rezistora v kOhm (na väčšine MQ modulov je 10kOhm, označený ako 103)
#define WARMUP_TIME_SEC 20    // Skrátené zahrievanie pre rýchlejšie testovanie (štandard je 60-120s)

// --- GLOBÁLNE PREMENNÉ ---
float R0 = 10.0; 
bool isReady = false;
unsigned long startTime;

void setup() {
  Serial.begin(115200); // Rýchlejšia komunikácia pre sériový monitor
  
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED displej nenájdený!"));
    while (true);
  }
  
  pinMode(MQ3_PIN, INPUT);
  display.clearDisplay();
  display.setTextColor(WHITE);
  
  startTime = millis();
}

void loop() {
  if (!isReady) {
    handleWarmup();
  } else {
    handleMeasurementFast();
  }
}

// Rýchle a citlivé čítanie ADC (znížený počet vzoriek pre okamžitú odozvu)
float getFastADC() {
  long sum = 0;
  for(int i = 0; i < 5; i++) { // Znížené z 20 na 5 vzoriek
    sum += analogRead(MQ3_PIN);
    delayMicroseconds(500);    // Nekresťansky rýchle vzorkovanie bez zbytočného delay()
  }
  return sum / 5.0;
}

// Výpočet odporu senzora Rs
float calculateRs(float rawADC) {
  float sensorVoltage = (rawADC / ADC_RESOLUTION) * VOLTAGE_REF;
  if (sensorVoltage <= 0.05) return 99999.0; // Ochrana pred delením nulou
  return RL * ((VOLTAGE_REF - sensorVoltage) / sensorVoltage);
}

// Správa zahrievania a automatická kalibrácia
void handleWarmup() {
  unsigned long elapsedSeconds = (millis() - startTime) / 1000;
  
  if (elapsedSeconds <= WARMUP_TIME_SEC) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(10, 5);
    display.print("STABILIZÁCIA SENZORA");
    
    display.setCursor(45, 22);
    display.setTextSize(2);
    display.print(WARMUP_TIME_SEC - elapsedSeconds);
    
    // Plynulý progress bar
    int barWidth = map(elapsedSeconds, 0, WARMUP_TIME_SEC, 0, 108);
    display.drawRect(9, 45, 110, 12, WHITE);
    display.fillRect(10, 46, barWidth, 10, WHITE);
    display.display();
  } 
  else {
    // Okamžitá kalibrácia základnej hladiny čistého vzduchu
    float adcBaseline = getFastADC();
    R0 = calculateRs(adcBaseline);
    if (R0 <= 0.1) R0 = 0.1; 
    
    isReady = true;
  }
}

// Hlavné meranie s okamžitým prekresľovaním displeja
void handleMeasurementFast() {
  float rawADC = getFastADC();
  float Rs = calculateRs(rawADC);
  float ratio = Rs / R0;
  
  if (ratio <= 0) ratio = 0.01;
  
  // Exponenciálny model pre alkohol (štandardná krivka citlivosti)
  float mgL = 0.44 * pow(ratio, -1.45); 
  
  // Odfiltrovanie nízkeho šumu prostredia (v čistom vzduchu ukáže striktne 0.00)
  if (mgL < 0.04) mgL = 0.0;
  
  // Prepočet na promile (BAC)
  float promile = mgL * 2.1;

  // Debugging do PC cez sériovú linku - uvidíš ako rýchlo to lieta
  Serial.print("ADC: "); Serial.print(rawADC, 0);
  Serial.print(" | Ratio: "); Serial.print(ratio, 2);
  Serial.print(" | Promile: "); Serial.println(promile, 2);

  // OKAMŽITÉ VYSTAVENIE NA DISPLEJ
  display.clearDisplay();
  
  // 1. Riadok: Surová hodnota z ADC (pre tvoju vizuálnu kontrolu)
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Surove ADC: ");
  display.print(rawADC, 0);
  
  // 2. Riadok: Veľké promile
  display.setCursor(0, 18);
  display.setTextSize(3);
  display.print(promile, 2);
  display.setTextSize(1);
  display.print(" promile");
  
  // 3. Riadok: Hodnota v mg/L
  display.setCursor(0, 45);
  display.setTextSize(1);
  display.print("Dych (mg/L): ");
  display.print(mgL, 2);
  
  // Grafický indikátor okamžitej reakcie (rýchly pás na spodku displeja)
  int liveBar = map((int)rawADC, 0, (int)ADC_RESOLUTION, 0, 128);
  display.fillRect(0, 58, liveBar, 6, WHITE);

  display.display();
}
