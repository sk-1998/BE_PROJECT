/**
 * @file ONBOARD_MONITOR.ino 
 *       BOGEE_BITMAP_240x320.c
 * 
 * @brief  ONBOARD MONITORING SYSTEM INO Programing File 
 *
 *  NOTE:- This Program Will Work on STM32 ROGER CORE : https://github.com/rogerclarkmelbourne/Arduino_STM32 
 *         and Supports WIRESLAVE Library will not work with WIRE
 *         C:\Users\saiki\AppData\Local\Arduino15\packages\stm32duino\hardware\STM32F1\2021.3.18\libraries
 *         C:\Users\saiki\OneDrive\Documents\Arduino\hardware\STM32F1\2021.3.18\libraries
 *  
 *  
 *   This will Display Some Important Parameters on Screen , RPM , STATUS OF Recording System , and Temprature 
 *   of BOGIEE
 *  
 * @pinout    - TFT_480x320 :  LCD_CS  --> PA0    |   LM35 : T1 --> PA5
 *                             LCD_CD  --> PA1    |          T2 --> PA6
 *                             LCD_WR  --> PA2    |          T3 --> PA7 
 *                             LCD_RD  --> PA3    |          T4 --> PB0
 *                             LCD_RST --> PA3    | 
 *                                                |
 *                             LCD_D0  --> PB6    |
 *                             LCD_D1  --> PB7    |
 *                             LCD_D2  --> PB8    |
 *                             LCD_D3  --> PB9    |
 *                             LCD_D4  --> PB10   |
 *                             LCD_D5  --> PB11   |
 *                             LCD_D6  --> PB14   |
 *                             LCD_D7  --> PB15   |
 *
 * @reference - http://docs.leaflabs.com/static.leaflabs.com/pub/leaflabs/maple-docs/0.0.12/index-2.html
 *              https://github.com/prenticedavid/MCUFRIEND_kbv/issues/85
 *              
 *              
 * @author SAIKIRAN BEHARA
 * @date 17-MARCH-2021
 *
 */

#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;

#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMono12pt7b.h>
#include "BOGEE_BITMAP_240x320.c"

#define SAMPLE_RATE 1000000

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define GREY    0x8410
#define ORANGE  0xE880


#define T1 PA5 
#define T2 PA6 
#define T3 PA7 
#define T4 PB0 

HardwareTimer timer(2);

String conversion_temp_dc(int IN);
void disp_temp();
void serial_terminal(String MSG);

String MSG_lv[9] = {"\0"};

void setup() {
  // put your setup code here, to run once:
      Serial.begin(115200);
      uint16_t ID = tft.readID();
      
      Serial.print(F("ID = 0x"));
      Serial.println(ID, HEX);

      tft.begin(ID);
      tft.setRotation(1);
      tft.fillScreen(BLACK );

      //INIT & SETTING TIMER INTERRUPT
      timer.pause();
      // timer.setPrescaleFactor(7200);
      //  timer.setOverflow(255);
      timer.setPeriod(SAMPLE_RATE); // IN MICROSECOND
      timer.setChannel1Mode(TIMER_OUTPUT_COMPARE); // SET TIMMER CHANNEL 1 MODE TO OUTPUT COMPARE 
      timer.setCompare(TIMER_CH1, 1); 
      timer.attachCompare1Interrupt(disp_temp);
      timer.refresh();
 
      
      tft.fillRect(240+3,160-25,240-6,26,MAGENTA);
      tft.fillRect(3,3,240-5,30,MAGENTA);
      tft.fillRect(243,3,240-6,26,MAGENTA);
      tft.drawLine(240,0,240,320,MAGENTA);
      tft.drawRect(0,0,480,320,MAGENTA);
      tft.drawLine(240,160-28,480,160-28,MAGENTA);
      tft.setFont(&FreeMono9pt7b);
      tft.drawRGBBitmap(45,104 , BOGEE, 150, 172);
      tft.setTextColor(BLACK);  
      tft.setTextSize(1);
      tft.setCursor(3+2, 20);
      tft.print(" TEMPERATURE MONITOR ");
      tft.setCursor(240+3+2, 20);
      tft.print("    SYSTEM MONITOR ");
      tft.setCursor(240+3+2, (160-25)+(15));
      tft.print("   SERIAL TERMINAL ");
      tft.setTextColor(WHITE);  
      tft.setCursor(243,60);
      tft.println(" STATUS: ");
      tft.setCursor(243,80);
      tft.println(" RPM   : ");
      tft.setCursor(243,100);
      tft.println(" SIZE  : ");
      tft.setCursor(243,120);
      tft.println(" TOTAL PACKETS: ");
      tft.setTextWrap(false);
      
      pinMode(T1,INPUT_ANALOG);
      pinMode(T2,INPUT_ANALOG);
      pinMode(T3,INPUT_ANALOG);
      pinMode(T4,INPUT_ANALOG);

      timer.resume();
           
          
}

void loop() {

  if (Serial.available()){
    String IN = Serial.readStringUntil(' ');
    if (IN.indexOf("TERM") > -1){
      int S = IN.indexOf("TERM") , C = IN.indexOf(":");
      IN.remove(S,(C-S)+1);
      serial_terminal(IN);
    }
    else if (IN.indexOf("STATUS") > -1 ){
      int S1 = IN.indexOf("STATUS") , C1 = IN.indexOf(":");
      IN.remove(S1,(C1-S1)+1);
      //Serial.print("STATUS : ");
      //Serial.println(IN);
      tft.fillRect(330,47,147,20,BLACK); 
      tft.setCursor(332,60);
      tft.println(IN);       
      IN = "\0" ;
    }
    else if (IN.indexOf("RPM") > -1 ){
      int S1 = IN.indexOf("RPM") , C1 = IN.indexOf(":");
      IN.remove(S1,(C1-S1)+1);
      //Serial.print("RPM :");
      //Serial.println(IN);  
      tft.fillRect(340,67,137,20,BLACK);
      tft.setCursor(350,80);
      tft.println(IN);
      IN = "\0" ; 
    }
    else if (IN.indexOf("SIZE") > -1 ){
      int S1 = IN.indexOf("SIZE") , C1 = IN.indexOf(":");
      IN.remove(S1,(C1-S1)+1);
      //IN.concat("B");
      //Serial.print("SIZE :");
      //Serial.println(IN);  
      tft.fillRect(340,87,137,20,BLACK);
      tft.setCursor(350,100);
      tft.println(IN);
      IN = "\0" ;
    }
    else if (IN.indexOf("PACK") > -1 ){
      int S1 = IN.indexOf("PACK") , C1 = IN.indexOf(":");
      IN.remove(S1,(C1-S1)+1);
      //IN.concat("B");
      //Serial.print("PACK :");
      //Serial.println(IN);  
      tft.fillRect(340+65,107,137-65,20,BLACK);
      tft.setCursor(410,120);
      tft.println(IN);
      IN = "\0" ;
    }
  }


}

String conversion_temp_dc(int IN){
  double STEP = (3.3/pow(2,12)), VOLT = 0 , MV = 0 , TEMP = 0;
  String VAL = "\0" ;
  VOLT = IN * STEP ;
  MV = VOLT*1000 ;
  TEMP = VOLT*100;
  VAL = String(TEMP) + "*C" ;
  return VAL ;
}

void disp_temp(){
  tft.fillRect(1,(320-45),239,44,BLACK);
  tft.fillRect(1,(104-45),239,45,BLACK);
 
  tft.setTextColor(WHITE);  
  tft.setTextSize(1);

  tft.setCursor(15, (320-30));
  tft.println(conversion_temp_dc(analogRead(T1)));
  tft.setCursor(15, (99));
  tft.println(conversion_temp_dc(analogRead(T2)));
  tft.setCursor(150, (320-30));
  tft.println(conversion_temp_dc(analogRead(T3)));
  tft.setCursor(150, (99));
  tft.println(conversion_temp_dc(analogRead(T4)));
}

void serial_terminal(String MSG){
  int wt = 245 , ht = 155 ,dht = 20;
  tft.fillRect(240+3,160+2,240-6,156,BLACK);
  MSG_lv[8] =  MSG ;
  for (int i = 0 ; i <= 8 ; i ++ ){
    MSG_lv[i] = MSG_lv[i+1]; 
  }
  for ( int j = 0 ; j <= 8 ; j ++){
    ht += dht;
  tft.setCursor(wt, ht);
  tft.print(MSG_lv[j]);
  }
  ht = 155 ;
}
