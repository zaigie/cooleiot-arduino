# cooleiot-arduino
 CoolE IoT Platform Library. Works with ESP8266, ESP32.

> 项目已迁移到 [https://github.com/cooleiot/arduino-sdk](https://github.com/cooleiot/arduino-sdk)

## 实例

实例化IoT连接，须填入设备的DevelopKey

```cpp
CoolE iot("{developkey}");
```

### iot.setDebug

```cpp
/* 设置debug状态 */
void setDebug(bool is_debug = true);
```

使用：

```cpp
iot.setDebug();
```

### iot.getRevContent

```cpp
/* 获取MQTT接收到的消息 */
static String getRevContent();
```

使用：

```cpp
String recv = iot.getRevContent();
```

### iot.wifistatus

```cpp
/* 获取WIFI状态 */
bool wifistatus();
```

使用：

> 在`iot.init()`或`iot.start()`之后才能调用此方法获取WIFI状态

```cpp
bool sta = iot.wifistatus();
```

### iot.setClearWiFi

```cpp
/* 设置是否清空所有保存的WIFI信息 */
void setClearWiFi();
```

使用：

> 该方法是一次性的，务必只能放在`setup()`中，主要用于测试，正式使用请注释或删除该行

```cpp
iot.setClearWiFi();
```

### iot.getDeviceId

```cpp
/* 获取设备ID */
String getDeviceId();
```

使用：

> 在`iot.init()`或`iot.start()`之后才能调用此方法获取设备ID

```cpp
String device_id = iot.getDeviceId();
```


## 连接

### iot.init

```cpp
/* 配置设备初始化并连接到网络(推荐) */
void init();
/* 自定义WIFI信息连接 */
void init(const char *ssid,const char *pswd);
```

配置设备初始化并连接到网络，该方法有2个重载<br />
 - 不携带任何参数的情况下，将会为设备自动创建一个WiFiManager，可以通过Web配网设置设备连接信息
 - 而携带了WIFI信息参数时，将会连接到该WIFI，且无法进行Web配网，修改连接信息时需要重新烧录

使用：

```cpp
iot.init();     //*推荐
iot.init("Hello", "cooleiot");
```

### iot.connect

```cpp
/* 连接MQTT */
void connect();
```

使用：

> 需要在`iot.init()`之后调用此方法

```cpp
iot.connect();
```

### iot.start

```cpp
/* 开始IoT */
void start(); // ==> iot.init();iot.connect();
```

> 该方法其实就是将`iot.init()`和`iot.connect()`融合，方便编写

使用：

```cpp
iot.start();
```

### iot.loop

```cpp
/* MQTT接收循环 */
void loop();
```

> 循环接收MQTT消息，如需要在设备运行过程中接收MQTTBroker消息，**务必将该方法放入`loop()`**

使用：

```cpp
iot.loop();
```

### iot.next

```cpp
/* IoT 结尾部分 */
static void next();
```

> 该函数放在`loop()`最底部，用于处理消息重复接收逻辑以及断网重连，为必须，若不加该函数，将会导致不可预见的问题

使用：

```cpp
iot.next();
```

### iot.stop

```cpp
/* 关闭MQTT连接 */
void stop();
```

使用：

```cpp
iot.stop();
```

## 字段

### iot.decode

```cpp
/* 字段解析 */
String decode(String payload, enum DecodeType type);
enum DecodeType{
  COMMAND = 1,          // 命令字段
  GET,                  // GET指令
  CONTENT_NAME,         // 内容字段名称
  CONTENT_DATA,         // 内容字段内容
  SYSTEM                // 系统字段
};
```

> 该方法用于解析在程序运行过程中接收到的三类字段

使用：

```cpp
String recv = iot.getRevContent();          //获取MQTT接收到的消息
String command = iot.decode(recv,COMMAND);  //解析命令字段
if (command == "open") {
  Serial.println("接收到open命令字段");
}
```

---

> 开发中，您需要先将字段数据更新至iot实例中的一个成员变量，然后再进行消息发布<br />
也就是说，更新数据和发布消息是两步！

### iot.updateReport

```cpp
/* 为通信字段添加一个字段内容 */
void updateReport(String field, String data);
void updateReport(String field, const char* data);
void updateReport(String field, int data);
void updateReport(String field, float data);
void updateReport(String field, byte data);
```

> 该方法用于将数据更新至指定通信字段，相同字段名，后更新的数据会覆盖之前的数据

使用：

```cpp
iot.updateReport("temp",23.6);
iot.updateReport("ledsta","开");
```

### iot.getReport

```cpp
/* 获取通信字段内容 */
String getReport(String field);
```

使用：

```cpp
String ledsta = iot.getReport("ledsta");
```

### iot.clearReport

```cpp
/* 清除当前成员变量存储的所有通信字段信息 */
void clearReport();
```

使用：

```cpp
iot.clearReport();
```

### iot.updateContent

```cpp
/* 为通信字段添加一个字段内容 */
void updateContent(String field, String data);
void updateContent(String field, const char* data);
```

> 该方法用于将数据更新至指定内容字段，相同字段名，后更新的数据会覆盖之前的数据

使用：

```cpp
// 接收内容字段数据
String content_name = iot.decode(recv,CONTENT_NAME);
String content_data = iot.decode(recv,CONTENT_DATA);
if (content_name == "line1") {
  iot.updateContent("line1",content_data);
} else if (content_name == "line2") {
  iot.updateContent("line2",content_data);
}
```

### iot.getContent

```cpp
/* 获取通信字段内容 */
String getContent(String field);
```

使用：

```cpp
Serial.println(iot.getContent("line1"));
```

### iot.clearContent

```cpp
/* 清除当前成员变量存储的所有内容字段信息 */
void clearContent();
```

使用：

```cpp
iot.clearContent();
```

## 发布

> 为了避免代码编写不规范等导致的Broker阻塞<br />
无论代码编写方法如何，publish方法**每秒只会发送一条消息**

### iot.publish

```cpp
/* 发布所有update过的字段消息 */
void publish();
/* 发布指定update过的字段消息 */
void publish(String field);
/* 手动发布指定字段的消息内容 */
void publish(String field, String payload);
void publish(String field, const char* payload);
void publish(String field, int payload);
void publish(String field, float payload);
void publish(String field, byte payload);
/* 手动发布键值对消息 */
void publish(String key, String value, enum MessageType type);
enum MessageType{
  NORMAL = 0,
  STRING
};
```

#### 发布消息到MQTT服务器

 - 如果您想发送之前使用`iot.updateReport()`存储的**所有**通信字段内容

```cpp
iot.publish();
```

 - 如果您想发送之前使用`iot.updateReport()`存储的**指定**通信字段内容

```cpp
iot.publish("temp");
```

 - 如果您想发送指定通信字段内容的自定义内容，而不在其前面进行`updateReport()`。<br />
也就是说您想单独发送此次消息，而不将这次发送的数据更新到通信字段成员变量中

```cpp
iot.publish("temp",23.8);
iot.publish("ledsta","开");
```

 - 如果您想发送自定义key-value键值对
   - MessageType为`STRING`时，认为您想要发送字符串数据，则发送的消息将会打上双引号
   - MessageType为`NORMAL`时，不会为发送的消息打上双引号，而此时您的数据实际为字符串内容，则属于不规范JSON格式

```cpp
iot.publish("hello", "world", STRING);      //{"hello":"world"}
iot.publish("ram", 400, NORMAL);            //{"ram":400}
iot.publish("ram", 400, STRING);            //{"ram":"400"}
```
## 云配置

> 为了节省服务器资源，云配置每个DevelopKey每天仅能调用200次

> 在ArduinoSDK中并不支持JSON类型的云配置，同样也没有这方面的需求，故在开发时请注意

### iot.getStrConfig

```cpp
/* 获取字符串类型的云配置 */
String getStrConfig(String field);
```

使用：

```cpp
String citycode = iot.getStrConfig("citycode");
```

### iot.getNumConfig

```cpp
/* 获取数字类型的云配置 */
float getNumConfig(String field);
```

> 数字类型云配置在ArduinoSDK中默认返回浮点型，但并不影响将其直接强制转化为整型

使用：

```cpp
float speed = iot.getNumConfig("speed");
int speed = iot.getNumConfig("speed");
```
