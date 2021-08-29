#include <Arduino.h>
//數位模組
#include <TM1637.h>
const int ToPin = D7;  //凸台
const int FirePin = D2;  //點火
//0~14000
volatile float now_degree = 12;
volatile const float degree[16] = {6, 12, 12, 12, 17, 27, 27, 25, 23, 20, 17, 13, 10, 8, 8, 8};
//volatile const float degree[16] = {56,55,56,55,54,52.23,39.15,38.74,41.29,44.86,48.48,52.21,56.43,59.74,59.74,59.74,59.74};
//volatile float rdegree[15] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; //60 - degree
volatile const float fullAdv = 56;
volatile int fireTimes = 0;
#define CLK D6
#define DIO D5
TM1637 tm1637(CLK, DIO);
volatile unsigned long C = micros(), rpm = 0, RPM_DELAY = 0, isShowCount = 0, RPM_START_TIME = 0, C_old = 0;
volatile double CDI_DELAY = 0;
volatile bool CFLAG = false;
void ICACHE_RAM_ATTR countup() {  //For newest version
  //收到CDI點火，扣掉偵測到凸台RISING時間
  //只要是Rising就是Fire

  C = micros();
  // (1/(17000/60)0 *1000 * 1000 = 3529
  // 不可能有超過 17000rpm 的狀況
  if(C - C_old < 3529) return;
  RPM_DELAY = C - C_old;
  rpm = 60000000UL / RPM_DELAY;
  C_old = C;
  digitalWrite(FirePin, LOW);
  if(rpm>17000)
  {
    //不可能的
    return;
  }
  if(rpm<80)
  {
    //太低了，熄火吧
    return;
  }
  if (rpm != 0)
  {
    int index = (int)rpm / 1000;
    int index_n = index + 1;
    index = (index >= 16) ? 15 : index;
    if (index_n >= 16) index_n = 15;
    long s = index * 1000;
    long e = index_n * 1000;
    long ss = degree[index] * 1000;
    long ee = degree[index_n] * 1000;
    double r = map(int(rpm), s, e, ss, ee) / 1000.0;    
    now_degree = r;

    //新的 CDI 計算方法
    //delay = adv/360*轉速一圈時間
    //rpm 一圈時間 * r * 360)

    
    CDI_DELAY = (1000000.0 / (float(rpm) / 60.0)) * ((fullAdv - r)/ 360.0);
    delayMicroseconds(long(CDI_DELAY));
    digitalWrite(FirePin, HIGH);
    //點火持續時間
    //固定點 200us
    //fireTimes = (int)(map(rpm,600,14000,350,150));
    delayMicroseconds(200);
    digitalWrite(FirePin, LOW);
  }

}



void setup() {
  // put your setup code here, to run once:
  Serial.begin(250000);
  Serial.println("Counting...");
  pinMode(ToPin, INPUT_PULLUP);
  pinMode(FirePin, OUTPUT);
  digitalWrite(FirePin, LOW);
  //degree to rdegree
  //for(int i=0;i<15;i++)
  //{
  //rdegree[i] = 32 + degree[i];
  //}


  attachInterrupt(digitalPinToInterrupt(ToPin), countup, RISING); //RISING
  tm1637.init();
  tm1637.set(BRIGHT_TYPICAL); //BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;
  //playFirstTime();
  diaplayOnLed(0);
}

void loop() {

  // put your main code here, to run repeatedly:
  if (isShowCount > 100)
  {
    //display_rpm();
    isShowCount = 0;    
    Serial.print(rpm);
    Serial.print(" ");
    Serial.print(now_degree);
    Serial.print(" ");
    Serial.print(CDI_DELAY);
    Serial.println("");
    if(micros()-C_old>=500000)
    {
      //低於 100 轉以下
      diaplayOnLed(0);
    }
    else
    {
      diaplayOnLed(rpm);
    }
  }
  isShowCount++;
  //200rpm ~= 300003us
  
}
void playFirstTime()
{
  // 0000~9999 跑二次
  for (int i = 0; i <= 9; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      tm1637.display(j, i);
    }
    delay(100);
  }
}
void display_rpm() {
  if (rpm > 30000) return;
  //if(duty>=500000) return;
  Serial.print("rpm: ");
  Serial.print(rpm);
  Serial.print(" , CDI_DELAY: ");
  Serial.println(CDI_DELAY);
  //Serial.print(" , duty: ");
  //Serial.println(duty);
}
void diaplayOnLed(int show_rpm)
{
  //將轉速，變成顯示值
  //只顯示 萬千百十
  //如果要顯示 千百十個，就不用除了
  //太多數位有點眼花
  //String rpm_str = String(show_rpm/10);
  String rpm_str = String(show_rpm);
  if (rpm_str.length() <= 3)
  {
    rpm_str = lpad(rpm_str, 4, "X"); // 變成如 "XXX0"
  }
  //Serial.print("\nAfter lpad:");
  //Serial.println(rpm_str);
  for (int i = 0; i < 4; i++)
  {
    if (rpm_str[i] == 'X')
    {
      tm1637.display(i, -1); //-1 代表 blank 一片黑
    }
    else
    {
      // Serial.println(rpm_str[i]);
      // 腦包直接轉回 String 再把單字轉 int
      // From : https://www.arduino.cc/en/Tutorial.StringToIntExample
      tm1637.display(i, String(rpm_str[i]).toInt());
    }
  }
}
String lpad(String temp , byte L , String theword) {
  //用來補LED左邊的空白
  byte mylen = temp.length();
  if (mylen > (L - 1))return temp.substring(0, L - 1);
  for (byte i = 0; i < (L - mylen); i++)
    temp = theword + temp;
  return temp;
}
