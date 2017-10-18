#include <SoftwareSerial.h>
#include <TinyGPS.h>

SoftwareSerial BTSerial(2, 3);
SoftwareSerial GPSSerial(7, 8);
// 시리얼을 두개 이상 사용할 때 http://m.blog.naver.com/roboholic84/220550737629
TinyGPS gps;

static void smartdelay(unsigned long ms);
static void print_float(float val, float invalid, int len, int prec);
static void print_int(unsigned long val, unsigned long invalid, int len);
static void print_date(TinyGPS &gps);
static void print_str(const char *str, int len);
static String makeHex(long value);

void setup()
{
  Serial.begin(9600);
  BTSerial.begin(9600);
  
  GPSSerial.begin(9600);
  delay(1000);
  
  BTSerial.listen();

  BTSerial.write("AT+DELO2");
  BTSerial.flush();
  delay(1000);

  BTSerial.write("AT+IBEA1");
  BTSerial.flush();
  delay(1000);

  BTSerial.write("AT+POWE3");
  BTSerial.flush();
  delay(1000);

  BTSerial.write("AT+PWRM1");
  BTSerial.flush();
  delay(1000);

  BTSerial.write("AT+ROLE0");
  BTSerial.flush();
  delay(1000);

  BTSerial.write("AT+ADTY3");
  BTSerial.flush();
  delay(1000);

  BTSerial.write("AT+ADVI0");
  BTSerial.flush();
  delay(1000);

  BTSerial.write("AT+ROLE0");
  BTSerial.flush();
  delay(1000);

}

void loop()
{ 
Serial.println();
Serial.println("---------------------------------------");

  GPSSerial.listen();
  smartdelay(1000);
  float flat, flon;
  unsigned long age;
  
  gps.f_get_position(&flat, &flon, &age);
  Serial.print(flat,10);
  // 6자리로 이상의 소수점 단위는 가만히 있어도 오차가 발생하는 정도. 완전 정확할 수 없다.
  // flat 값을 소수점 이하 6자리까지만 저장해서
  // 16진수로 변환해야 함.
  Serial.println();
  Serial.println(TinyGPS::GPS_INVALID_F_ANGLE);
  

  Serial.println();
  Serial.println("위도");
  print_float(flat, TinyGPS::GPS_INVALID_F_ANGLE, 10, 6);
  
  Serial.println();
  Serial.println("경도");
  print_float(flon, TinyGPS::GPS_INVALID_F_ANGLE, 11, 6);

  Serial.println();
  Serial.println("고도");
  print_int(age, TinyGPS::GPS_INVALID_AGE, 5);

  long tempFlat = flat*1000000;
  long tempFlon = flon*1000000;
  //Serial.println(tempFlat);
  String flatHex = makeHex(tempFlat);
  String flonHex = makeHex(tempFlon);
  //Serial.println(flatHex);
  String comFlat = String("AT+IBE0" + flatHex);
  String comFlon = String("AT+IBE1" + flonHex);
  Serial.println(comFlat);
  Serial.println(comFlon);
  byte flatBuff[16];
  byte flonBuff[16];
  comFlat.getBytes(flatBuff,16); 
  comFlon.getBytes(flonBuff,16); 

  BTSerial.listen();
  BTSerial.flush();
  BTSerial.write(flatBuff,15);
  BTSerial.flush();
  delay(1000);
  BTSerial.write(flonBuff,15);
  delay(3000);
  
    if(BTSerial.available()){
    Serial.write(BTSerial.read());
  }

  if(Serial.available()){
    BTSerial.write(Serial.read());
  }
  Serial.println();
}

///////////////////--------------------------- 함수 ------------------------------/////////////////////////////

static void smartdelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (GPSSerial.available())
      gps.encode(GPSSerial.read());
  } while (millis() - start < ms);
}

static void print_float(float val, float invalid, int len, int prec)
{
  if (val == invalid)
  {
    while (len-- > 1)
      Serial.print('*');
    Serial.print(' ');
  }
  else
  {
    Serial.print(val, prec);
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1); // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i=flen; i<len; ++i)
      Serial.print(' ');
  }
  smartdelay(0);
}

static void print_int(unsigned long val, unsigned long invalid, int len)
{
  char sz[32];
  if (val == invalid)
    strcpy(sz, "*******");
  else
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i=strlen(sz); i<len; ++i)
    sz[i] = ' ';
  if (len > 0) 
    sz[len-1] = ' ';
  Serial.print(sz);
  smartdelay(0);
}

static void print_str(const char *str, int len)
{
  int slen = strlen(str);
  for (int i=0; i<len; ++i)
    Serial.print(i<slen ? str[i] : ' ');
  smartdelay(0);
}

static String makeHex(long value){
 String hexValue = String(value, HEX);
   
 hexValue.toUpperCase();
 int lengthHex = hexValue.length();
 switch(lengthHex){
  case 7:
  hexValue = String("0"+hexValue);
  break;
    case 6:
  hexValue = String("00"+hexValue);
  break;
    case 5:
  hexValue = String("000"+hexValue);
  break;
    case 4:
  hexValue = String("0000"+hexValue);
  break;
    case 3:
  hexValue = String("00000"+hexValue);
  break;
    case 2:
  hexValue = String("000000"+hexValue);
  break;
    case 1:
  hexValue = String("0000000"+hexValue);
  break;
  }
  return hexValue;
}

