const int gasSensorPin = A0;    // MQ-2 analogový výstup
const int buzzerPin = 9;        // pin pre bzučiak

void setup() {
  pinMode(buzzerPin, OUTPUT);
  Serial.begin(9600);           // na ladenie
}

void loop() {
  int sensorValue = analogRead(gasSensorPin);
  Serial.println(sensorValue);  // Výpis do seriovej konzoly

  if (sensorValue > 450) {
    // Veľmi silný únik plynu
    tone(buzzerPin, 2000); // vysoký tón
    delay(200);
    noTone(buzzerPin);
    delay(200);
  } else if (sensorValue > 250) {
    // Stredný únik
    tone(buzzerPin, 1500);
    delay(400);
    noTone(buzzerPin);
    delay(400);
  } else if (sensorValue > 220) {
    // Slabý únik
    tone(buzzerPin, 1000);
    delay(800);
    noTone(buzzerPin);
    delay(800);
  } else {
    // Bez úniku
    noTone(buzzerPin);
  }

  delay(100);  // krátka pauza medzi meraniami
}