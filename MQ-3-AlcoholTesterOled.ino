#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128  // Šírka displeja
#define SCREEN_HEIGHT 64  // Výška displeja
#define OLED_RESET    -1  // Ak nemáte reset pin, nastavte na -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define MQ3 A0  // Definovanie analógového pinu pre MQ-3

// Funkcia na výpočet promile na základe hodnoty zo senzora
float vypocitajPromile(int hodnota) {
  // Príkladné mapovanie hodnôt zo senzora na promile
  // Tento vzorec je len orientačný a závisí od kalibrácie
  if (hodnota <= 208) {
    return 0.1;  // V prípade veľmi nízkej hodnoty (bez alkoholu alebo veľmi nízke množstvo)
  } else if (hodnota >= 270 && hodnota <= 290) {
    return 0.5;  // Príklad pre 0.5 promile
  } else if (hodnota >= 300 && hodnota <= 400) {
    return 1.0;  // Príklad pre 1.0 promile
  } else if (hodnota >= 500) {
    return 2.0;  // Príklad pre 2.0 promile
  }
  
  // Mimo rozsahu hodnôt
  return 0.0;
}

void setup() {
  // Inicializácia displeja
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Nie je možné nájsť displej SSD1306"));
    while (1);
  }
  display.clearDisplay();  // Vymazanie displeja na začiatku
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setRotation(false);
  
  // Inicializácia sériovej komunikácie
  Serial.begin(9600);

  // Nastavenie pinu pre MQ-3
  pinMode(MQ3, INPUT);
}

void loop() {
  // Čítanie hodnoty z analógového pinu
  int Detekcia = analogRead(MQ3);

  // Výpočet predpokladaného promile zo senzora
  float promile = vypocitajPromile(Detekcia);

  // Zobrazenie predpokladaného promile na displeji
  display.clearDisplay();  // Vymazanie displeja pred zobrazením novej hodnoty
  display.setCursor(0, 0);  // Nastavenie kurzora na začiatok
  display.print("Hodnota: ");
  display.println(Detekcia);
  display.print("Promile: ");
  display.println(promile, 2);  // Zobrazenie promile s 2 desatinnými miestami
  display.display();  // Aktualizácia displeja

  // Zobrazenie hodnoty v sériovom výstupe
  Serial.print("Hodnota: ");
  Serial.println(Detekcia);
  Serial.print("Predpokladané promile: ");
  Serial.println(promile, 2);  // Zobrazenie promile na sériovom monitore

  delay(1000);  // Pauza medzi jednotlivými cyklami
}
