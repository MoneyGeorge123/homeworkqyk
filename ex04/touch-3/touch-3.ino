#define TOUCH_PIN 4
#define LED_PIN 2
#define THRESHOLD 500 // 需要根据实际测试修改此阈值

bool ledState = false;
int touchValue;
int counter=0;

// 中断服务函数 (ISR)
void gotTouch() {
  delay(100);
  touchValue= touchRead(TOUCH_PIN);
  if(touchValue<THRESHOLD)
  {
    ledState=!ledState;
    digitalWrite(LED_PIN,ledState);
    Serial.print("中断\n");
  }
  /*counter++;
  if(counter%2==1) 
  {
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
    Serial.print("中断\n");
  }*///可以使用touchvalue的值大于或小于阈值来判断是手放上去还是松开，为了防止抖动，可以delay100左右等到它稳定
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  pinMode(LED_PIN, OUTPUT);
  
  // 绑定中断函数
touchAttachInterrupt(TOUCH_PIN, gotTouch, THRESHOLD);   
}

void loop() {
  touchValue = touchRead(TOUCH_PIN);
  Serial.print("Touch Value: ");
  Serial.println(touchValue);
  // 主循环可以处理其他任务，或者置空
  // 注意：在某些ESP32核心版本中，loop中可能需要保留简单的代码以保持看门狗复位
  delay(100);
}