/*  
 * ____                     _      ______ _____    _____
  / __ \                   | |    |  ____|  __ \  |  __ \               
 | |  | |_ __   ___ _ __   | |    | |__  | |  | | | |__) |__ _  ___ ___ 
 | |  | | '_ \ / _ \ '_ \  | |    |  __| | |  | | |  _  // _` |/ __/ _ \
 | |__| | |_) |  __/ | | | | |____| |____| |__| | | | \ \ (_| | (_|  __/
  \____/| .__/ \___|_| |_| |______|______|_____/  |_|  \_\__,_|\___\___|
        | |                                                             
        |_|          
 Open LED Race
 An minimalist cars race for LED strip  
  
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.

 by gbarbarov@singulardevices.com  for Arduino day Seville 2019 

 Code made dirty and fast, next improvements in: 
 https://github.com/gbarbarov/led-race
 https://www.hackster.io/gbarbarov/open-led-race-a0331a
 https://twitter.com/openledrace

 Modifications made by Jim Leether and Kevin Justice
 at the M4Reactor Makerspace
 https://m4reactor.org
 
*/

                                                            
#include <Adafruit_NeoPixel.h>
#define MAXLED         300-69 // MAX LEDs actives on strip

#define PIN_LED        A0  // R 500 ohms to DI pin for WS2812 and WS2813, for WS2813 BI pin of first LED to GND  ,  CAP 1000 uF to VCC 5v/GND,power supplie 5V 2A  
#define PIN_START      8   // switch start button PIN and GND
#define PIN_P1         7   // switch player 1 to PIN and GND
#define PIN_P2         6   // switch player 2 to PIN and GND
#define PIN_P3         5   // switch player 3 to PIN and GND
#define PIN_P4         4   // switch player 4 to PIN and GND 
#define PIN_AUDIO      3   // through CAP 2uf to speaker 8 ohms
#define DEMO_PIN       2   // toggles demo mode

// Pixels de interés en circuito Niubit
#define PIX_COMIENZO_SUBIDA  12
#define PIX_FIN_SUBIDA  26
#define PIX_COMIENZO_BAJADA  71
#define PIX_FIN_BAJADA  84
#define PIX_FOCO1_TUNEL1  145
#define PIX_FOCO2_TUNEL1  146
#define PIX_FOCO1_TUNEL2  182
#define PIX_FOCO2_TUNEL2  183

// define frequencies for musical notes
const int c = 261;
const int d = 294;
const int e = 329;
const int f = 349;
const int g = 391;
const int gS = 415;
const int a = 440;
const int aS = 455;
const int b = 466;
const int cH = 523;
const int cSH = 554;
const int dH = 587;
const int dSH = 622;
const int eH = 659;
const int fH = 698;
const int fSH = 740;
const int gH = 784;
const int gSH = 830;
const int aH = 880;

int NPIXELS=MAXLED; // leds on track

// red, green, blue and yellow dots
#define COLOR1    track.Color(255,0,0)
#define COLOR2    track.Color(0,255,0)
#define COLOR3    track.Color(0,0,255)
#define COLOR4    track.Color(255,255,0)

int win_music[] = {
  2637, 2637, 0, 2637, 
  0, 2093, 2637, 0,
  3136    
};
      
byte  gravity_map[MAXLED];     

int TBEEP=3; 

float speed1=0;
float speed2=0;
float speed3=0;
float speed4=0;

float dist1=MAXLED; // Partimos con una vuelta por si al salir volvemos atrás por una rampa
float dist2=MAXLED; // Partimos con una vuelta por si al salir volvemos atrás por una rampa
float dist3=MAXLED; // Partimos con una vuelta por si al salir volvemos atrás por una rampa
float dist4=MAXLED; // Partimos con una vuelta por si al salir volvemos atrás por una rampa

byte p1joined=0;
byte p2joined=0;
byte p3joined=0;
byte p4joined=0;

byte entryCount=0;

float ramp_variance=127;

byte loop1=0;
byte loop2=0;
byte loop3=0;
byte loop4=0;

byte leader=0;
byte raceStarted=0;
byte loop_max=5; //total laps race

int eyecatcher_pos=0;

byte demoMode=1;

float ACEL=0.2;
float kf=0.015; //friction constant orig .015
float kg=0.003; //gravity constant

byte flag_sw1=0;
byte flag_sw2=0;
byte flag_sw3=0;
byte flag_sw4=0;

byte draworder=0;
 
unsigned long timestamp=0;
unsigned long start_time = millis();
long rand_number;

Adafruit_NeoPixel track = Adafruit_NeoPixel(MAXLED, PIN_LED, NEO_GRB + NEO_KHZ800);

int tdelay = 5; 

void set_ramp(byte H,byte a,byte b,byte c)
{for(int i=0;i<(b-a);i++){gravity_map[a+i]=ramp_variance-i*((float)H/(b-a));};
 gravity_map[b]=ramp_variance; 
 for(int i=0;i<(c-b);i++){gravity_map[b+i+1]=ramp_variance+H-i*((float)H/(c-b));};
}

void set_loop(byte H,byte a,byte b,byte c)
{for(int i=0;i<(b-a);i++){gravity_map[a+i]=ramp_variance-i*((float)H/(b-a));};
 gravity_map[b]=255; 
 for(int i=0;i<(c-b);i++){gravity_map[b+i+1]=ramp_variance+H-i*((float)H/(c-b));};
}


void setup() {
  //Serial.begin(9600);
  randomSeed(analogRead(0));
  for(int i=0;i<NPIXELS;i++){gravity_map[i]=ramp_variance;};
  track.begin();
  track.show();
  // define pins for start button, players 1-4 and interrupt pin for disabling demo mode
  pinMode(PIN_START, INPUT_PULLUP);
  pinMode(PIN_P1,INPUT_PULLUP); 
  pinMode(PIN_P2,INPUT_PULLUP);
  pinMode(PIN_P3,INPUT_PULLUP);
  pinMode(PIN_P4,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(DEMO_PIN), disableDemoMode, LOW); 
  pinMode(DEMO_PIN, INPUT_PULLUP);     
}

void start_race(){
   tone(PIN_AUDIO,g, 250);
    delay(230);
   noTone(PIN_AUDIO);  
    tone(PIN_AUDIO,cH, 250);
    delay(230);
   noTone(PIN_AUDIO);  
    tone(PIN_AUDIO,eH, 250);
    delay(230);
   noTone(PIN_AUDIO);  
    tone(PIN_AUDIO,gH, 500);
    delay(230);
   noTone(PIN_AUDIO);
    delay(230);
     tone(PIN_AUDIO,eH, 250);
    delay(230);
   noTone(PIN_AUDIO);  
    tone(PIN_AUDIO,gH, 750);
    delay(230);
   noTone(PIN_AUDIO);
    
  for(int i=0;i<NPIXELS;i++){track.setPixelColor(i, track.Color(0,0,0));};
  track.show();
  delay(2000);
  track.setPixelColor(12, track.Color(255,0,0));
  track.setPixelColor(11, track.Color(255,0,0));
  track.show();
  tone(PIN_AUDIO,400);
  delay(2000);
  noTone(PIN_AUDIO);                  
  track.setPixelColor(12, track.Color(0,0,0));
  track.setPixelColor(11, track.Color(0,0,0));
  track.setPixelColor(10, track.Color(255,255,0));
  track.setPixelColor(9, track.Color(255,255,0));
  track.show();
  tone(PIN_AUDIO,600);
  delay(2000);
  noTone(PIN_AUDIO);                  
  track.setPixelColor(9, track.Color(0,0,0));
  track.setPixelColor(10, track.Color(0,0,0));
  track.setPixelColor(8, track.Color(0,255,0));
  track.setPixelColor(7, track.Color(0,255,0));
  track.show();
  tone(PIN_AUDIO,1200);
  delay(2000);
  noTone(PIN_AUDIO);                               
  timestamp=0;              
 }

void winner_fx() {
   // play win music and light color of winning dot
   int msize = sizeof(win_music) / sizeof(int);
   for (int note = 0; note < msize; note++) {
   tone(PIN_AUDIO, win_music[note],200);
   delay(230);
   noTone(PIN_AUDIO);
   }
  // clear track
  delay(1000);
   for(int i=0;i<NPIXELS;i++){track.setPixelColor(i, track.Color(0,0,0));}; track.show();                                 
  }


void draw_car1(void){for(int i=0;i<=loop1;i++){track.setPixelColor(((word)dist1 % NPIXELS)+i, track.Color(0,255-i*20,0));};                   
  }

void draw_car2(void){for(int i=0;i<=loop2;i++){track.setPixelColor(((word)dist2 % NPIXELS)+i, track.Color(255-i*20,0,0));};            
 }

void draw_car3(void){for(int i=0;i<=loop3;i++){track.setPixelColor(((word)dist3 % NPIXELS)+i, track.Color(0,0,255-i*20));};            
 }

void draw_car4(void){for(int i=0;i<=loop4;i++){track.setPixelColor(((word)dist4 % NPIXELS)+i, track.Color(255-i*20,255-i*20,0));};            
 }
  
void loop() {
    // if the race has not started and no one has joined the race
    if ((raceStarted==0) && (p1joined+p2joined+p3joined+p4joined==0))
      {
        unsigned long current_time = millis();
        // once demo off button is pressed, allow 20 seconds for racers to join
        if ((demoMode==0) && (current_time-start_time>20000)) 
          {
            demoMode=1;
          }
        else if (demoMode==1)
          {
            // if demo mode is active, play random effects
            eyecatcher_pos+=1;
            if (eyecatcher_pos>5) eyecatcher_pos=1;
            start_time=current_time;
            switch (eyecatcher_pos)
            {
              case 1:
                tone(PIN_AUDIO,g, 250);
                delay(230);
               noTone(PIN_AUDIO);  
                tone(PIN_AUDIO,cH, 250);
                delay(230);
               noTone(PIN_AUDIO);  
                tone(PIN_AUDIO,eH, 250);
                delay(230);
               noTone(PIN_AUDIO);  
                tone(PIN_AUDIO,gH, 125);
                delay(230);
               noTone(PIN_AUDIO);
                delay(125);
                tone(PIN_AUDIO,gH, 75);
                delay(75);
               noTone(PIN_AUDIO);
               tone(PIN_AUDIO,gH, 75);
                delay(75);
               noTone(PIN_AUDIO);
               tone(PIN_AUDIO,gH, 75);
                delay(75);
               noTone(PIN_AUDIO);
                tone(PIN_AUDIO,eH, 125);
                delay(230);
               noTone(PIN_AUDIO);
                delay(125);
                tone(PIN_AUDIO,eH, 75);
                delay(75);
               noTone(PIN_AUDIO);
               tone(PIN_AUDIO,eH, 75);
                delay(75);
               noTone(PIN_AUDIO);
               tone(PIN_AUDIO,eH, 75);
                delay(75);
               noTone(PIN_AUDIO);
                tone(PIN_AUDIO,cH, 250);
                delay(230);
               noTone(PIN_AUDIO);  
                tone(PIN_AUDIO,eH, 250);
                delay(230);
               noTone(PIN_AUDIO);
                tone(PIN_AUDIO,cH, 250);
                delay(230);
               noTone(PIN_AUDIO);
                tone(PIN_AUDIO,g, 500);
                delay(500);
               noTone(PIN_AUDIO);
                colorWipe(track.Color(255, 0, 0), 15); // Red
                colorWipe(track.Color(0, 255, 0), 15); // Green
                colorWipe(track.Color(0, 0, 255), 15); // Blue
                colorWipe(track.Color(255, 255, 0), 15); //Yellow
                break;
              case 2:
                // Send a theater pixel chase in...
                theaterChase(track.Color(127, 127, 127), 25); // White
                theaterChase(track.Color(127,   0,   0), 25); // Red
                theaterChase(track.Color(  0,   0, 127), 25); // Blue
                break;
              case 3:
                rainbow(20);
                break;
              case 4:
                rainbowCycle(20);
                break;
              case 5:
                theaterChaseRainbow(50);
                break;
            }
            for(int i=0;i<NPIXELS;i++){track.setPixelColor(i, track.Color(0,0,0));}; track.show();                                 
          }
     
      }
    if ((digitalRead(PIN_START)== LOW) && (entryCount>1) && (raceStarted==0)) //push start switch to start race
    {
      set_ramp(11,PIX_COMIENZO_SUBIDA,PIX_FIN_SUBIDA,PIX_FIN_SUBIDA);
      set_ramp(11,PIX_COMIENZO_BAJADA,PIX_COMIENZO_BAJADA,PIX_FIN_BAJADA);
      track.setPixelColor(PIX_FOCO1_TUNEL1, track.Color(0,0,255));
      track.setPixelColor(PIX_FOCO2_TUNEL1, track.Color(0,0,255));
      track.setPixelColor(PIX_FOCO1_TUNEL2, track.Color(0,0,255));
      track.setPixelColor(PIX_FOCO2_TUNEL2, track.Color(0,0,255));
      delay(500);
      raceStarted=1;
      start_race();
    }

    if ((digitalRead(PIN_P1)==0) && (raceStarted==0) && (p1joined==0)) //push button player 1 controller to join
    {
      p1joined=1;
      entryCount++;
      tone(PIN_AUDIO,500,200);
      for(int i=0;i<NPIXELS;i++){track.setPixelColor(i, track.Color(0,255,0));}; track.show();
      delay(100);
      for(int i=0;i<NPIXELS;i++){track.setPixelColor(i, track.Color(0,0,0));}; track.show();
      delay(100);
      for(int i=0;i<NPIXELS;i++){track.setPixelColor(i, track.Color(0,255,0));}; track.show();
      delay(100);
      for(int i=0;i<NPIXELS;i++){track.setPixelColor(i, track.Color(0,0,0));}; track.show();
    }

    if ((digitalRead(PIN_P2)==0) && (raceStarted==0) && (p2joined==0)) //push button player 2 controller to join
    {
      p2joined=1;
      entryCount++;
      tone(PIN_AUDIO,600,200);
      for(int i=0;i<NPIXELS;i++){track.setPixelColor(i, track.Color(255,0,0));}; track.show();
      delay(100);
      for(int i=0;i<NPIXELS;i++){track.setPixelColor(i, track.Color(0,0,0));}; track.show();
      delay(100);
      for(int i=0;i<NPIXELS;i++){track.setPixelColor(i, track.Color(255,0,0));}; track.show();
      delay(100);
      for(int i=0;i<NPIXELS;i++){track.setPixelColor(i, track.Color(0,0,0));}; track.show();
    }

    if ((digitalRead(PIN_P3)==0) && (raceStarted==0) && (p3joined==0)) //push button player 3 controller to join
    {
      p3joined=1;
      entryCount++;
      tone(PIN_AUDIO,700,200);
      for(int i=0;i<NPIXELS;i++){track.setPixelColor(i, track.Color(0,0,255));}; track.show();
      delay(100);
      for(int i=0;i<NPIXELS;i++){track.setPixelColor(i, track.Color(0,0,0));}; track.show();
      delay(100);
      for(int i=0;i<NPIXELS;i++){track.setPixelColor(i, track.Color(0,0,255));}; track.show();
      delay(100);
      for(int i=0;i<NPIXELS;i++){track.setPixelColor(i, track.Color(0,0,0));}; track.show();
    }

    if ((digitalRead(PIN_P4)==0) && (raceStarted==0) && (p4joined==0)) //push button player 4 controller to join
    {
      p4joined=1;
      entryCount++;
      tone(PIN_AUDIO,800,200);
      for(int i=0;i<NPIXELS;i++){track.setPixelColor(i, track.Color(255,255,0));}; track.show();
      delay(100);
      for(int i=0;i<NPIXELS;i++){track.setPixelColor(i, track.Color(0,0,0));}; track.show();
      delay(100);
      for(int i=0;i<NPIXELS;i++){track.setPixelColor(i, track.Color(255,255,0));}; track.show();
      delay(100);
      for(int i=0;i<NPIXELS;i++){track.setPixelColor(i, track.Color(0,0,0));}; track.show();
    }
    
    if (raceStarted==1)
    {
      for (int i=0; i<NPIXELS; i++)
        track.setPixelColor(i, track.Color(0,0,0));
      track.setPixelColor(PIX_FOCO1_TUNEL1, track.Color(0,0,255));
      track.setPixelColor(PIX_FOCO2_TUNEL1, track.Color(0,0,255));
      track.setPixelColor(PIX_FOCO1_TUNEL2, track.Color(0,0,255));
      track.setPixelColor(PIX_FOCO2_TUNEL2, track.Color(0,0,255));
      
      if ( (flag_sw1==1) && (digitalRead(PIN_P1)==0) ) {flag_sw1=0;speed1+=ACEL;}
      if ( (flag_sw1==0) && (digitalRead(PIN_P1)==1) ) {flag_sw1=1;}
  
      if ((gravity_map[(word)dist1 % NPIXELS])<ramp_variance) speed1-=kg*(ramp_variance-(gravity_map[(word)dist1 % NPIXELS]));
      if ((gravity_map[(word)dist1 % NPIXELS])>ramp_variance) speed1+=kg*((gravity_map[(word)dist1 % NPIXELS])-ramp_variance);
      
      
      speed1-=speed1*kf; 
      
      if ( (flag_sw2==1) && (digitalRead(PIN_P2)==0) ) {flag_sw2=0;speed2+=ACEL;}
      if ( (flag_sw2==0) && (digitalRead(PIN_P2)==1) ) {flag_sw2=1;}
  
      if ((gravity_map[(word)dist2 % NPIXELS])<ramp_variance) speed2-=kg*(ramp_variance-(gravity_map[(word)dist2 % NPIXELS]));
      if ((gravity_map[(word)dist2 % NPIXELS])>ramp_variance) speed2+=kg*((gravity_map[(word)dist2 % NPIXELS])-ramp_variance);
          
      speed2-=speed2*kf; 
  
      if ( (flag_sw3==1) && (digitalRead(PIN_P3)==0) ) {flag_sw3=0;speed3+=ACEL;}
      if ( (flag_sw3==0) && (digitalRead(PIN_P3)==1) ) {flag_sw3=1;}
  
      if ((gravity_map[(word)dist3 % NPIXELS])<ramp_variance) speed3-=kg*(ramp_variance-(gravity_map[(word)dist3 % NPIXELS]));
      if ((gravity_map[(word)dist3 % NPIXELS])>ramp_variance) speed3+=kg*((gravity_map[(word)dist3 % NPIXELS])-ramp_variance);
          
      speed3-=speed3*kf;
  
      if ( (flag_sw4==1) && (digitalRead(PIN_P4)==0) ) {flag_sw4=0;speed4+=ACEL;}
      if ( (flag_sw4==0) && (digitalRead(PIN_P4)==1) ) {flag_sw4=1;}
  
      if ((gravity_map[(word)dist4 % NPIXELS])<ramp_variance) speed4-=kg*(ramp_variance-(gravity_map[(word)dist4 % NPIXELS]));
      if ((gravity_map[(word)dist4 % NPIXELS])>ramp_variance) speed4+=kg*((gravity_map[(word)dist4 % NPIXELS])-ramp_variance);
          
      speed4-=speed4*kf;
          
      dist1+=speed1;
      dist2+=speed2;
      dist3+=speed3;
      dist4+=speed4;
  
      if ((dist1>dist2) && (dist1>dist3) && (dist1>dist4)) {leader=1;}
      if ((dist2>dist1) && (dist2>dist3) && (dist2>dist4)) {leader=2;}
      if ((dist3>dist1) && (dist3>dist2) && (dist3>dist4)) {leader=3;}
      if ((dist4>dist1) && (dist4>dist2) && (dist4>dist3)) {leader=4;}
        
      if (dist1>NPIXELS*loop1) {loop1++;tone(PIN_AUDIO,500);TBEEP=2;}
      if (dist2>NPIXELS*loop2) {loop2++;tone(PIN_AUDIO,600);TBEEP=2;}
      if (dist3>NPIXELS*loop3) {loop3++;tone(PIN_AUDIO,700);TBEEP=2;}
      if (dist4>NPIXELS*loop4) {loop4++;tone(PIN_AUDIO,800);TBEEP=2;}
  
      if (loop1>loop_max) {for(int i=0;i<NPIXELS;i++){track.setPixelColor(i, track.Color(0,255,0));}; track.show();
                                                      winner_fx();loop1=0;loop2=0;loop3=0;loop4=0;dist1=0;dist2=0;dist3=0;dist4=0;speed1=0;speed2=0;speed3=0;speed4=0;p1joined=0;p2joined=0;p3joined=0;p4joined=0;raceStarted=0;entryCount=0;timestamp=0;
                                                     }
      if (loop2>loop_max) {for(int i=0;i<NPIXELS;i++){track.setPixelColor(i, track.Color(255,0,0));}; track.show();
                                                      winner_fx();loop1=0;loop2=0;loop3=0;loop4=0;dist1=0;dist2=0;dist3=0;dist4=0;speed1=0;speed2=0;speed3=0;speed4=0;p1joined=0;p2joined=0;p3joined=0;p4joined=0;raceStarted=0;entryCount=0;timestamp=0;
                                                     }
      if (loop3>loop_max) {for(int i=0;i<NPIXELS;i++){track.setPixelColor(i, track.Color(0,0,255));}; track.show();
                                                      winner_fx();loop1=0;loop2=0;loop3=0;loop4=0;dist1=0;dist2=0;dist3=0;dist4=0;speed1=0;speed2=0;speed3=0;speed4=0;p1joined=0;p2joined=0;p3joined=0;p4joined=0;raceStarted=0;entryCount=0;timestamp=0;
                                                     }
      if (loop4>loop_max) {for(int i=0;i<NPIXELS;i++){track.setPixelColor(i, track.Color(255,255,0));}; track.show();
                                                      winner_fx();loop1=0;loop2=0;loop3=0;loop4=0;dist1=0;dist2=0;dist3=0;dist4=0;speed1=0;speed2=0;speed3=0;speed4=0;p1joined=0;p2joined=0;p3joined=0;p4joined=0;raceStarted=0;entryCount=0;timestamp=0;
                                                     }
      draworder=random(1,4);
  
      if (draworder==1) {if (p1joined==1){draw_car1();};
                         if (p2joined==1){draw_car2();};
                         if (p3joined==1){draw_car3();};
                         if (p4joined==1){draw_car4();};
                        }
      else if (draworder==2) {if (p2joined==1){draw_car2();};
                              if (p3joined==1){draw_car3();};
                              if (p4joined==1){draw_car4();};
                              if (p1joined==1){draw_car1();};
                             }
      else if (draworder==3) {if (p3joined==1){draw_car3();};
                              if (p4joined==1){draw_car4();};
                              if (p1joined==1){draw_car1();};
                              if (p2joined==1){draw_car2();};
                             }
      else {if (p4joined==1){draw_car4();};
            if (p1joined==1){draw_car1();};
            if (p2joined==1){draw_car2();};
            if (p3joined==1){draw_car3();};
           } 
                   
      track.show(); 
      delay(tdelay);
      
      if (TBEEP>0) {TBEEP-=1; 
                    if (TBEEP==0) {noTone(PIN_AUDIO);}; // lib conflict !!!! interruption off by neopixel
                   }
    }
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<track.numPixels(); i++) {
      if(demoMode==0){
        break;
      }
      track.setPixelColor(i, c);
      track.show();
      delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    if(demoMode==0){
      break;
    }
    for(i=0; i<track.numPixels(); i++) {
      if(demoMode==0){
        break;
      }
      track.setPixelColor(i, Wheel((i+j) & 255));
    }
    track.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    if(demoMode==0){
       break;
    }
    for(i=0; i< track.numPixels(); i++) {
      if(demoMode==0){
        break;
      }
      track.setPixelColor(i, Wheel(((i * 256 / track.numPixels()) + j) & 255));
    }
    track.show();
    delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    if(demoMode==0){
      break;
    }
    for (int q=0; q < 3; q++) {
      if(demoMode==0){
        break;
      }
      for (int i=0; i < track.numPixels(); i=i+3) {
        if(demoMode==0){
        break;
      }
        track.setPixelColor(i+q, c);    //turn every third pixel on
      }
      track.show();
     
      delay(wait);
     
      for (int i=0; i < track.numPixels(); i=i+3) {
        if(demoMode==0){
          break;
         }
        track.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    if(demoMode==0){
      break;
    }
    for (int q=0; q < 3; q++) {
          if(demoMode==0){
            break;
          }
        for (int i=0; i < track.numPixels(); i=i+3) {
          if(demoMode==0){
            break;
          }
          track.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
        }
        track.show();
       
        delay(wait);
       
        for (int i=0; i < track.numPixels(); i=i+3) {
          if(demoMode==0){
        break;
      }
          track.setPixelColor(i+q, 0);        //turn every third pixel off
        }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
   return track.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
   return track.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
   WheelPos -= 170;
   return track.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}

void toggleDemoMode(){
  if (demoMode==1){
    demoMode=0;
  }else{
    demoMode=1;
  }
}


void disableDemoMode(){
  demoMode=0;
  start_time=millis();
}
