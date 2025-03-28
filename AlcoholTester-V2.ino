    /////////////////////////////////////////////////////////////////
   //             Arduino Breathalyzer Project             v1.01  //
  //       Get the latest version of the code here:              //
 //      http://educ8s.tv/arduino-breathalyzer-project          //
/////////////////////////////////////////////////////////////////

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int TIME_UNTIL_WARMUP = 100; // Originally was set 900 (but it caused waiting for a long time)
unsigned long time;

int analogPin = 0;
int val = 0;

void setup()   {                

 display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
 display.clearDisplay();
}


void loop() {  
  
  delay(100);

  val = readAlcohol();
  printTitle();
  printWarming();

  time = millis()/1000;
  
  if(time<=TIME_UNTIL_WARMUP)
  {
    time = map(time, 0, TIME_UNTIL_WARMUP, 0, 100);
    display.drawRect(10, 50, 110, 10, WHITE); //Empty Bar
    display.fillRect(10, 50, time,10,WHITE);
  }else
  {
     printTitle();
     printAlcohol(val);
     printAlcoholLevel(val);  
  }
  display.display();

}


void printTitle()
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(22,0);
  display.println("AlkoholTester");
}

void printWarming()
{
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,20);
  display.println("Kalibracia");
}

void printAlcohol(int value)
{
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(45,25);
  display.println(val);
}

void printAlcoholLevel(int value)
{
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(10,55);
  
  if(value<241) {
      display.println("Triezvy"); // You are sober
  }
  if (value>=242 && value<280)
  {
      display.println("Jedno pivo");
  }
  if (value>=280 && value<350)
  {
      display.println("2 alebo viac piv");
  }
  if (value>=350 && value <450)
  {
      display.println("Citim 3 < piv");
  }
  if(value>450)
  {
     display.println("Opityyyy!");
  }
 }
 
 int readAlcohol()
 {
  int val = 0;
  int val1;
  int val2;
  int val3; 


  display.clearDisplay();
  val1 = analogRead(analogPin); 
  delay(10);
  val2 = analogRead(analogPin); 
  delay(10);
  val3 = analogRead(analogPin);
  
  val = (val1+val2+val3)/3;
  return val;
 }
