//用來作排序的套件
//數位模組
#include <TM1637.h>
//用來偵測 D1 腳抓脈衝凸台的訊號

unsigned long checkZeroCounts = 0; //用來檢查引擎是不是熄了，連續一段時間沒偵測到轉速， loop 就重來

//範例：https://github.com/Seeed-Studio/Grove_4Digital_Display/blob/master/examples/DisplayNum/DisplayNum.ino
//範例：https://github.com/Seeed-Studio/Grove_4Digital_Display/blob/master/examples/DisplayStr/DisplayStr.ino
//用來處理 4 Digit-Diplay TM1637 的部分
//給 4 Digit-Display TM1637 使用
const int FIRE_SIG = D0; //偵測點火
const int TO_SIG = D1; //凸台，脈衝
#define CLK D4 
#define DIO D5 
TM1637 tm1637(CLK,DIO);

unsigned long start = 0;
unsigned long pulseCounts = 0;
unsigned long st = 0; //用來紀錄時間序

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);    

  // 4 Digit-Display
  tm1637.init();  
  tm1637.set(BRIGHT_TYPICAL); //BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;
  playFirstTime();
  playFirstTime();
  //回歸零
  tm1637.display(0,-1); //-1 for blank
  tm1637.display(1,-1); //-1 for blank
  tm1637.display(2,-1); //-1 for blank
  tm1637.display(3,0);
     
  //讀入脈衝
  pinMode(TO_SIG, INPUT);  

  //讀入點火
  pinMode(FIRE_SIG, INPUT);  
}
void playFirstTime()
{
  // 0000~9999 跑二次
  for(int i=0;i<= 9;i++)
  {
    for(int j=0;j<4;j++)
    {
      tm1637.display(j,i);    
    }
    delay(100);  
  }
}

void diaplayOnLed(int show_rpm)
{
  //將轉速，變成顯示值
  //只顯示 萬千百十
  //如果要顯示 千百十個，就不用除了
  //太多數位有點眼花
  String rpm_str = String(show_rpm/10);
  if(rpm_str.length()<=3)
  {
    rpm_str = lpad(rpm_str,4,"X"); // 變成如 "XXX0"
  }
  Serial.print("\nAfter lpad:");
  Serial.println(rpm_str);
  for(int i=0;i<4;i++)
  { 
      if(rpm_str[i]=='X')
      {
        tm1637.display(i,-1); //-1 代表 blank 一片黑    
      }
      else
      {
        // Serial.println(rpm_str[i]);
        // 腦包直接轉回 String 再把單字轉 int
        // From : https://www.arduino.cc/en/Tutorial.StringToIntExample
        tm1637.display(i,String(rpm_str[i]).toInt());    
      }
  }
}
String lpad(String temp , byte L , String theword){
  //用來補LED左邊的空白
  byte mylen = temp.length();
  if(mylen > (L - 1))return temp.substring(0,L-1);
  for (byte i=0; i< (L-mylen); i++) 
    temp = theword + temp;
  return temp;
}

boolean get_TO_PulseStatus(){
  //取得脈衝訊號是否為高電位
  return (digitalRead(TO_SIG)==HIGH);
}
boolean get_FIRE_PulseStatus(){
  //取得點火是否為高電位
  return (digitalRead(FIRE_SIG)==HIGH);
}


void loop() {
  //首次一定要等到凸台開始才算開始
  checkZeroCounts = 0;
  while(!get_TO_PulseStatus()) {   //等待 Low，隔一次偵測，首次不管
    //當轉速訊號消失一段時間後，轉速表也要歸零
    checkZeroCounts++;
    if(checkZeroCounts>= 600000){ //超時，低於50rpm
      //直接輸出 0 
      checkZeroCounts = 0;      
      diaplayOnLed(checkZeroCounts);      
      return;
    }
  };
  checkZeroCounts = 0;
  while(get_TO_PulseStatus()){ start = micros(); //凸台變正了，抓時間
    //凸開變正了，開始計數    
    while(!get_FIRE_PulseStatus()){ 
      //一直等到點火發生
      checkZeroCounts++;
      if(checkZeroCounts>= 600000){ //超時，低於50rpm = 1200000 ,  1500rpm = 40000
        //直接輸出 9999
        checkZeroCounts = 9999;    
        int show = (int)(map(checkZeroCounts,1500,16000,40000,3750) / 100);    
        show=(show>9999)?9999:show;
        diaplayOnLed(show);      
        return;
      }
    }
  }
  
  pulseCounts=micros()-start; //總算抓到一個 pulse 時間，用現在時間減去開始時間
  /*
   * 參考：http://stm32-learning.blogspot.com/2014/05/arduino.html   
   * 轉速  1500 轉 = 每分鐘  1500 轉，每秒  25   轉，1轉多少秒呢，一轉 = 0.04       秒 =  40.000 ms =  40000us
   * 轉速  6000 轉 = 每分鐘  6000 轉，每秒  60   轉，1轉多少秒呢，一轉 = 0.01666... 秒 =  16.667 ms =  16667us
   * 轉速 14000 轉 = 每分鐘 14000 轉，每秒 233.3 轉，1轉多少秒呢，一轉 = 0.0042863. 秒 =   4.286 ms =   4286us
   * 轉速 16000 轉 = 每分鐘 16000 轉，每秒 266.6 轉，1轉多少秒呢，一轉 = 0.0037500. 秒 =   3.750 ms =   3750us
   */ 

  //讓 LED 不要刷的太快，太快眼睛跟不上字會黏在一起
  if (micros()-st > 55000) //550ms 眼睛才受的了
  {       
    int show = (int)(map(checkZeroCounts,1500,16000,40000,3750) / 100);    
    show=(show>9999)?9999:show;
    diaplayOnLed(show);          
    //在 TM1637 顯示   
    Serial.print("show: ");   
    Serial.print(show);
    Serial.println();   
    st = micros(); 
  }  
}
