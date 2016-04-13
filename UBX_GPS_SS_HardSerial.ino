#include <Adafruit_SSD1306.h>

#define OLED_RESET 9
Adafruit_SSD1306 display(OLED_RESET);

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

int intpin = 12;
bool oldState = false;

const unsigned char UBX_HEADER[] = { 0xB5, 0x62 };

struct NAV_PVT {
  unsigned char cls;
  unsigned char id;
  unsigned short len;
  unsigned long iTOW;
  unsigned short year;
  byte month;
  byte day;
  byte hour;
  byte miniute;
  byte sec;
  unsigned char valid;
  unsigned long tAcc;
  long nano;
  byte fixType;
  unsigned char flag;
  unsigned char flag2;
  byte numSV;
  long lon;
  long lat;
  long height;
  long hMSL;
  unsigned long hAcc;
  unsigned long vAcc;
  long velN;
  long velE;
  long velD;
  long gSpeed;
  long headMot;
  unsigned long sAcc;
  unsigned long headAcc;
  unsigned short pDop;
  byte reserved1[6];
  long headVeh;
  byte reserved2[4];
};

NAV_PVT pvt;

void calcChecksum(unsigned char* CK) {
  memset(CK, 0, 2);
  for (int i = 0; i < (int)sizeof(NAV_PVT); i++) {
    CK[0] += ((unsigned char*)(&pvt))[i];
    CK[1] += CK[0];
  }
}

bool processGPS() {
  static int fpos = 0;
  static unsigned char checksum[2];
  const int payloadSize = sizeof(NAV_PVT);
  
  while ( Serial.available() ) {
    byte c = Serial.read();
    if ( fpos < 2 ) { //SYNC
      if ( c == UBX_HEADER[fpos] )
        fpos++;
      else
        fpos = 0;
    }
    else {
      if ( (fpos-2) < payloadSize )
        ((unsigned char*)(&pvt))[fpos-2] = c;

      fpos++;

      if ( fpos == (payloadSize+2) ) {
        calcChecksum(checksum);
      }
      else if ( fpos == (payloadSize+3) ) {
        if ( c != checksum[0] )
          fpos = 0;
      }
      else if ( fpos == (payloadSize+4) ) {
        fpos = 0;
        if ( c == checksum[1] ) {
          return true;
        }
      }
      else if ( fpos > (payloadSize+4) ) {
        fpos = 0;
      }
    }
  }
  return false;
}

void PrintScreen()
{
  display.clearDisplay();

  //Size 2
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  float sik = (pvt.gSpeed * 0.001) * 3.6f;
  if (sik<100)
    display.print(" ");
  if (sik<10)
    display.print(" ");
  display.print(sik,1);//SpeedInKph
  display.setCursor(84,0);
  if (pvt.numSV<10)
    display.print(" ");
  display.print(pvt.numSV);//SV
  
  //Size 1
  display.setTextSize(1);
  display.setCursor(60,8);
  display.print("km/h");
  display.setCursor(63,0);
  display.print("CHL");
  display.setCursor(108,8);
  display.print("SVs");
  display.setCursor(108,0);
  switch (pvt.fixType){
    case 0:
      display.print("N.F");
      break;
    case 2:
      display.print("2D");
      break;
    case 3:
      display.print("3D");
      if ((byte)pvt.flag & 2)
        display.print("+");
      break;
    default:
      display.print("INV");
      break;
  }
    
    
  
  long  temp;
  //Line 1
  display.setCursor(0,16);
  if (pvt.lat >= 0)
    temp = pvt.lat;
  else
    temp = -pvt.lat;
  if (temp >= 1000000000)
    display.print("lat:");
  else 
    if (temp >= 100000000)
      display.print("lat: ");
    else
      display.print("lat:  ");
  display.print(temp / 10000000);display.print(".");
  long temp2 = 1000000;
  temp = temp % 10000000;
  while ((temp / temp2 == 0)&&(temp2>1))
  {
    display.print("0");
    temp2 /= 10;
  }
  display.print(temp);
  if (pvt.lat >= 0)
    display.print("N");
  else
    display.print("S");
  temp = 10000;
  while (((pvt.pDop / temp) == 0)&&(temp>1))
  {
    display.print(" ");
    temp /= 10;
  }
  display.println(pvt.pDop);

  //Line2
  if (pvt.lon >= 0)
    temp = pvt.lon;
  else
    temp = -pvt.lon;
  if (temp >= 1000000000)
    display.print("lon:");
  else 
    if (temp >= 100000000)
      display.print("lon: ");
    else
      display.print("lon:  ");
  display.print(temp / 10000000);display.print(".");
  temp2 = 1000000;
  temp = temp % 10000000;
  while ((temp / temp2 == 0)&&(temp2>1))
  {
    display.print("0");
    temp2 /= 10;
  }
  display.print(temp);
  if (pvt.lon >= 0)
    display.print("E");
  else
    display.print("W");
  display.println(" HEAD");

  //Line3
  display.print("hMSL:");
  if (pvt.hMSL < 0)
    pvt.hMSL = -pvt.hMSL;
  temp = 10000000;
  while (((pvt.hMSL / temp) == 0)&&(temp>1000))
  {
    display.print(" ");
    temp /= 10;
  }
  display.print((pvt.hMSL % 100000000) / 1000);display.print(".");
  if ((pvt.hMSL % 1000) < 100)
    display.print("0");
  if ((pvt.hMSL % 1000) < 10)
    display.print("0");
  display.print(pvt.hMSL % 1000);
  display.print("|");
  temp = pvt.headMot / 100000;
  if (temp < 100)
    display.print(" ");
  if (temp < 10)
    display.print(" ");
  display.print(temp);display.print(".");
  temp2 = (pvt.headMot%100000)/1000;
  if (temp2<0)
    temp2 = -temp2;
  if (temp2<10)
    display.print("0");
  display.println(temp2);

  //Line4
  display.print("h,vAcc:");
  if (pvt.hAcc >= 100000)
    display.print("  >100");
  else
  {
    display.print(pvt.hAcc / 1000);display.print(".");
    if (pvt.hAcc % 1000 < 100)
      display.print("0");
    if (pvt.hAcc % 1000 < 10)
      display.print("0");
    display.print(pvt.hAcc % 1000);
  }
  display.print(",");
  if (pvt.vAcc >= 100000)
    display.print(" >100");
  else
  {
    display.print(pvt.vAcc / 1000);display.print(".");
    if (pvt.vAcc%1000 < 100)
      display.print("0");
    if (pvt.vAcc%1000 < 10)
      display.print("0");
    display.print(pvt.vAcc%1000);
  }
  display.println("m");

  //Line5
  display.print("V:");
  if (pvt.gSpeed < 1000000)
    display.print(" ");
  if (pvt.gSpeed < 100000)
    display.print(" ");
  if (pvt.gSpeed < 10000)
    display.print(" ");
  display.print((pvt.gSpeed % 10000000)/1000);display.print(".");
  if (pvt.gSpeed % 1000 < 100)
    display.print("0");
  if (pvt.gSpeed % 1000 < 10)
    display.print("0");
  display.print(pvt.gSpeed % 1000);
  display.print("m/s");
  if (pvt.sAcc < 1000000)
  {
    if (pvt.sAcc < 100000)
      display.print(" ");
    if (pvt.sAcc < 10000)
      display.print(" ");
    display.print("~");
    display.print(pvt.sAcc / 1000);display.print(".");
    if (pvt.sAcc % 1000 < 100)
      display.print("0");
    if (pvt.sAcc % 1000 < 10)
      display.print("0");
    display.println(pvt.sAcc % 1000);
  }
  else
  display.println(" ~>100");
  
  //Line6
  if ((pvt.year<10000)&&(pvt.year>=1000))
    display.print(pvt.year);
  else
    display.print("????");
  display.print("/");
  if ((pvt.month<=12)&&(pvt.month>=1))
  {
    if (pvt.month < 10)
      display.print("0");
    display.print(pvt.month);
  }
  else
    display.print("??");
  display.print("/");
  if ((pvt.day<=31)&&(pvt.day>=1))
  {
    if (pvt.day < 10)
      display.print("0");
    display.print(pvt.day);
  }
  else
    display.print("??");
  if (pvt.valid & 0x04)
    display.print("UTC");
  else
    display.print("INV");
  if ((pvt.hour<=23)&&(pvt.hour>=0))
  {
    if (pvt.hour < 10)
      display.print("0");
    display.print(pvt.hour);
  }
  else
    display.print("??");
  display.print(":");
  if ((pvt.miniute<=59)&&(pvt.miniute>=0))
  {
    if (pvt.miniute < 10)
      display.print("0");
    display.print(pvt.miniute);
  }
  else
    display.print("??");
  display.print(":");
  if ((pvt.sec<=60)&&(pvt.sec>=0))
  {
    if (pvt.sec < 10)
      display.print("0");
    display.print(pvt.sec);
  }
  else
    display.print("??");
  
  display.display();
}

void PrintSec()
{
  pvt.sec = (pvt.sec + 1)%60;
  display.fillRect(114,56,12,8,BLACK);
  display.setCursor(114,56);
  if (pvt.sec<10)
    display.print("0");
  display.println(pvt.sec);
  display.display();
}

void setup() 
{
  Serial.begin(115200);
  pinMode(intpin, INPUT);
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); 
  // init done
}

void loop() {
  bool state = digitalRead(intpin);
  if (oldState != state){
    oldState = state;
    //RISING
    if (state)
      PrintSec();
  }
  if ( processGPS() ) 
    PrintScreen();
}

