// 定义LED引脚，ESP32通常板载LED连接在GPIO 2
const int ledPin = 2; 
unsigned long previous =0;
int s1=0,O=0,s2=0;
unsigned long current=0;
void setup() {
  // 初始化串口通信，设置波特率为115200
  Serial.begin(115200);
  // 将LED引脚设置为输出模式
  pinMode(ledPin, OUTPUT);
}

void loop() {
  while(s1<3)//第一次S
  {
    current=millis();
    if((current-previous)>=0&&(current-previous)<100){digitalWrite(ledPin,HIGH);}
    if((current-previous)>=100&&(current-previous)<200){digitalWrite(ledPin,LOW);}
    if((current-previous)>=200){previous=current;s1++;}
  }
  if(s1==3)s1=0;


  while(current-previous<500)//暂停500ms
  {
    current=millis();
  }
  previous=current;


  while(O<3)
  {
    current=millis();
    if((current-previous)>=0&&(current-previous)<300){digitalWrite(ledPin,HIGH);}
    if((current-previous)>=300&&(current-previous)<600){digitalWrite(ledPin,LOW);}
    if((current-previous)>=600){previous=current;O++;}
  }
  if(O==3)O=0;


  while(current-previous<500)//暂停500ms
  {
    current=millis();
  }
  previous=current;


  while(s2<3)
  {
    current=millis();
    if((current-previous)>=0&&(current-previous)<100){digitalWrite(ledPin,HIGH);}
    if((current-previous)>=100&&(current-previous)<200){digitalWrite(ledPin,LOW);}
    if((current-previous)>=200){previous=current;s2++;}
  }
  if(s2==3)s2=0;
  while(current-previous<2000)//暂停500ms
  {
    current=millis();
  }
  previous=current;
}