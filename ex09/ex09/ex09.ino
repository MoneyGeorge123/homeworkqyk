#include <WiFi.h>
#include <WebServer.h>

// WiFi配置
const char* ssid = "Q";
const char* password = "scc170226";

// 引脚定义
const int TOUCH_PIN = 4;         // 触摸传感器引脚（GPIO4，TOUCH0）
const int LED_PIN = 2;           // LED指示灯，用于状态指示

// Web服务器
WebServer server(80);

// 传感器数据统计
struct SensorData {
  int currentValue = 0;          // 当前值
  int minValue = 100;            // 最小值
  int maxValue = 0;              // 最大值
  float averageValue = 0;        // 平均值
  unsigned long sampleCount = 0; // 采样计数
  unsigned long lastUpdate = 0;  // 最后更新时间
} sensorData;

// 触摸阈值
const int TOUCH_THRESHOLD = 20;  // 触摸阈值

// 生成JSON格式的传感器数据
String generateSensorJSON() {
  String json = "{";
  json += "\"value\":" + String(sensorData.currentValue) + ",";
  json += "\"minValue\":" + String(sensorData.minValue) + ",";
  json += "\"maxValue\":" + String(sensorData.maxValue) + ",";
  json += "\"averageValue\":" + String(sensorData.averageValue, 1) + ",";
  json += "\"sampleCount\":" + String(sensorData.sampleCount) + ",";
  json += "\"timestamp\":" + String(millis()) + ",";
  json += "\"status\":\"" + String(sensorData.currentValue < TOUCH_THRESHOLD ? "touching" : "idle") + "\",";
  json += "\"threshold\":" + String(TOUCH_THRESHOLD);
  json += "}";
  return json;
}

// 生成主页面
String generateDashboard() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 传感器实时监控</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            padding: 20px;
        }
        
        .dashboard {
            width: 100%;
            max-width: 600px;
            background: rgba(255, 255, 255, 0.95);
            border-radius: 20px;
            box-shadow: 0 20px 40px rgba(0, 0, 0, 0.1);
            overflow: hidden;
        }
        
        .header {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 25px 30px;
            text-align: center;
        }
        
        .header h1 {
            font-size: 28px;
            margin-bottom: 5px;
        }
        
        .header p {
            opacity: 0.9;
            font-size: 16px;
        }
        
        .connection-status {
            display: inline-flex;
            align-items: center;
            background: rgba(255, 255, 255, 0.2);
            padding: 8px 15px;
            border-radius: 20px;
            margin-top: 10px;
            font-size: 14px;
        }
        
        .status-dot {
            width: 8px;
            height: 8px;
            background: #4CAF50;
            border-radius: 50%;
            margin-right: 8px;
            animation: pulse 2s infinite;
        }
        
        .main-content {
            padding: 30px;
        }
        
        .sensor-display {
            text-align: center;
            margin-bottom: 30px;
            padding: 20px;
            background: #f8f9fa;
            border-radius: 15px;
        }
        
        .sensor-value {
            font-size: 100px;
            font-weight: 700;
            color: #333;
            margin: 20px 0;
            font-family: 'Arial', monospace;
            text-shadow: 0 5px 15px rgba(0,0,0,0.1);
        }
        
        .value-label {
            font-size: 24px;
            color: #666;
            margin-bottom: 10px;
        }
        
        .sensor-status {
            display: inline-block;
            padding: 8px 20px;
            border-radius: 20px;
            font-weight: 600;
            font-size: 16px;
            margin-top: 10px;
        }
        
        .status-idle {
            background: #e8f5e8;
            color: #2e7d32;
        }
        
        .status-touching {
            background: #ffebee;
            color: #c62828;
            animation: alert 1s infinite alternate;
        }
        
        .stats-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 20px;
            margin-top: 30px;
        }
        
        .stat-card {
            background: #f8f9fa;
            padding: 20px;
            border-radius: 15px;
            text-align: center;
            transition: transform 0.3s;
        }
        
        .stat-card:hover {
            transform: translateY(-5px);
            box-shadow: 0 10px 20px rgba(0,0,0,0.1);
        }
        
        .stat-value {
            font-size: 36px;
            font-weight: 700;
            color: #667eea;
            margin: 10px 0;
        }
        
        .stat-label {
            color: #666;
            font-size: 16px;
        }
        
        .instructions {
            background: #e3f2fd;
            padding: 20px;
            border-radius: 15px;
            margin-top: 30px;
            border-left: 5px solid #2196F3;
        }
        
        .instructions h3 {
            color: #1565c0;
            margin-bottom: 10px;
        }
        
        .instructions ul {
            padding-left: 20px;
        }
        
        .instructions li {
            margin: 5px 0;
            color: #555;
        }
        
        .update-info {
            text-align: center;
            margin-top: 20px;
            color: #666;
            font-size: 14px;
        }
        
        .control-buttons {
            display: flex;
            justify-content: center;
            gap: 15px;
            margin-top: 20px;
        }
        
        .control-btn {
            padding: 10px 20px;
            border: none;
            border-radius: 8px;
            background: #667eea;
            color: white;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.3s;
        }
        
        .control-btn:hover {
            background: #764ba2;
            transform: translateY(-2px);
        }
        
        .control-btn.reset {
            background: #f44336;
        }
        
        .control-btn.reset:hover {
            background: #d32f2f;
        }
        
        .touch-indicator {
            display: inline-block;
            width: 12px;
            height: 12px;
            border-radius: 50%;
            margin-right: 8px;
        }
        
        .touch-active {
            background: #4CAF50;
            animation: pulse 1s infinite;
        }
        
        .touch-inactive {
            background: #ddd;
        }
        
        @keyframes pulse {
            0% { opacity: 1; }
            50% { opacity: 0.5; }
            100% { opacity: 1; }
        }
        
        @keyframes alert {
            from { background: #ffebee; }
            to { background: #ffcdd2; }
        }
        
        @keyframes valueChange {
            0% { transform: scale(1); }
            50% { transform: scale(1.05); }
            100% { transform: scale(1); }
        }
        
        .value-update {
            animation: valueChange 0.3s;
        }
    </style>
</head>
<body>
    <div class="dashboard">
        <div class="header">
            <h1>🔧 ESP32 传感器实时监控</h1>
            <p>触摸传感器数据实时显示</p>
            <div class="connection-status">
                <div class="status-dot"></div>
                <span>IP: )rawliteral";
  html += WiFi.localIP().toString();
  html += R"rawliteral( | 已连接</span>
            </div>
        </div>
        
        <div class="main-content">
            <div class="sensor-display">
                <div class="value-label">当前触摸传感器值</div>
                <div id="sensorValue" class="sensor-value">0</div>
                <div id="sensorStatus" class="sensor-status status-idle">传感器空闲</div>
            </div>
            
            <div class="stats-grid">
                <div class="stat-card">
                    <div class="stat-label">最小值</div>
                    <div id="minValue" class="stat-value">0</div>
                </div>
                <div class="stat-card">
                    <div class="stat-label">最大值</div>
                    <div id="maxValue" class="stat-value">0</div>
                </div>
                <div class="stat-card">
                    <div class="stat-label">平均值</div>
                    <div id="avgValue" class="stat-value">0.0</div>
                </div>
                <div class="stat-card">
                    <div class="stat-label">采样次数</div>
                    <div id="sampleCount" class="stat-value">0</div>
                </div>
            </div>
            
            <div class="instructions">
                <h3>📋 使用说明</h3>
                <ul>
                    <li>靠近或触摸ESP32的GPIO4引脚，观察数值变化</li>
                    <li>数值范围: 0-100，数值越小表示触摸强度越大</li>
                    <li>触摸阈值: 20，低于此值表示检测到触摸</li>
                    <li>数据每200毫秒自动更新一次</li>
                    <li>LED指示灯会根据触摸状态变化</li>
                </ul>
            </div>
            
            <div class="control-buttons">
                <button class="control-btn" onclick="resetStatistics()">重置统计</button>
                <button class="control-btn reset" onclick="location.reload()">刷新页面</button>
            </div>
            
            <div class="update-info">
                最后更新: <span id="lastUpdate">--:--:--</span> | 采样间隔: 200ms
            </div>
        </div>
    </div>
    
    <script>
        // 更新传感器显示
        function updateSensorDisplay(data) {
            const sensorValueElement = document.getElementById('sensorValue');
            const sensorStatusElement = document.getElementById('sensorStatus');
            
            // 添加动画效果
            sensorValueElement.classList.add('value-update');
            setTimeout(() => {
                sensorValueElement.classList.remove('value-update');
            }, 300);
            
            // 解析JSON数据
            let sensorData;
            try {
                sensorData = JSON.parse(data);
            } catch (e) {
                console.error('JSON解析错误:', e);
                return;
            }
            
            // 更新数值
            sensorValueElement.textContent = sensorData.value;
            
            // 更新状态
            if (sensorData.value < sensorData.threshold) {
                sensorStatusElement.textContent = '🖐️ 检测到触摸';
                sensorStatusElement.className = 'sensor-status status-touching';
            } else {
                sensorStatusElement.textContent = '🆓 传感器空闲';
                sensorStatusElement.className = 'sensor-status status-idle';
            }
            
            // 更新统计数据
            document.getElementById('minValue').textContent = sensorData.minValue;
            document.getElementById('maxValue').textContent = sensorData.maxValue;
            document.getElementById('avgValue').textContent = sensorData.averageValue;
            document.getElementById('sampleCount').textContent = sensorData.sampleCount.toLocaleString();
            document.getElementById('lastUpdate').textContent = new Date().toLocaleTimeString();
        }
        
        // 获取传感器数据
        async function fetchSensorData() {
            try {
                const response = await fetch('/sensorData');
                if (response.ok) {
                    const data = await response.text();
                    updateSensorDisplay(data);
                } else {
                    console.error('获取数据失败:', response.status);
                }
            } catch (error) {
                console.error('获取数据时出错:', error);
            }
        }
        
        // 重置统计数据
        async function resetStatistics() {
            if (confirm('确定要重置统计数据吗？')) {
                try {
                    const response = await fetch('/resetStats');
                    if (response.ok) {
                        alert('统计数据已重置');
                        fetchSensorData(); // 重新获取数据
                    }
                } catch (error) {
                    console.error('重置失败:', error);
                }
            }
        }
        
        // 页面加载完成
        document.addEventListener('DOMContentLoaded', function() {
            console.log('传感器监控仪表盘已加载');
            console.log('服务器IP:', document.querySelector('.connection-status span').textContent);
            
            // 立即获取一次数据
            fetchSensorData();
            
            // 设置定时器，每200ms获取一次数据
            setInterval(fetchSensorData, 200);
            
            // 添加键盘快捷键
            document.addEventListener('keydown', function(e) {
                if (e.key === 'r' || e.key === 'R') {
                    if (e.ctrlKey) {
                        resetStatistics();
                    } else {
                        location.reload();
                    }
                }
                if (e.key === 'Escape') {
                    alert('ESC键按下 - 系统运行中\n当前时间: ' + new Date().toLocaleTimeString());
                }
            });
        });
    </script>
</body>
</html>
)rawliteral";
  return html;
}

// 获取传感器数据的API端点
void handleSensorData() {
  // 获取当前触摸值
  int touchValue = touchRead(TOUCH_PIN);
  
  // 更新统计数据
  sensorData.currentValue = touchValue;
  sensorData.sampleCount++;
  sensorData.lastUpdate = millis();
  
  // 更新最小值和最大值
  if (touchValue < sensorData.minValue) {
    sensorData.minValue = touchValue;
  }
  if (touchValue > sensorData.maxValue) {
    sensorData.maxValue = touchValue;
  }
  
  // 计算移动平均值
  if (sensorData.sampleCount == 1) {
    sensorData.averageValue = touchValue;
  } else {
    // 指数移动平均
    float alpha = 0.1;  // 平滑因子
    sensorData.averageValue = alpha * touchValue + (1 - alpha) * sensorData.averageValue;
  }
  
  // 发送JSON响应
  server.send(200, "application/json", generateSensorJSON());
  
  // 控制LED指示状态
  static bool ledState = false;
  if (touchValue < TOUCH_THRESHOLD) {
    // 触摸中时快速闪烁
    if (millis() % 200 < 100) {
      digitalWrite(LED_PIN, HIGH);
    } else {
      digitalWrite(LED_PIN, LOW);
    }
  } else {
    // 空闲时慢闪
    if (millis() % 1000 < 100) {
      digitalWrite(LED_PIN, HIGH);
    } else {
      digitalWrite(LED_PIN, LOW);
    }
  }
}

// 重置统计数据的API端点
void handleResetStats() {
  sensorData.minValue = 100;
  sensorData.maxValue = 0;
  sensorData.averageValue = 0;
  sensorData.sampleCount = 0;
  server.send(200, "text/plain", "Statistics reset");
  Serial.println("📊 统计数据已重置");
}

// 主页面
void handleRoot() {
  server.send(200, "text/html", generateDashboard());
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n📊 ESP32 传感器实时监控系统");
  Serial.println("==================================");
  
  // 初始化引脚
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // 连接WiFi
  WiFi.begin(ssid, password);
  Serial.print("📡 正在连接WiFi: ");
  Serial.println(ssid);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("✅ WiFi连接成功!");
    Serial.print("🌐 IP地址: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("❌ WiFi连接失败!");
  }
  
  // 测试传感器
  Serial.println("\n🔧 传感器测试...");
  int initialValue = touchRead(TOUCH_PIN);
  Serial.print("初始触摸值: ");
  Serial.println(initialValue);
  Serial.print("触摸阈值: ");
  Serial.println(TOUCH_THRESHOLD);
  Serial.println("说明: 数值越小表示触摸强度越大");
  Serial.println("==================================");
  
  // 设置服务器路由
  server.on("/", handleRoot);
  server.on("/sensorData", handleSensorData);
  server.on("/resetStats", handleResetStats);
  
  server.onNotFound([]() {
    server.send(404, "text/plain", "404: Not Found");
  });
  
  server.begin();
  Serial.println("✅ HTTP服务器已启动");
  Serial.println("\n📱 访问地址: http://" + WiFi.localIP().toString());
  Serial.println("📈 打开网页查看实时传感器数据");
  Serial.println("📊 数据更新间隔: 200ms");
  Serial.println("==================================");
  
  // 初始LED闪烁指示
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    delay(200);
  }
}

void loop() {
  server.handleClient();
  
  // 每5秒显示一次传感器值（用于调试）
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 5000) {
    Serial.print("传感器值: ");
    Serial.print(sensorData.currentValue);
    Serial.print(" | 状态: ");
    Serial.println(sensorData.currentValue < TOUCH_THRESHOLD ? "触摸中" : "空闲");
    Serial.print("统计: 最小值=");
    Serial.print(sensorData.minValue);
    Serial.print(" 最大值=");
    Serial.print(sensorData.maxValue);
    Serial.print(" 平均值=");
    Serial.println(sensorData.averageValue, 1);
    Serial.println("----------------------------------");
    lastPrint = millis();
  }
  
  delay(10);
}