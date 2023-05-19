#include "Arduino.h"
#include "DFRobotDFPlayerMini.h"
#include <GyverOLED.h>

//#define DEBUG

#ifdef DEBUG
//#define DEBUG3
//#define DEBUG2
//#define DEBUG1
#endif

GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> oled;
DFRobotDFPlayerMini myDFPlayer;
HardwareSerial Serial3(PB11, PB10);
HardwareTimer Timer(TIM3);

volatile bool isRunning = false;
volatile bool nextIsPressed = false;
volatile bool prevIsPressed = false;
volatile bool LED_on =true;
volatile int nextCounter = 0;
volatile int prevCounter = 0;
#define SWITCH_DELAY 5 // times delay


void setup()
{
  analogReadResolution(12);

  Timer.pause();
  Timer.setPrescaleFactor(720);
  Timer.attachInterrupt(blink_LED);
  Timer.refresh();
  Timer.resume();
  
  oled.init();        // инициализация
  oled.clear();       // очистка
  oled.setScale(2);   // масштаб текста (1..4)
  oled.home();        // курсор в 0,0
  oled.print(" ЗАГРУЗКА");

  pinMode(PC13, OUTPUT);
  digitalWrite(PC13, HIGH);

  pinMode(PA0, INPUT);
 
  pinMode(PB13, INPUT_PULLDOWN);
  attachInterrupt(PB13, interrupt_tumbler_change, CHANGE);
    
  pinMode(PB14, INPUT_PULLDOWN);
  attachInterrupt(PB14, interrupt_button_prev_change, CHANGE);
  
  pinMode(PB15, INPUT_PULLDOWN);
  attachInterrupt(PB15, interrupt_button_next_change, CHANGE);

  Serial3.begin(9600);
  #ifdef DEBUG
  Serial.begin(115200);
  #endif

  if (!myDFPlayer.begin(Serial3)) {  //Use softwareSerial to communicate with mp3.
    #ifdef DEBUG1
     Serial.println(F("Unable to begin:"));
     Serial.println(F("1.Please recheck the connection!"));
     Serial.println(F("2.Please insert the SD card!"));
     //while(true){
     //  Serial.println(F("ERROR"));
     //}
     #endif
  }
  myDFPlayer.setTimeOut(500);

  //myDFPlayer.volume(10);   //Set volume value. From 0 to 30
  myDFPlayer.EQ(DFPLAYER_EQ_ROCK);
  //myDFPlayer.play(1);      //Play the first mp3
  myDFPlayer.enableLoop(); //Enable loop
  
  if (digitalRead(PB13)){
    isRunning = true;  
  }
}


void loop()
{
  delay(50);
  static bool wasRunning = !wasRunning;
  #ifdef DEBUG2
  Serial.println("-----Status----");
  Serial.printf("isRunning: %d; read: %d\n", isRunning, digitalRead(PB13));
  Serial.printf("nextIsPressed: %d; read: %d\n", nextIsPressed, digitalRead(PB15));
  Serial.printf("prevIsPressed: %d; read: %d\n", prevIsPressed, digitalRead(PB14));
  Serial.println("------END------");
  #endif
  if(isRunning){

    if(!wasRunning){
      myDFPlayer.start();  
      wasRunning = isRunning;
    }

    if(nextIsPressed){
      if(nextCounter == 0 or nextCounter > SWITCH_DELAY){
          myDFPlayer.next();
          nextCounter = 1;
      }
      nextCounter++;
    }
    if(prevIsPressed){
      if(prevCounter == 0 or prevCounter > SWITCH_DELAY){
          myDFPlayer.previous();
          prevCounter = 1;
      }
      prevCounter++;
    }
    
    int valpot1 = analogRead(0);
    int valpot2 = analogRead(0);
    int valpot3 = analogRead(0);
    int valpot4 = analogRead(0);
    int valpot = (valpot1 + valpot2 + valpot3 + valpot4)/4;
    
    valpot1 = analogRead(0);
    valpot2 = analogRead(0);
    valpot3 = analogRead(0);
    valpot4 = analogRead(0);
    valpot = (valpot1 + valpot2 + valpot3 + valpot4)/8 + valpot/2;
    double temp = (double)valpot / 4096.0 * 30.0;
    #ifdef DEBUG3
    Serial.printf("valpot = %d\ntemp = %d\n------\n", valpot, int(round(temp)));
    #endif
    myDFPlayer.volume(30 - round(temp));
    
    oled_update();
  }
  else{
    
    if(wasRunning){
      myDFPlayer.pause();  
      wasRunning = isRunning;
      oled.clear();
    }
    oled_pause(); 
    delay(500);
  }

}

void blink_LED(){
    if(LED_on){
      digitalWrite(PC13, LOW);
      LED_on = false;
    }
    else{
        digitalWrite(PC13, HIGH);
        LED_on = true;
    }
}


void interrupt_tumbler_change(){
  isRunning = digitalRead(PB13);
}

void interrupt_button_next_change()
{
  nextIsPressed = digitalRead(PB15);
  if(!nextIsPressed){
    nextCounter = 0;  
  }
}

void interrupt_button_prev_change()
{
  prevIsPressed = digitalRead(PB14);
  if(!prevIsPressed){
    prevCounter = 0;  
  }
}


void oled_pause()
{
  //oled.clear();   
  oled.setScale(2);
  oled.home(); 
  oled.print("   ПАУЗА   ");
}

void oled_update()
{
  //oled.clear();   
  oled.setScale(2);
  oled.home(); 
  oled.print(" Music Box ");

  oled.setScale(1);
  oled.setCursor(4, 4);
  oled.print("Громкость: ");
  oled.print(myDFPlayer.readVolume());
  oled.print(" из 30  ");

  oled.setCursor(4, 5);
  oled.print("Текущий трек: ");
  oled.print(myDFPlayer.readCurrentFileNumber());
  oled.print(" ");

  oled.setCursor(4, 6);
  oled.print("Всего треков: ");
  oled.print(myDFPlayer.readFileCounts());
  oled.print(" ");
  
}
