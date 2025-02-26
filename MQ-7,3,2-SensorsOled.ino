#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Nastavenie OLED displeja (I2C adresa 0x3C)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Definovanie analógových pinov pre senzory
#define MQ2_PIN A0   // MQ-2 - Horľavé plyny
#define MQ3_PIN A1   // MQ-3 - Alkohol
#define MQ7_PIN A2   // MQ-7 - Oxid uhoľnatý

void setup() {
  Serial.begin(9600);

  // Inicializácia OLED displeja
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED displej nenájdený!");
    while (true);
  }
  
  // Zobrazenie úvodnej obrazovky
  display.setRotation(2);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(30, 20);
  display.println("Senzory MQ");
  display.setCursor(20, 40);
  display.println("MQ-2 | MQ-3 | MQ-7");
  display.display();
  delay(2000); // Pauza 2 sekundy
}

void loop() {
  // Čítanie analógových hodnôt zo senzorov
  int mq2Value = analogRead(MQ2_PIN);  // Čítanie analógovej hodnoty z MQ-2
  int mq3Value = analogRead(MQ3_PIN);  // Čítanie analógovej hodnoty z MQ-3
  int mq7Value = analogRead(MQ7_PIN);  // Čítanie analógovej hodnoty z MQ-7

  // Prevod analógových hodnôt na napätie (0 - 5V)
  float mq2Voltage = mq2Value * (5.0 / 1023.0);
  float mq3Voltage = mq3Value * (5.0 / 1023.0);
  float mq7Voltage = mq7Value * (5.0 / 1023.0);

  // Výpis do sériového monitora (pre ladanie)
  Serial.print("MQ-2: "); Serial.print(mq2Value); Serial.print(" - "); Serial.print(mq2Voltage, 2); Serial.print("V | ");
  Serial.print("MQ-3: "); Serial.print(mq3Value); Serial.print(" - "); Serial.print(mq3Voltage, 2); Serial.print("V | ");
  Serial.print("MQ-7: "); Serial.print(mq7Value); Serial.print(" - "); Serial.print(mq7Voltage, 2); Serial.println("V");

  // Aktualizácia OLED displeja
  display.clearDisplay();

  // Kreslenie rámu okolo displeja
  display.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, WHITE);

  // Nadpis
  display.setTextSize(1);
  display.setCursor(35, 3);
  display.println("SENZORY MQ");

  // Stručný výpis analógových hodnôt a napätí pre každý senzor
  display.setCursor(5, 15);
  display.print("MQ-2 : "); display.print(mq2Value); display.print(" - "); display.print(mq2Voltage, 2); display.println(" V");

  display.setCursor(5, 30);
  display.print("MQ-3 : "); display.print(mq3Value); display.print(" - "); display.print(mq3Voltage, 2); display.println(" V");

  display.setCursor(5, 45);
  display.print("MQ-7 : "); display.print(mq7Value); display.print(" - "); display.print(mq7Voltage, 2); display.println(" V");

  // Zobrazenie údajov na displeji
  display.display();

  delay(1000); // Aktualizácia každú sekundu
}
