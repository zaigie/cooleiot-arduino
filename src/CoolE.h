// -----
// CoolE.h - 
// -----

#ifndef CoolE_h
#define CoolE_h
#include "Arduino.h"

enum MessageType{
  NORMAL = 0,
  STRING
};
enum DecodeType{
  COMMAND = 1,
  GET,
  CONTENT_NAME,
  CONTENT_DATA,
  SYSTEM
};

void callback(char* topic, byte* payload, unsigned int length);
#if !defined(ESP8266)
static String byteToHexStr(uint8_t *buf, uint8_t length, String strSeperator); // byteToHexString
String get32ChipID();
#endif
class CoolE
{
public:
  /* 构造函数（USEFUL） */
  CoolE();

  /* 构造函数 */
  CoolE(const char* developkey);

  #ifndef WITHOUT_WEB
  /* 配置设备初始化并连接到网络(推荐) */
  void init();
  #endif

  /* 自定义WIFI信息连接 */
  void init(const char *ssid,const char *pswd);

  /* 连接MQTT */
  void connect();

  #ifndef WITHOUT_WEB
  /* 开始IoT */
  void start();
  #endif

  /* 开始IoT(自定义WIFI) */
  void start(const char *ssid,const char *pswd);

  /* MQTT接收循环 */
  void loop();

  /* 关闭MQTT连接 */
  void stop();

  /* 获取通信字段内容 */
  String getReport(String field);

  /* 为通信字段添加一个字段内容 */
  void updateReport(String field, String data);
  void updateReport(String field, const char* data);
  void updateReport(String field, int data);
  void updateReport(String field, float data);
  void updateReport(String field, byte data);

  /* 获取通信字段内容 */
  String getContent(String field);

  /* 为通信字段添加一个字段内容 */
  void updateContent(String field, String data);
  void updateContent(String field, const char* data);

  /* 清除report信息 */
  void clearReport();

  /* 清除content信息 */
  void clearContent();

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

  /* 字段解析 */
  String decode(String payload, enum DecodeType type);

  /* 获取字符串类型的云配置 */
  String getStrConfig(String field);
  /* 获取数字类型的云配置 */
  float getNumConfig(String field);

  /* 打印MQTT接收的数据，调试用，与iot.getRevContent()不能同时使用，需打印请使用Serial.println(iot.getRevContent()) */
  static void printRev();

  /* 设置debug状态 */
  void setDebug(bool result);

  /* DEBUG */
  template <typename Generic>
  void debug(Generic text);

  /* MQTT Callback内容处理 */
  static String rev_content;
  static String rev_content_temp;
  static void setRevContent(String content);
  static String getRevContent();

  /* WIFI状态处理 */
  bool wifistatus();
  void setClearWiFi();

  String getDeviceId();

private:
  const char* _developkey;           // 设备Developkey
  const char* _device_id;            // 设备ID
  const char* _username;             // 用户名
  String _topic;                     // 设备Topic
  String _client_id;                 // 设备Client-ID
  String _content_content;           // 所有内容字段信息
  String _report_content;            // 所有通信字段信息
  int field_num;                     // 存储通信字段数量
  String _publish_content;           // 当前消息发布内容   
  bool _is_debug = false;            // 是否开启DEBUG
  bool _wifi_connected = false;      // WIFI连接状态
  bool _clear_wifi = false;          // 是否设置清除保存的WIFI信息
  uint64_t last_publish_time;        // 上次发布时间
};

#endif
