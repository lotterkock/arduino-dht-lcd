/*

Display temp, hum and dew on oled display 
  
*/

#include "DHT.h"

#define DHTPIN 2     // what digital pin we're connected to

#include "U8glib.h"

#include <EEPROM.h>


float t;
float h;
float d;

float tmax;
float tmin;
float hmin;
float hmax;
unsigned long time;
unsigned long time1;
int addr = 1;
byte value;
String comms;
String mem = "Mem. OK";

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0);	// I2C / TWI 
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);


void draw(void) {
  // graphic commands to redraw the complete screen should be placed here  
  u8g.setFont(u8g_font_unifont);
  //u8g.setFont(u8g_font_osb21);
  h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  
  t = dht.readTemperature();
  d = dewPoint(t,h);
  if (t>tmax){
    tmax=t;
  }
  if (t<tmin){
    tmin=t;
  }

  if (h>hmax){
    hmax=h;
  }
  if (h<hmin){
    hmin=h;
  }

  String temp_ = "T " + String(int(t)) + "(" +String(int(tmin)) + "," + String(int(tmax)) + ")";
  String hum_ = "H " + String(int(h)) + "(" +String(int(hmin)) + "," + String(int(hmax)) + ")";
  String dew_ = "D " + String(int(d));
  
  u8g.setPrintPos(0,10);
  u8g.print(temp_);
  u8g.setPrintPos(0,25);
  u8g.print(hum_);
  u8g.setPrintPos(0,40);
  u8g.print(dew_);
  
 
//Time things

  time = millis()/60000;
  u8g.setPrintPos(40,40);
  String tt =String(value,DEC);
  String ti = "T " + String(time) + "/" + tt;
  u8g.print(ti);
  
  u8g.setPrintPos(0,52);
  u8g.print(comms);
  
  // Mem state
  u8g.setPrintPos(0,64);
  String mem1 = mem + String(addr) + "/511";
  u8g.print(mem1);
  
  
//  u8g.drawFrame(95,0,32,14);

  
}

void setup(void) {
  Serial.begin(9600);
  // flip screen, if required
  // u8g.setRot180();
  
  // set SPI backup if required
  //u8g.setHardwareBackup(u8g_backup_avr_spi);

  tmax = 0;
  tmin = 100;
  hmin = 100;
  hmax = 0;
  value = EEPROM.read(0);
//  Serial.print(value, DEC);
  
  // assign default color value
  if ( u8g.getMode() == U8G_MODE_R3G3B2 ) {
    u8g.setColorIndex(255);     // white
  }
  else if ( u8g.getMode() == U8G_MODE_GRAY2BIT ) {
    u8g.setColorIndex(3);         // max intensity
  }
  else if ( u8g.getMode() == U8G_MODE_BW ) {
    u8g.setColorIndex(1);         // pixel on
  }
  else if ( u8g.getMode() == U8G_MODE_HICOLOR ) {
    u8g.setHiColorByRGB(255,255,255);
  }
}


double dewPoint(double celsius, double humidity)
{
	// (1) Saturation Vapor Pressure = ESGG(T)
	double RATIO = 373.15 / (273.15 + celsius);
	double RHS = -7.90298 * (RATIO - 1);
	RHS += 5.02808 * log10(RATIO);
	RHS += -1.3816e-7 * (pow(10, (11.344 * (1 - 1/RATIO ))) - 1) ;
	RHS += 8.1328e-3 * (pow(10, (-3.49149 * (RATIO - 1))) - 1) ;
	RHS += log10(1013.246);

        // factor -3 is to adjust units - Vapor Pressure SVP * humidity
	double VP = pow(10, RHS - 3) * humidity;

        // (2) DEWPOINT = F(Vapor Pressure)
	double T = log(VP/0.61078);   // temp var
	return (241.88 * T) / (17.558 - T);
}

void Serial_ch() {
  if (Serial.available()){
    String s = Serial.readString();
    if (s == "log") {
      Serial.println(t);
      //Send dat
      Serial.println(h);
      Serial.println(d);
      comms="connected";
    }
//    Serial.println(Serial.readString());
    else if (s == "down") {
      comms="downloading";
      download();
       
      
    }
  }
  else{
    comms="not connected";
  }
}

void download() {
  u8g.firstPage(); 
  do {
    draw();
  } while( u8g.nextPage() );
  
  for (int i=1; i <= 511; i++){
        Serial.println(EEPROM.read(i));
        //delay(50);
      }
       
}
  

void logging() {
  //time1=mill
  if ((millis()/1000)%600<=3) {
    //Serial.println((millis()/1000)%10);
    EEPROM.write(0, time);  
    EEPROM.write(addr, int(t));
    addr = addr + 1;
    if (addr == 511) {
      mem="Mem. full";
      addr = 1;
    }
  }
}
  



void loop(void) {
  // picture loop
  
  Serial_ch();
  u8g.firstPage();  
  do {
    draw();
  } while( u8g.nextPage() );
  
  // rebuild the picture after some delay
  delay(2000);
//  Serial.println(String(time));
  
  
  logging();
  
  
}

