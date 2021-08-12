//田同學  2021/8/12 夜
unsigned long p0_t = 0;
unsigned long p0_t1= 0;
unsigned long p1_t= 0;
unsigned short rpm= 0;
bool p0_flag = true;
bool p1_flag = true;

void p0_ISR() {
  if (p0_flag) {
    p0_t = micros();
  }
  else {
    p0_t1 = micros() - p0_t;
    rpm = 60000000UL / p0_t1;
  }
  p0_flag = !p0_flag;
}
void p1_ISR() {
  p1_t = micros();
  p1_flag = true;
}
void setup() {
  pinMode(D3,INPUT);
  pinMode(D4,INPUT);
  attachInterrupt(0, &p0_ISR, RISING); //D3 接訊號
  //attachInterrupt(digitalPinToInterrupt(D4), &p1_ISR, RISING); //D4 接CDI
  Serial.begin(9600);
}
void loop() {
  //noInterrupts();
  //interrupts();
  if (p1_flag) {
    //p1_flag = false;
    Serial.print(rpm);
    Serial.println(" ");
    //Serial.println(p1_t - p0_t); //duty
  }
  
}
