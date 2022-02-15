#include <CoolE.h>

CoolE iot("{developkey}");    //!!!设备DevelopKey
String recv;

void handleGet();             // 处理GET指令更新通信字段
void handleCommand();         // 处理命令字段

void setup()
{
  // 功能部分
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  // IoT部分
  iot.setDebug();
  iot.start("{ssid}", "{pswd}");
}

void loop()
{
  // IoT部分
  iot.loop();
  recv = iot.getRevContent();
  handleGet();
  handleCommand();
  // IoT处理
  iot.next();
}

void handleGet(){
  // 接收GET指令并更新通信字段数据
  String get = iot.decode(recv,GET);
  if (get == "__all__" || get == "ledsta") {
    iot.publish();
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