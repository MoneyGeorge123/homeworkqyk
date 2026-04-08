// 如果LEDC库不可用，使用dacWrite（仅适用于GPIO25, GPIO26）
#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "Q";
const char* password = "scc170226";
const int LED_PIN = 25; // 使用GPIO25（DAC1）或GPIO26（DAC2）
int currentBrightness = 128; // 当前亮度值，范围0-255

WebServer server(80);

String makePage() {
  String state = (currentBrightness > 0) ? "ON" : "OFF";
  
  String html = "<!DOCTYPE html>";
  html += "<html lang='zh-CN'>";
  html += "<head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>ESP32 LED控制</title>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; text-align: center; padding: 20px; }";
  html += ".container { max-width: 500px; margin: 0 auto; }";
  html += "h1 { color: #333; }";
  html += ".slider-value { font-size: 20px; margin: 10px 0; }";
  html += "input[type='range'] { width: 100%; }";
  html += "</style>";
  html += "</head>";
  html += "<body>";
  html += "<div class='container'>";
  html += "<h1>ESP32 LED亮度控制</h1>";
  html += "<p>当前亮度: <span id='currentPWM'>" + String(currentBrightness) + "</span>/255</p>";
  html += "<div class='slider-value'>亮度: <span id='sliderValue'>" + String(currentBrightness) + "</span></div>";
  html += "<input type='range' id='brightnessSlider' min='0' max='255' value='" + String(currentBrightness) + "' oninput='updateSlider(this.value)'>";
  html += "<div>";
  html += "<a href='/on'><button>开</button></a>";
  html += "<a href='/off'><button>关</button></a>";
  html += "</div>";
  html += "</div>";
  
  html += "<script>";
  html += "function updateSlider(value) {";
  html += "  document.getElementById('sliderValue').textContent = value;";
  html += "  var xhr = new XMLHttpRequest();";
  html += "  xhr.open('GET', '/setPWM?value=' + value, true);";
  html += "  xhr.onreadystatechange = function() {";
  html += "    if (xhr.readyState == 4 && xhr.status == 200) {";
  html += "      document.getElementById('currentPWM').textContent = value;";
  html += "    }";
  html += "  };";
  html += "  xhr.send();";
  html += "}";
  html += "</script>";
  html += "</body>";
  html += "</html>";
  
  return html;
}

void handleRoot() {
  server.send(200, "text/html; charset=UTF-8", makePage());
}

void handleOn() {
  currentBrightness = 255;
  dacWrite(LED_PIN, currentBrightness);
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleOff() {
  currentBrightness = 0;
  dacWrite(LED_PIN, currentBrightness);
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleSetPWM() {
  if (server.hasArg("value")) {
    int pwmValue = server.arg("value").toInt();
    if (pwmValue >= 0 && pwmValue <= 255) {
      currentBrightness = pwmValue;
      dacWrite(LED_PIN, pwmValue);
      server.send(200, "text/plain", "OK");
      Serial.print("设置亮度: ");
      Serial.println(pwmValue);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  dacWrite(LED_PIN, currentBrightness);
  
  WiFi.begin(ssid, password);
  Serial.print("连接WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n连接成功");
  Serial.print("访问地址: http://");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/on", handleOn);
  server.on("/off", handleOff);
  server.on("/setPWM", handleSetPWM);
  server.begin();
}

void loop() {
  server.handleClient();
}