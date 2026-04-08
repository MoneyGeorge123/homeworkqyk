#include <WiFi.h>
#include <WebServer.h>

// WiFi配置
const char* ssid = "Q";
const char* password = "scc170226";

// 引脚定义
const int LED_PIN = 2;           // LED指示灯（GPIO2）
const int TOUCH_PIN = 4;         // 触摸传感器引脚（GPIO4，TOUCH0）

// 系统状态变量
enum SystemState {
  DISARMED,      // 撤防状态
  ARMED,         // 布防状态
  ALARM          // 报警状态
};
SystemState systemState = DISARMED;

// 触摸相关变量
bool touchDetected = false;
unsigned long touchStartTime = 0;
const unsigned long TOUCH_THRESHOLD = 2000;  // 触摸持续2秒触发报警
const int TOUCH_THRESHOLD_VALUE = 500;        // 触摸阈值（值越小越敏感）

// 报警相关变量
bool isAlarmActive = false;
unsigned long lastBlinkTime = 0;
const unsigned long BLINK_INTERVAL = 200;    // 报警闪烁间隔（毫秒）

WebServer server(80);

// 调试信息
void printSystemStatus() {
  Serial.print("当前系统状态: ");
  switch(systemState) {
    case DISARMED: Serial.println("撤防"); break;
    case ARMED: Serial.println("布防"); break;
    case ALARM: Serial.println("报警"); break;
  }
  Serial.print("触摸值: ");
  Serial.println(touchRead(TOUCH_PIN));
  Serial.print("LED状态: ");
  Serial.println(digitalRead(LED_PIN) ? "亮" : "灭");
  Serial.println("----------------------");
}

// 生成网页
String makePage() {
  String stateText = "";
  String statusColor = "";
  String statusDescription = "";
  int touchValue = touchRead(TOUCH_PIN);
  bool isTouching = touchValue < TOUCH_THRESHOLD_VALUE;
  
  // 根据系统状态设置显示内容
  switch(systemState) {
    case DISARMED:
      stateText = "系统已撤防";
      statusColor = "#4CAF50";  // 绿色
      statusDescription = "系统处于撤防状态，触摸传感器不工作。";
      break;
    case ARMED:
      stateText = "系统已布防";
      statusColor = "#FF9800";  // 橙色
      statusDescription = "系统处于布防状态，检测到触摸将触发报警。";
      break;
    case ALARM:
      stateText = "⚠️ 报警中！";
      statusColor = "#F44336";  // 红色
      statusDescription = "检测到入侵！LED闪烁报警，请立即撤防。";
      break;
  }
  
  // 直接使用字符串拼接，避免复杂结构
  String html = "<!DOCTYPE html>";
  html += "<html>";
  html += "<head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>ESP32安防系统</title>";
  html += "<style>";
  html += "body { font-family: Arial; text-align: center; margin: 20px; }";
  html += ".container { max-width: 500px; margin: 0 auto; padding: 20px; border: 1px solid #ddd; border-radius: 10px; }";
  html += ".status { padding: 20px; border-radius: 5px; margin: 20px 0; color: white; font-weight: bold; }";
  html += ".buttons { margin: 20px 0; }";
  html += ".btn { padding: 10px 20px; margin: 5px; border: none; border-radius: 5px; font-size: 16px; cursor: pointer; }";
  html += "#armBtn { background: #4CAF50; color: white; }";
  html += "#armBtn:hover { background: #45a049; }";
  html += "#armBtn:disabled { background: #ccc; cursor: not-allowed; }";
  html += "#disarmBtn { background: #f44336; color: white; }";
  html += "#disarmBtn:hover { background: #d32f2f; }";
  html += "#disarmBtn:disabled { background: #ccc; cursor: not-allowed; }";
  html += ".info { text-align: left; margin: 20px 0; padding: 10px; background: #f9f9f9; border-radius: 5px; }";
  html += "</style>";
  html += "</head>";
  html += "<body>";
  html += "<div class='container'>";
  html += "<h1>ESP32安防系统</h1>";
  
  // 状态显示
  html += "<div class='status' id='statusBox' style='background:" + statusColor + "'>";
  html += stateText;
  html += "<div style='font-size: 14px;'>" + statusDescription + "</div>";
  html += "</div>";
  
  // 控制按钮
  html += "<div class='buttons'>";
  if (systemState == DISARMED) {
    html += "<button id='armBtn' class='btn' onclick=\"window.location.href='/control?action=arm'\">布防</button>";
    html += "<button id='disarmBtn' class='btn' disabled>撤防</button>";
  } else if (systemState == ARMED || systemState == ALARM) {
    html += "<button id='armBtn' class='btn' disabled>布防</button>";
    html += "<button id='disarmBtn' class='btn' onclick=\"window.location.href='/control?action=disarm'\">撤防</button>";
  }
  html += "</div>";
  
  // 系统信息
  html += "<div class='info'>";
  html += "<p>触摸传感器值: " + String(touchValue) + "</p>";
  html += "<p>系统状态: " + stateText + "</p>";
  html += "<p>LED状态: " + String(digitalRead(LED_PIN) ? "亮" : "灭") + "</p>";
  html += "<p>IP地址: " + WiFi.localIP().toString() + "</p>";
  html += "</div>";
  
  // 添加自动刷新
  html += "<div style='margin-top: 20px; color: #666; font-size: 12px;'>";
  html += "页面每5秒自动刷新 | <a href='/' style='color: #2196F3;'>手动刷新</a>";
  html += "</div>";
  
  // 添加自动刷新meta标签
  html += "<meta http-equiv='refresh' content='5;url=/' />";
  
  html += "</div>";  // 结束container
  html += "</body>";
  html += "</html>";
  
  return html;
}

// 网页根路由
void handleRoot() {
  Serial.println("[HTTP] 收到主页请求");
  server.send(200, "text/html; charset=UTF-8", makePage());
}

// 控制系统状态
void handleControl() {
  Serial.println("\n[HTTP] 收到控制请求");
  Serial.print("请求URI: ");
  Serial.println(server.uri());
  Serial.print("参数数量: ");
  Serial.println(server.args());
  
  for (int i = 0; i < server.args(); i++) {
    Serial.print("参数 ");
    Serial.print(i);
    Serial.print(": ");
    Serial.print(server.argName(i));
    Serial.print(" = ");
    Serial.println(server.arg(i));
  }
  
  if (server.hasArg("action")) {
    String action = server.arg("action");
    Serial.print("控制命令: ");
    Serial.println(action);
    
    if (action == "arm") {
      if (systemState == DISARMED) {
        systemState = ARMED;
        touchDetected = false;
        isAlarmActive = false;
        digitalWrite(LED_PIN, LOW);
        Serial.println("[SYSTEM] 系统已布防");
        server.sendHeader("Location", "/");
        server.send(303);  // 303 See Other - 重定向到主页
      } else {
        Serial.println("[SYSTEM] 系统已布防或报警");
        server.send(200, "text/plain", "系统已布防或报警");
      }
    } 
    else if (action == "disarm") {
      systemState = DISARMED;
      isAlarmActive = false;
      touchDetected = false;
      digitalWrite(LED_PIN, LOW);
      Serial.println("[SYSTEM] 系统已撤防");
      server.sendHeader("Location", "/");
      server.send(303);  // 303 See Other - 重定向到主页
    } else {
      Serial.print("[ERROR] 未知命令: ");
      Serial.println(action);
      server.send(400, "text/plain", "无效命令: " + action);
    }
  } else {
    Serial.println("[ERROR] 缺少action参数");
    server.send(400, "text/plain", "缺少参数");
  }
  
  printSystemStatus();
}

// 获取触摸状态
void handleTouchStatus() {
  int touchValue = touchRead(TOUCH_PIN);
  Serial.print("[TOUCH] 触摸值: ");
  Serial.println(touchValue);
  server.send(200, "text/plain", String(touchValue));
}

// 处理触摸检测
void handleTouch() {
  int touchValue = touchRead(TOUCH_PIN);
  
  if (systemState == ARMED) {
    if (touchValue < TOUCH_THRESHOLD_VALUE) {  // 检测到触摸
      if (!touchDetected) {
        touchDetected = true;
        touchStartTime = millis();
        Serial.println("[TOUCH] 触摸检测开始");
      } else {
        // 检查触摸持续时间
        unsigned long touchDuration = millis() - touchStartTime;
        if (touchDuration > TOUCH_THRESHOLD) {
          if (!isAlarmActive) {
            triggerAlarm();
          }
        }
        // 显示触摸倒计时
        if (touchDuration % 1000 < 50) {  // 每秒打印一次
          Serial.print("[TOUCH] 触摸持续: ");
          Serial.print(touchDuration / 1000);
          Serial.print("秒 (阈值: ");
          Serial.print(TOUCH_THRESHOLD / 1000);
          Serial.println("秒)");
        }
      }
    } else {
      if (touchDetected) {
        touchDetected = false;
        Serial.println("[TOUCH] 触摸结束");
      }
    }
  }
}

// 触发报警
void triggerAlarm() {
  systemState = ALARM;
  isAlarmActive = true;
  touchDetected = false;
  Serial.println("[ALARM] 🚨 报警触发！");
  Serial.println("[ALARM] LED开始高频闪烁");
  Serial.println("[ALARM] 请登录网页点击撤防解除报警");
  printSystemStatus();
}

// 控制报警LED
void handleAlarmLED() {
  if (isAlarmActive) {
    unsigned long currentTime = millis();
    if (currentTime - lastBlinkTime >= BLINK_INTERVAL) {
      digitalWrite(LED_PIN, !digitalRead(LED_PIN));  // 切换LED状态
      lastBlinkTime = currentTime;
    }
  } else if (systemState == DISARMED) {
    // 撤防状态下确保LED熄灭
    digitalWrite(LED_PIN, LOW);
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);  // 等待串口初始化
  
  Serial.println("\n🔧 ESP32安防系统");
  Serial.println("================================");
  Serial.println("WiFi: " + String(ssid));
  Serial.println("密码: " + String(password));
  Serial.println("================================");
  
  // 初始化引脚
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_PIN, LOW);
  delay(1000);
  digitalWrite(LED_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_PIN, LOW);
  Serial.println("✅ LED引脚初始化完成");
  
  // 测试触摸传感器
  int touchValue = touchRead(TOUCH_PIN);
  Serial.print("📊 触摸传感器初始值: ");
  Serial.println(touchValue);
  Serial.println("正常范围: 0-100，值越小表示触摸越强");
  Serial.print("触摸阈值: ");
  Serial.println(TOUCH_THRESHOLD_VALUE);
  
  // 连接WiFi
  WiFi.begin(ssid, password);
  Serial.print("📡 正在连接WiFi: ");
  Serial.println(ssid);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
    if (attempts % 20 == 0) Serial.println();
  }
  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("✅ WiFi连接成功!");
    Serial.print("🌐 IP地址: ");
    Serial.println(WiFi.localIP());
    Serial.print("📶 信号强度: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  } else {
    Serial.println("❌ WiFi连接失败!");
    Serial.println("可能原因:");
    Serial.println("1. WiFi名称或密码错误");
    Serial.println("2. ESP32不在WiFi信号范围内");
    Serial.println("3. 路由器限制连接");
    Serial.println("正在尝试继续启动...");
  }
  
  // 设置服务器路由
  Serial.println("\n🌍 设置Web服务器路由...");
  server.on("/", handleRoot);
  server.on("/control", handleControl);
  server.on("/touchStatus", handleTouchStatus);
  
  // 测试路由
  server.on("/test", []() {
    Serial.println("[TEST] 测试路由被调用");
    server.send(200, "text/plain", "Test OK - " + String(millis()));
  });
  
  server.onNotFound([]() {
    Serial.print("[404] 页面不存在: ");
    Serial.println(server.uri());
    server.send(404, "text/plain", "404: Not Found - " + server.uri());
  });
  
  server.begin();
  Serial.println("✅ HTTP服务器已启动");
  Serial.println("\n📱 访问地址: http://" + WiFi.localIP().toString());
  Serial.println("📱 测试地址: http://" + WiFi.localIP().toString() + "/test");
  Serial.println("================================");
  Serial.println("🔧 系统已就绪");
  Serial.println("初始状态: 撤防");
  Serial.println("================================");
}

void loop() {
  server.handleClient();
  
  // 处理触摸检测
  handleTouch();
  
  // 处理报警LED
  handleAlarmLED();
  
  delay(10);
}