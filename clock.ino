#include <util/delay.h>
#include <avr/interrupt.h>
#include <Wire.h> // Used to establied serial communication on the I2C bus
#include "SparkFunTMP102.h" 
#include <PCD8544.h>
#include <DS1307.h>

volatile unsigned long time1 = 0;
volatile unsigned long time2 = 0;

DS1307 rtc(A2, A3); //clock
Time t; //for time setting
TMP102 sensor0(0x48); //temperature sensor
static PCD8544 lcd; //lcd
const int buzzer = 9;

volatile int zile_luna [] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31,
30, 31};
volatile int fusOrar; // TRUE 24H, false 12h
volatile char fus_test;
volatile byte set_mode;
volatile byte perioada;
static const byte LCD_WIDTH = 84;
static const byte LCD_HEIGHT = 48;

// The number of lines for the temperature chart...
static const byte CHART_HEIGHT = 5;

// A custom "degrees" symbol...
static const byte DEGREES_CHAR = 1;
static const byte degrees_glyph[] = { 0x00, 0x07, 0x05, 0x07, 0x00 };

// A bitmap graphic (10x2) of a thermometer...
static const byte THERMO_WIDTH = 10;
static const byte THERMO_HEIGHT = 2;
static const byte thermometer[] = { 0x00, 0x00, 0x48, 0xfe, 0x01, 0xfe, 0x00, 0x02, 0x05, 0x02,
                                    0x00, 0x00, 0x62, 0xff, 0xfe, 0xff, 0x60, 0x00, 0x00, 0x00};
void setup(){

  EIMSK |= (1 << INT0);
  EIMSK |= (1 << INT1);

  //PIN 10 int

  PCICR |= (1 << PCIE0);
  PCMSK0 |= (1<<PCINT2);

  //PIN 11 int
  PCICR |= (1 << PCIE0);
  PCMSK0 |= (1<<PCINT3);

  //normal function mode
  set_mode = 0;
  
  lcd.begin(LCD_WIDTH, LCD_HEIGHT);
   // Register the custom symbol...
  lcd.createChar(DEGREES_CHAR, degrees_glyph);
  sensor0.begin();  // Join I2C bus

  // set the Conversion Rate (how quickly the sensor gets a new reading)
  //0-3: 0:0.25Hz, 1:1Hz, 2:4Hz, 3:8Hz
  sensor0.setConversionRate(2);
  
  //set Extended Mode.
  //0:12-bit Temperature(-55C to +128C) 1:13-bit Temperature(-55C to +150C)
  sensor0.setExtendedMode(0);

  //set T_HIGH, the upper limit to trigger the alert on
  //sensor0.setHighTempF(85.0);  // set T_HIGH in F
  sensor0.setHighTempC(30.4); // set T_HIGH in C
  
  //set T_LOW, the lower limit to shut turn off the alert
  //sensor0.setLowTempF(84.0);  // set T_LOW in F
  sensor0.setLowTempC(10.67); // set T_LOW in C

  //Time set
  //rtc.halt(false);
//  
//      rtc.setDOW(MONDAY);      //day of week
//      rtc.setTime(11, 37, 20);     //time set
//      rtc.setDate(06, 11, 2017);   //date set
// 
 
}
void loop(){
  float temp;
  fusOrar = 0;
  // Turn sensor on to start temperature measurement.
  // Current consumtion typically ~10uA.
  sensor0.wakeup();

  // read temperature data
  //temperature = sensor0.readTempF();
  temp = sensor0.readTempC();
  
  // Print the temperature (using the custom "degrees" symbol)...
  lcd.setCursor(25, 10);
  lcd.print(temp, 1);
  lcd.print(" \001C ");

  // Draw the thermometer bitmap at the bottom left corner...
  lcd.setCursor(13, LCD_HEIGHT/8 - THERMO_HEIGHT);
  lcd.drawBitmap(thermometer, THERMO_WIDTH, THERMO_HEIGHT);

  // Place sensor in sleep mode to save power.
  // Current consumtion typically <0.5uA.
  sensor0.sleep();
  t = rtc.getTime();
  timp(t);
 
  lcd.setCursor(20, 1);
  lcd.println(rtc.getDateStr(FORMAT_SHORT));
  lcd.setCursor(20, 2);
  lcd.println(rtc.getDOWStr(FORMAT_SHORT));

  
  
  if (set_mode!=0)
 {
   lcd.setCursor(25,3);
   lcd.print("SET ");
   switch (set_mode) {
     case 1:
     lcd.print("Z");
     break;
     case 2:
     lcd.print("L");
     break;
     case 3:
     lcd.print("A");
     break;
     case 4:
     lcd.print("O");
     break;
     case 5:
     lcd.print("M");
     break;
     case 6:
     lcd.print("S");
     break;
     case 7:
     lcd.print("Day");
     break;
     
     
   }
 }
  delay(1000);  // Wait 1000ms
  lcd.clear();
}

void timp(Time t){
  lcd.setCursor(19, 0);
  if(fusOrar == 0){
    
    lcd.print(rtc.getTimeStr());
  }
  
  else{
    if(t.hour + 1 > 12){
    perioada = 1;
    fus_test == 'P';
  }
    else{
      perioada = 0;
      fus_test = 'A';
    }
  
    if(perioada == 0 && fus_test == 'A'){
          lcd.print(t.hour);
          lcd.print(":");
          lcd.print(t.min);
          lcd.print(":");
          lcd.print(t.sec);
          lcd.print(" AM");
         
      }
    if (perioada == 1 && fus_test == 'P' ){
          lcd.print(t.hour - 12 );
          lcd.print(":");
          lcd.print(t.min);
          lcd.print(":");
          lcd.print(t.sec);
          lcd.print(" PM");
          
      }
    if (t.hour == 0 && fus_test == 'P') {
        lcd.print(12);
          lcd.print(":");
          lcd.print(t.min);
          lcd.print(":");
          lcd.print(t.sec);
          lcd.print(" PM");
      }
    
      
  }
}


ISR(INT0_vect) {
 set_mode++;
 if (set_mode==8) set_mode=0;
 _delay_ms(400);
}


ISR(INT1_vect) {
  t = rtc.getTime();
  if (set_mode != 0)
    switch (set_mode){
      case 1:
          
         if(t.year == 2020) zile_luna[1] = 29;
         else zile_luna[1] = 28; 
         if ((t.date + 1) > (zile_luna[t.mon-1]))  t.date = 1 ;
         else  t.date ++;
         rtc.setDate(t.date,t.mon,t.year);
        break;
      case 2:
        if ((t.mon + 1) > 12) t.mon = 1;
        else
          t.mon ++;
        rtc.setDate(t.date,t.mon,t.year);

        break;
      case 3:
        t.year ++;
        rtc.setDate(t.date,t.mon,t.year);

        break;
      case 4:
      
      if(fusOrar == 0){
        if((t.hour + 1) > 23) t.hour = 0;       
       else 
        t.hour ++;
        rtc.setTime(t.hour, t.min, t.sec);
      } 
      if(fusOrar == 1){
        if(t.hour < 13) perioada = 0;
        else if(t.hour > 12 || t.hour == 0) perioada = 1;
          if((t.hour + 1) > 12){
            t.hour = 1;
            
            if(perioada == 0){
              fus_test = 'P';
              perioada = 1;
            }
            else {
              fus_test = 'A';
              perioada = 0;
            }
          }
          else t.hour ++;
        rtc.setTime(t.hour, t.min, t.sec);
      }    
        
      break;
      case 5:
        if ((t.min + 1) > 59) t.min = 0;
        else
          t.min ++;
        rtc.setTime(t.hour, t.min, t.sec);
        break;
      case 6:
      if ( t.sec + 1 > 59 ) t.sec = 0;
      else
        t.sec ++;
        
        rtc.setTime(t.hour, t.min, t.sec);
        break;

      case 7:
        if((t.dow + 1) > 7) t.dow = 1;
        else t.dow ++;
        rtc.setDOW(t.dow);
      
      }


    else{
      
    
//AM PM
//24 h
  if(fusOrar + 1 > 1) fusOrar = 0;
  else fusOrar++;
  
}
_delay_ms(400);
}




