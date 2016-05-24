/*
  SD card datalogger

 This example shows how to log data from three analog sensors
 to an SD card using the SD library.

 The circuit:
 * analog sensors on analog ins 0, 1, and 2
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4

 created  24 Nov 2010
 modified 9 Apr 2012
 by Tom Igoe

 This example code is in the public domain.

 */

#include <SPI.h>
#include <SD.h>


const int chipSelect = 4;
String filename;

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

//long linecounter;

void setup() {
  // Open serial communications and wait for port to open:
  //Serial.begin(9600);
  Serial.begin(115200);


  //Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    //Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  //Serial.println("card initialized.");

  File count = SD.open("count");
  long fileCount = 0;
  char c;
  if (count){
    while (count.available()){
      c = count.read();
      if ((c>='0')&&(c<='9'))
      {
        fileCount = fileCount*10 + (c - '0');
      }
    }
  }
  count.close();

  SD.remove("count");
  fileCount++;
  count = SD.open("count",FILE_WRITE);
  if (count){
    count.print(fileCount);
  }
  count.close();
  filename = String(fileCount);
  File output = SD.open(filename,FILE_WRITE);
  if (output)
  {
    output.println("UTC,fixType,lat,lon,height,hMSL,hAcc,vAcc,VelN,VelE,VelD,gSpeed,sAcc,PDOP,numSV,DGPS");
    output.close();
  }
 // Serial.println(filename);
}

void writefile()
{
  File output = SD.open(filename,FILE_WRITE);
  if (output)
  {
    //UTC
    output.print(pvt.year);output.print("-");
    if (pvt.month<10)
      output.print(0);  
    output.print(pvt.month);output.print("-");
    if (pvt.day<10)
      output.print(0);
    output.print(pvt.day);output.print("T");
    if (pvt.hour<10)
      output.print(0);
    output.print(pvt.hour);output.print(":");
    if (pvt.miniute<10)
      output.print(0);
    output.print(pvt.miniute);output.print(":");
    if (pvt.sec<10)
      output.print(0);
    output.print(pvt.sec);
    output.print("Z,");

    //fixType
    output.print(pvt.fixType);
    output.print(",");

    //lat
    output.print(pvt.lat/10000000);output.print(".");
    long temp = 1000000;
    if (pvt.lat<0)
      pvt.lat = -pvt.lat;
    pvt.lat = pvt.lat % 10000000;
    while ((pvt.lat / temp == 0)&&(temp>1))
    {
      output.print("0");
      temp /= 10;
    }
    output.print(pvt.lat);
    output.print(",");
    //lon
    output.print(pvt.lon / 10000000);output.print(".");
    temp = 1000000;
    if (pvt.lon<0)
      pvt.lon = -pvt.lon;
    pvt.lon = pvt.lon % 10000000;
    while ((pvt.lon / temp == 0)&&(temp>1))
    {
      output.print("0");
      temp /= 10;
    }
    output.print(pvt.lon);
    output.print(",");
    //height
    output.print(pvt.height / 1000);output.print(".");
    temp = 100;
    if (pvt.height<0)
      pvt.height = -pvt.height;
    pvt.height = pvt.height % 1000;
    while ((pvt.height / temp == 0)&&(temp>1))
    {
      output.print("0");
      temp /= 10;
    }
    output.print(pvt.height);
    output.print(",");
    //hMSL
    output.print(pvt.hMSL / 1000);output.print(".");
    temp = 100;
    if (pvt.hMSL<0)
      pvt.hMSL = -pvt.hMSL;
    pvt.hMSL = pvt.hMSL % 1000;
    while ((pvt.hMSL / temp == 0)&&(temp>1))
    {
      output.print("0");
      temp /= 10;
    }
    output.print(pvt.hMSL);
    output.print(",");

    //hAcc
    output.print(pvt.hAcc / 1000);output.print(".");
    if (pvt.hAcc % 1000 < 100)
      output.print("0");
    if (pvt.hAcc % 1000 < 10)
      output.print("0");
    output.print(pvt.hAcc % 1000);
    output.print(",");
    //vAcc
    output.print(pvt.vAcc / 1000);output.print(".");
    if (pvt.vAcc % 1000 < 100)
      output.print("0");
    if (pvt.vAcc % 1000 < 10)
      output.print("0");
    output.print(pvt.vAcc % 1000);
    output.print(",");

    //VelN
    output.print(pvt.velN / 1000);output.print(".");
    if (pvt.velN % 1000 < 100)
      output.print("0");
    if (pvt.velN % 1000 < 10)
      output.print("0");
    output.print(pvt.velN % 1000);
    output.print(",");
    //VelE
    output.print(pvt.velE / 1000);output.print(".");
    if (pvt.velE % 1000 < 100)
      output.print("0");
    if (pvt.velE % 1000 < 10)
      output.print("0");
    output.print(pvt.velE % 1000);
    output.print(",");
    //VelD
    output.print(pvt.velD / 1000);output.print(".");
    if (pvt.velD % 1000 < 100)
      output.print("0");
    if (pvt.velD % 1000 < 10)
      output.print("0");
    output.print(pvt.velD % 1000);
    output.print(",");

    //gSpeed
    output.print(pvt.gSpeed / 1000);
    output.print(".");
    if (pvt.gSpeed % 1000 < 100)
      output.print("0");
    if (pvt.gSpeed % 1000 < 10)
      output.print("0");
    output.print(pvt.gSpeed % 1000);
    output.print(",");
    
    //sAcc
    output.print(pvt.sAcc / 1000);
    output.print(".");
    if (pvt.sAcc % 1000 < 100)
      output.print("0");
    if (pvt.sAcc % 1000 < 10)
      output.print("0");
    output.print(pvt.sAcc % 1000);
    output.print(",");

    //PDOP
    output.print(pvt.pDop / 100);
    output.print(".");
    if (pvt.pDop % 100 < 10)
      output.print("0");
    output.print(pvt.pDop % 100);
    output.print(",");


    output.print(pvt.numSV);
    output.print(",");
    if ((byte)pvt.flag & 2)
      output.print("+");
    else
      output.print("-");
    
    output.println();
    //linecounter++;
    //Serial.println(linecounter);
  }
  else
  {
    //Serial.println("openfail");
  }

  output.close();
}


void loop() {
  if (processGPS())
    writefile();
}

