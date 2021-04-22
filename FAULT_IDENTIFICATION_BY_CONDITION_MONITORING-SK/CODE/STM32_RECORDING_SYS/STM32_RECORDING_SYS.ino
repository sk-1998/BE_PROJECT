/**
 * @file STM32_RECORDING_SYS.ino 
 * 
 * @brief  STM32_RECORDING_SYS INO Programing File 
 *
 *  NOTE:- This Program Will Work on STM32 ROGER CORE : https://github.com/rogerclarkmelbourne/Arduino_STM32 
 *         and Supports WIRE Library will not work with WIRESLAVE
 *         C:\Users\saiki\AppData\Local\Arduino15\packages\stm32duino\hardware\STM32F1\2021.3.18\libraries
 *         C:\Users\saiki\OneDrive\Documents\Arduino\hardware\STM32F1\2021.3.18\libraries
 *         
 *  
 *  The Acceleration and Gyro DATA is Sampled at 33.33Hz 
 *  and Stored in SD CARD , It will Start Recording if the RPM is Greater then Some threshold Value,
 *  When it is IDEAL Mode it will Start finding Radio Receiver to transmitt the data which is stored in sd card,
 *  
 *  
 *  
 * @pinout -  ADXL345     : SDA --> PB7  ; SCL --> PB6
 *            MPU6050     : SDA --> PB11 ; SCL --> PB10
 *            SERIAL      : TXD --> PA9  ; RXD --> PA10
 *            SD CARD/NRF : MOSI --> PA7 ; MISO --> PA6
 *                          SCK  --> PA5  
 *            TACHO       : DIN  --> PB13
 *            SD CARD     : CS   --> PA4
 *            NRF24       : CE   --> PA3 ; CSN --> PA2
 *
 * @reference - http://docs.leaflabs.com/static.leaflabs.com/pub/leaflabs/maple-docs/0.0.12/index-2.html
 *              https://www.analog.com/media/en/technical-documentation/data-sheets/ADXL345.pdf
 *              https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Datasheet1.pdf
 *              https://www.electronicshub.org/wp-content/uploads/2020/02/STM32F103C8T6-Blue-Pill-Pin-Layout.gif
 *              https://github.com/rogerclarkmelbourne/Arduino_STM32
 *              https://oshwlab.com/saikiran.sk1998/on_board_stm32f103cx
 *              
 * @author SAIKIRAN BEHARA
 * @date 8-FEB-2021
 *
 */


// INCLUDE LIBRARY
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <nRF24L01.h>
#include <RF24.h>

// ADDRESS DEFINED 
#define ADD_ADXL345 0x53 
#define BW_RATE 0x2C
#define POWER_CTL 0x2D
#define DATA_FORMAT 0x31
#define DATA_ACC 0x32

#define ADD_MPU6050 0x68
#define PWR_MGMT_1 0x6B
#define DATA_GYRO 0x43


// DEFINE
#define SAMPLE_RATE 30000  // IN MICROSECOND ; SHOULD GIVE 100Hz SAMPLE RATE
#define MIN_RPM_RATE 6000000 
#define PIN PB13
#define THRESHOLD_RPM 50 

// GLOBAL VARIABLE 
int16_t ACC_DATA_X = 0x0000 , ACC_DATA_Y = 0x0000 , ACC_DATA_Z = 0x0000 ;    // ONE WORD LENGTH FOR GYRO AND ACC 
int16_t GYRO_DATA_X = 0x0000 ,GYRO_DATA_Y = 0x0000, GYRO_DATA_Z = 0x0000 ; //DATA OF EACH AXIS      
int16_t DISTANCE = 0x0000; 
uint32_t SAMPLE = 0x00000000;     
String STR_DATA = "";                     
int ST = 0 , ET = 0 , ACT = 0 ;
uint16_t T1 = 0 ;
float PSV = 0;
double RPM_01 = 0 ;
String STATUS_FLAG = "\0";
uint16_t NUMBER_OF_PACKETS = 0 ,NUMBER_OF_PACKETS_ACK = 0 , PACKET_NUMBER = 0 ,PACKET_NUMBER_ACK = 0 ;
const byte address[6] = "00001";
const uint8_t pipenum = 1 ;
char ACK[32] = {NULL};
 

// CREAT OBJECT 
TwoWire AccWire(1, I2C_FAST_MODE);  // I2C_1 (PB7-SDA, PB6-SCL )
TwoWire GyroWire(2, I2C_FAST_MODE); // I2C_2 (PB11-SDA, PB10-SCL )

HardwareTimer timer_a(2);
HardwareTimer timer_b(3);

File myFile;
RF24 radio(PA3, PA2);  // CE, CSN

// FUNCTION 
//void Starr_Recording(void);
void Recording(void);
void RPM(void);
void startRecording(void);
void stopRecording(void);
void Print(void);
void transmit_data(void);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);   // INIT SERIAL AT 115200 BAUDRATE

  AccWire.begin();
  GyroWire.begin();
  


  // INIT ADXL345
  AccWire.beginTransmission(ADD_ADXL345);   // TRANSMISSION STARTS
  AccWire.write(BW_RATE);      // DATA RATE AND POWER MODE CONTROL 
  AccWire.write(0x0D);         // DATA RATE SET TO 400Hz 
  AccWire.endTransmission();
  AccWire.beginTransmission(ADD_ADXL345);   
  AccWire.write(POWER_CTL); 
  AccWire.write(0x08);
  AccWire.endTransmission();
  AccWire.beginTransmission(ADD_ADXL345);     
  AccWire.write(DATA_FORMAT);   // DATA FORMAT CONTROL 
  AccWire.write(0x03);         // SET SENSITIVITY TO +- 4g 
  AccWire.endTransmission();   // TRANSMISSION ENDS
  delay(250);

  //INIT MPU6050
  GyroWire.beginTransmission(ADD_MPU6050);   // TRANSMISSION STARTS 
  GyroWire.write(PWR_MGMT_1);
  GyroWire.write(0x00);
  GyroWire.endTransmission();

  

  //INIT SD CARD 
    if (!SD.begin(PA4)) {
    Serial.println("TERM:initialization_failed! ");
    while (1);
   }
 // Serial.println("initialization done.");

  SD.remove("GATEWAY.txt"); // REMOVE PREVIOUS FILE 
  myFile = SD.open("GATEWAY.txt", FILE_WRITE); // CREAT FILE
  myFile.close(); 
  

  //INIT & SETTING TIMER INTERRUPT
  timer_a.pause();
  timer_b.pause();
    // timer1.setPrescaleFactor(7200);
    // timer1.setOverflow(255);
    // timer2.setPeriod(MIN_RPM_RATE);
  timer_b.setPrescaleFactor(6592);
  timer_b.setOverflow(65534);
  timer_b.setCount(0);
  timer_a.setPeriod(SAMPLE_RATE); // IN MICROSECOND
  timer_a.setChannel1Mode(TIMER_OUTPUT_COMPARE); // SET TIMMER CHANNEL 1 MODE TO OUTPUT COMPARE 
  timer_b.setChannel2Mode(TIMER_OUTPUT_COMPARE);
  timer_a.setCompare(TIMER_CH1, 1); 
  timer_b.setCompare(TIMER_CH2,65534 );
  timer_a.attachCompare2Interrupt(Recording);
  timer_b.attachCompare3Interrupt(stopRecording);
  timer_b.refresh();
  timer_a.refresh(); 
/*  uint32_t ST = 0 , ET = 0 , AT = 0  ;
  ST = systick_get_count();
  Recording();
  ET = systick_get_count();
  AT = ET - ST ; 
  Serial.println(AT); */

  PSV = (72*pow(10,6)/timer_b.getPrescaleFactor());

  //ATTACH TACO ON INTERRUP PIN 
  pinMode(PIN,INPUT);
  attachInterrupt(PIN,RPM,FALLING );

  //RADIO INIT
  Serial.println(radio.begin());
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.setChannel(76);
  radio.setPayloadSize(32);
  radio.setAutoAck(true);
  radio.enableDynamicPayloads();
  radio.enableAckPayload ();
  //Set module as transmitter
  radio.stopListening();

  pinMode(PC13,OUTPUT);
  
  
}

void loop() {
  // put your main code here, to run repeatedly:
 // Recording();


  if (STATUS_FLAG == "TRANSMITT"){
    delay(1000);
    transmit_data();
  }
  else{
  delay(300);
  digitalWrite(PC13,LOW);
  delay(300);
  Serial.print("RPM:" + String(RPM_01) + " ");
  digitalWrite(PC13,HIGH);
  }
}


void Recording(void){

 
  // ACCELERATION DATA READING
  AccWire.beginTransmission(ADD_ADXL345);
  AccWire.write(DATA_ACC);
  AccWire.endTransmission();
  AccWire.requestFrom(ADD_ADXL345,6);
  if (AccWire.available()){
    ACC_DATA_X = AccWire.read() << 8  | AccWire.read() ;    
    ACC_DATA_Y = AccWire.read() << 8  | AccWire.read() ;  
    ACC_DATA_Z = AccWire.read() << 8  | AccWire.read() ;  
  }

  // ROTATIONAL DATA READING
  GyroWire.beginTransmission(ADD_MPU6050);
  GyroWire.write(DATA_GYRO);         
  GyroWire.endTransmission();
  GyroWire.requestFrom(ADD_MPU6050,6);
  if(GyroWire.available()){
  GYRO_DATA_X = GyroWire.read() << 8  | GyroWire.read() ;
  GYRO_DATA_Y = GyroWire.read() << 8  | GyroWire.read() ;
  GYRO_DATA_Z = GyroWire.read() << 8  | GyroWire.read() ;
  }

    // DATA STORING 
  if (myFile) { 
    STR_DATA += String(ACC_DATA_X) + ":" + String(ACC_DATA_Y) + ":" + String(ACC_DATA_Z) + ":" + \
                  String(GYRO_DATA_X) + ":" +String(GYRO_DATA_Y) + ":" +String(GYRO_DATA_Z) + ":" + \
                  String(DISTANCE) + ";"  ;  // CATINATING ALL THE DATA WITH  DELIMETRY ":"
   
    myFile.print(SAMPLE);
    myFile.print(":");
    myFile.print(STR_DATA);
    myFile.println();
    STR_DATA = "" ;
    SAMPLE += 1 ;
  } 
   // Print();
    
}

void Print (void) {
  Serial.print(ACC_DATA_X);
  Serial.print("    ");
  Serial.print(ACC_DATA_Y);
  Serial.print("    ");
  Serial.print(ACC_DATA_Z);
  Serial.print("    ");
  Serial.print(GYRO_DATA_X);
  Serial.print("    ");
  Serial.print(GYRO_DATA_Y);
  Serial.print("    ");
  Serial.print(GYRO_DATA_Z);
  Serial.println("");
}

void RPM(void){
  timer_b.pause();
  T1 = timer_b.getCount();
  if (T1 > 0){
  RPM_01 = (60/(T1/PSV) );
  }
  else {
    RPM_01 = 0 ; 
  }

  if ( RPM_01 > THRESHOLD_RPM && (STATUS_FLAG != "RECORDING" )){
    Serial.print("TERM:RECORDING_STARTED ");
    startRecording();
    
  }
  
  timer_b.setCount(0);
  timer_b.resume();
}

void startRecording(){
  myFile = SD.open("GATEWAY.txt", FILE_WRITE);
  Serial.print("STATUS:RECORDING ");
  timer_a.resume();
}

void stopRecording(){
  Serial.print("TERM:Recording_Stoped ");
  Serial.print("STATUS:NOT_RECORDING ");
  timer_a.pause();
  timer_a.refresh();
  myFile.close();
  RPM_01 = 0;
  timer_b.pause();
  timer_b.refresh();
  delay(500);
  STATUS_FLAG = "TRANSMITT";
  
}

void transmit_data(){

  myFile = SD.open("GATEWAY.txt");
  NUMBER_OF_PACKETS = 0 ;
  NUMBER_OF_PACKETS = ((myFile.size()/32)+1);
  Serial.print("SIZE:" + String(myFile.size()) + " ") ;
  Serial.print("PACK:" + String(NUMBER_OF_PACKETS) + " ") ;


  delay(500);

  if (NUMBER_OF_PACKETS < 100 ){
     myFile.close();
  }
  
  



  else {
    delay(500);
    Serial.print("TERM:START_TRANSMISSION ");


  char CMD[] = "START" ,CMD_ACK[32] = {NULL} ;
  
  Serial.println(NUMBER_OF_PACKETS );

  //###############TRANSMIT_NUMBER_OF_PACKETS_META_DATA###################
  NUMBER_OF_PACKETS_ACK = 0 ;
  CMD_ACK[32] = {NULL} ; 
   
   while(!(String(CMD_ACK) == "OK")){
    radio.write(&NUMBER_OF_PACKETS,sizeof(NUMBER_OF_PACKETS));
    if(radio.isAckPayloadAvailable()){
      radio.read(&CMD_ACK,32);
     // Serial.println(CMD_ACK);
    }
    else{
      delay(100);
    }
    delay(50);
   }

    while(!(NUMBER_OF_PACKETS == NUMBER_OF_PACKETS_ACK)){   
    radio.write(&CMD,sizeof(CMD));
    if(radio.isAckPayloadAvailable()){
      radio.read(&NUMBER_OF_PACKETS_ACK,sizeof(NUMBER_OF_PACKETS_ACK));
     // Serial.println(NUMBER_OF_PACKETS_ACK);
    }
    else{
      delay(100);
    }
    delay(50);
  
  }

 // Serial.println("TRANSMISSION STARTED");
  
  
  if (myFile) {
  //  Serial.println("GATEWAY.txt");

    // read from the file until there's nothing else in it:
    for ( PACKET_NUMBER = 1 ; PACKET_NUMBER <= NUMBER_OF_PACKETS ; PACKET_NUMBER++) {
      char buff[32] = {"\0"};
      int len = 0 ;
        if(myFile.available() < 32 ){
          len = myFile.available() ;
        }
        else {
          len = 32 ; 
        }
//####################WORKING#############################
         for( int j = 0 ; j < len ; j++){
        buff[j] = myFile.read();                  // WE CAN CHANGE THIS 
      }
 /*     for( int k = 0 ; k < len ; k++){
        Serial.write(buff[k]);                    // WE DON't NEED THIS 
      }*/
//########################################################      


/*     while(!(radio.write(&NUMBER_OF_PACKETS, sizeof(NUMBER_OF_PACKETS)) && (PACKET_NUMBER == PACKET_NUMBER_ACK) )){
      radio.read(&PACKET_NUMBER_ACK, sizeof(PACKET_NUMBER_ACK));
      delay(10);
     }*/

     while(true){
      delay(150);
      if (radio.write(&buff, sizeof(buff))){
        radio.read(&PACKET_NUMBER_ACK, sizeof(PACKET_NUMBER_ACK));
        radio.flush_rx();
        radio.flush_tx();
     //   Serial.println(PACKET_NUMBER_ACK);
      //  Serial.println(PACKET_NUMBER);
        break ; 
     /*   if (PACKET_NUMBER_ACK == PACKET_NUMBER){
          break;
        }
        else{
          delay(500);
        }*/
        
      }
     }
     


    }
    // close the file:
    myFile.close();
    
  }

    
  
  
  else {
    // if the file didn't open, print an error:
    Serial.print("TERM:error opening GATEWAY.txt");
  }
    SD.remove("GATEWAY.txt"); // REMOVE PREVIOUS FILE 
    Serial.print("TERM:TRANSMIT_SUCESSFULLY ");
    STATUS_FLAG = "NOT_TRANSMITT";
  }
}
