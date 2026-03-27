
// 设置PWM属性
const int freq = 5000;          // 频率 5000Hz
const int resolution = 8;       // 分辨率 8位 (0-255)
#define TOUCH_PIN 4
#define LED_PIN 2
#define THRESHOLD 500 // 需要根据实际测试修改此阈值
int speed=1;
int touchValue;
int dutyCycle;
int counter=0;

// 中断服务函数 (ISR)
void gotTouch() {
  /*delay(100);
  touchValue= touchRead(TOUCH_PIN);
  if(touchValue<THRESHOLD)*///触摸变速
  counter++;
  if(counter%2==1)
  {
    speed=(speed%6)+2;
  }
  Serial.print("当前为第");
  Serial.println(speed/2+1);
  Serial.print("档\n");
}
void setup() {
  Serial.begin(115200);

  // 【新版用法】直接将引脚、频率和分辨率绑定
  // 它会自动返回一个关联的通道（如果需要的话）
  ledcAttach(LED_PIN, freq, resolution);
  delay(1000);
  
  // 绑定中断函数
  touchAttachInterrupt(TOUCH_PIN, gotTouch, THRESHOLD);
}

void loop() {
  // 逐渐变亮
  for(dutyCycle = 0; dutyCycle <= 255;){   
    // 【新版用法】直接通过引脚号写入，不再需要指定通道
    ledcWrite(LED_PIN, dutyCycle);   
    delay(10);
    dutyCycle=dutyCycle+speed;
  }

  // 逐渐变暗
  for(dutyCycle = 255; dutyCycle >= 0;){
    ledcWrite(LED_PIN, dutyCycle);   
    delay(10);
    dutyCycle=dutyCycle-speed;
  }
  
  Serial.println("Breathing cycle completed");
}