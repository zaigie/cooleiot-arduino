#include <CoolE.h>
#include <SimpleDHT.h>
#include <U8g2lib.h>
#include <Wire.h>

CoolE iot("{developkey}");    //!!!设备DevelopKey
String recv;
SimpleDHT11 dht11(14);
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0,U8X8_PIN_NONE);

void DisplayDisconnect();     // WIFI连接成功前的OLED显示
void updateDHT();             // 更新温湿度数据
void OLEDDisplay();           // OLED展示
void handleGet();             // 处理GET指令更新通信字段
void handleCommand();         // 处理命令字段
void handleContent();         // 处理内容字段

void setup()
{
  // 功能部分
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  u8g2.begin();
  updateDHT();
  DisplayDisconnect();
  // IoT部分
  iot.setDebug();
  iot.start("{ssid}", "{pswd}");
}

void loop()
{
  // 显示部分
  OLEDDisplay();
  // IoT部分
  iot.loop();
  recv = iot.getRevContent();
  handleGet();
  handleCommand();
  handleContent();
  // IoT处理
  iot.next();
}

void handleGet(){
  // 接收GET指令并更新通信字段数据
  String get = iot.decode(recv,GET);
  if (get == "__all__") {
    updateDHT();
    iot.updateReport("ledsta",digitalRead(LED_BUILTIN) == HIGH ? "关" : "开");
    iot.publish();
  } else if (get == "temp") {
    updateDHT();
    iot.publish("temp");
  } else if (get == "humi") {
    updateDHT();
    iot.publish("humi");
  } else if (get == "ledsta") {
    iot.publish("ledsta");
  }
}
void handleCommand(){
  // 接收命令字段数据
  String command = iot.decode(recv,COMMAND);
  if (command == "open") {
    digitalWrite(LED_BUILTIN, LOW);
    iot.updateReport("ledsta", "开");
    iot.publish("ledsta");
  } else if (command == "close") {
    digitalWrite(LED_BUILTIN, HIGH);
    iot.updateReport("ledsta", "关");
    iot.publish("ledsta");
  }
}
void handleContent(){
  // 接收内容字段数据
  String content_name = iot.decode(recv,CONTENT_NAME);
  String content_data = iot.decode(recv,CONTENT_DATA);
  if (content_name == "line1") {
    iot.updateContent("line1",content_data);
  } else if (content_name == "line2") {
    iot.updateContent("line2",content_data);
  }
}

void DisplayDisconnect(){
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_unifont_t_chinese2);
  u8g2.drawStr(0, 15,"AP:device_id");
  u8g2.drawStr(0, 30,"Pswd:cooleiot");
  u8g2.drawStr(0, 48,"To APP Set WiFi");
  u8g2.drawStr(0, 62,"or 192.168.4.1");
  u8g2.sendBuffer();
}

void updateDHT(){
  byte temperature = 0;
  byte humidity = 0;
  int err = SimpleDHTErrSuccess;
  if ((err = dht11.read(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
    // Serial.print("Read DHT11 failed, err="); Serial.println(err);
    delay(1000);
    return;
  }
  iot.updateReport("temp",temperature);
  iot.updateReport("humi",humidity);
}

void OLEDDisplay(){
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_unifont_t_chinese2);
  u8g2.drawStr(0,10,"CoolE IoT Demo");
  u8g2.setCursor(0, 28);
  u8g2.print(iot.getContent("line1"));
  u8g2.setCursor(0, 43);
  u8g2.print(iot.getContent("line2"));
  u8g2.setCursor(0, 62);
  u8g2.print("Tem:"+iot.getReport("temp")+"C");
  u8g2.print("  Hum:"+iot.getReport("humi")+"%");
  u8g2.sendBuffer();
}