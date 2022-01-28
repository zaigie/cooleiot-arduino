/**
 * @file CoolE.cpp
 *
 * @brief CoolE IoT platfrom Library
 *
 * @author Zaigie, https://www.zaigie.com
 * @Copyright Copyright (c) by CoolE IoT, https://cooleiot.tech
 *
 * More information on: https://doc.cooleiot.tech
 *
 * Changelog: see CoolE.h
 */

#include "CoolE.h"

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#if defined(ESP32)
#include <WiFi.h>
#endif
#ifndef WITHOUT_WEB
#include "modules/ESPAsyncWiFiManager/ESPAsyncWiFiManager.h"
#endif
#include "modules/ArduinoHttpClient/ArduinoHttpClient.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* api_server = "api.cooleiot.tech";
int api_port = 80;
const char *server = "broker.cooleiot.tech";

WiFiClient wificlient;
HttpClient httpclient = HttpClient(wificlient, api_server, api_port);
PubSubClient mqttclient(server, 1883, wificlient);
DynamicJsonDocument doc(4096);
DynamicJsonDocument report(4096);
DynamicJsonDocument content(4096);

// MQTT Callback
void callback(char *topic, byte *payload, unsigned int length)
{
  byte *end = payload + length;
  String content;
  for (byte *p = payload; p < end; ++p)
  {
    content += *((const char *)p);
  }
  CoolE::setRevContent(content);
}

String CoolE::rev_content = "";
String CoolE::rev_content_temp = "";
void CoolE::setRevContent(String content)
{
  rev_content = content;
}
String CoolE::getRevContent()
{
  if (rev_content != rev_content_temp && rev_content != NULL)
  {
    rev_content_temp = rev_content;
    return rev_content;
  }
  else
  {
    return "";
  }
}

void CoolE::printRev()
{
  if (rev_content != rev_content_temp && rev_content != NULL)
  {
    Serial.println(rev_content);
    rev_content_temp = rev_content;
  }
}

void CoolE::setClearWiFi()
{
  _clear_wifi = true;
}

CoolE::CoolE() {}

CoolE::CoolE(const char *developkey)
{
  _developkey = developkey;
}
#ifndef WITHOUT_WEB
void CoolE::init()
{
  Serial.begin(115200);
  AsyncWebServer server(80);
  DNSServer dns;
  AsyncWiFiManager wifiManager(&server, &dns); // 创建 wifimanager 对象
  if (_clear_wifi)
  {
    wifiManager.resetSettings(); // 重置保存的修改
  }
  wifiManager.setConnectTimeout(10);                    // 配置连接超时
  wifiManager.setDebugOutput(_is_debug);                // 打印调试内容
  wifiManager.setMinimumSignalQuality(30);              // 设置最小信号强度
  IPAddress _ip = IPAddress(192, 168, 4, 1);            // 设置固定AP信息
  IPAddress _gw = IPAddress(192, 168, 4, 1);            // 设置固定AP信息
  IPAddress _sn = IPAddress(255, 255, 255, 0);          // 设置固定AP信息
  wifiManager.setAPStaticIPConfig(_ip, _gw, _sn);       // 设置固定AP信息
  wifiManager.setBreakAfterConfig(true);                // 设置 如果配置错误的ssid或者密码 退出配置模式
  wifiManager.setRemoveDuplicateAPs(true);              // 设置过滤重复的AP 默认可以不用调用 这里只是示范
  if (!wifiManager.autoConnect(_device_id, "cooleiot")) // 尝试连接网络，失败去到配置页面
  {
    debug("连接失败，准备重启尝试");
#if defined(ESP8266)
    ESP.reset(); //重置并重试
#endif
#if defined(ESP32)
    ESP.restart(); //重置并重试
#endif
    delay(1000);
  }
  _wifi_connected = true;
  // 请求设备详情接口
  httpclient.get("/device/develop/"+String(_developkey));
  httpclient.sendHeader(HTTP_HEADER_CONTENT_TYPE, "application/x-www-form-urlencoded");
  // int statusCode = httpclient.responseStatusCode();
  String response = httpclient.responseBody();
  //解析JSON
  const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(19) + 440;
  DynamicJsonDocument res(capacity);
  DeserializationError error = deserializeJson(res, response);
  if (error) {
    debug("deserializeJson() 失败:");
    debug(error.f_str());
    return;
  }
  debug(response);
  int req_code = res["code"];
  // const char* req_msg = res["msg"];
  JsonObject data = res["data"];
  if(req_code == 1000){
    const char* data_device_id = data["device_id"];
    _device_id = data_device_id;
    const char* data_username = data["username"];
    _username = data_username;
  }else{
    return;
  }
  res.clear();
  _topic = "cooleiot/" + String(_username) + "/" + String(_device_id);
  #if defined(ESP8266)
    _client_id = "coole-device-" + String(_device_id) + '-' + ESP.getChipId();
  #else
    _client_id = "coole-device-" + String(_device_id) + '-' + get32ChipID();
  #endif
}
#endif

void CoolE::init(const char *ssid, const char *pswd)
{
  Serial.begin(115200);
  //Connect WiFi
  WiFi.begin(ssid, pswd);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  _wifi_connected = true;
  // 请求设备详情接口
  httpclient.get("/device/develop/"+String(_developkey));
  httpclient.sendHeader(HTTP_HEADER_CONTENT_TYPE, "application/x-www-form-urlencoded");
  // int statusCode = httpclient.responseStatusCode();
  String response = httpclient.responseBody();
  //解析JSON
  const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(19) + 440;
  DynamicJsonDocument res(capacity);
  DeserializationError error = deserializeJson(res, response);
  if (error) {
    debug("deserializeJson() 失败:");
    debug(error.f_str());
    return;
  }
  debug(response);
  int req_code = res["code"];
  // const char* req_msg = res["msg"];
  JsonObject data = res["data"];
  if(req_code == 1000){
    const char* data_device_id = data["device_id"];
    _device_id = data_device_id;
    const char* data_username = data["username"];
    _username = data_username;
  }else{
    return;
  }
  res.clear();
  _topic = "cooleiot/" + String(_username) + "/" + String(_device_id);
  #if defined(ESP8266)
    _client_id = "coole-device-" + String(_device_id) + '-' + ESP.getChipId();
  #else
    _client_id = "coole-device-" + String(_device_id) + '-' + get32ChipID();
  #endif
}

void CoolE::connect()
{
  if (!mqttclient.connected())
  {
    debug("MQTT 状态码:" + String(mqttclient.state()));
    if (mqttclient.connect(_client_id.c_str(), _developkey, NULL))
    {
      debug("MQTT 已连接.");
      debug("Topic:" + _topic);
      debug("Client Id:" + _client_id);
      mqttclient.subscribe(_topic.c_str());
      mqttclient.setCallback(callback);
    }
  }
  else
  {
    static int counter = 0;
    String buffer{"尝试次数:" + String{counter++}};
    debug(buffer);
  }
}

#ifndef WITHOUT_WEB
void CoolE::start()
{
  init();
  connect();
}
#endif

void CoolE::start(const char *ssid, const char *pswd)
{
  init(ssid, pswd);
  connect();
}

void CoolE::stop()
{
  mqttclient.unsubscribe(_topic.c_str());
  mqttclient.disconnect();
}

bool CoolE::wifistatus()
{
  return _wifi_connected;
}

void CoolE::loop()
{
  mqttclient.loop();
}

void CoolE::clearReport()
{
  field_num = 0;
  report.clear();
}

void CoolE::clearContent()
{
  content.clear();
}

String CoolE::getReport(String field){
  if (!report.containsKey(field))
  {
    return "";
  }
  return report[field];
}

void CoolE::updateReport(String field, String data)
{
  if (field_num > 30)
  {
    debug("最多存储30个字段!");
    return;
  }
  report[field] = data;
  serializeJson(report, _report_content);
  if (!report.containsKey(field))
  {
    field_num++;
  }
}
void CoolE::updateReport(String field, const char* data)
{
  if (field_num > 30)
  {
    debug("最多存储30个字段!");
    return;
  }
  report[field] = data;
  serializeJson(report, _report_content);
  if (!report.containsKey(field))
  {
    field_num++;
  }
}
void CoolE::updateReport(String field, int data)
{
  if (field_num > 30)
  {
    debug("最多存储30个字段!");
    return;
  }
  report[field] = data;
  serializeJson(report, _report_content);
  if (!report.containsKey(field))
  {
    field_num++;
  }
}
void CoolE::updateReport(String field, float data)
{
  if (field_num > 30)
  {
    debug("最多存储30个字段!");
    return;
  }
  report[field] = data;
  serializeJson(report, _report_content);
  if (!report.containsKey(field))
  {
    field_num++;
  }
}
void CoolE::updateReport(String field, byte data)
{
  if (field_num > 30)
  {
    debug("最多存储30个字段!");
    return;
  }
  report[field] = data;
  serializeJson(report, _report_content);
  if (!report.containsKey(field))
  {
    field_num++;
  }
}

String CoolE::getContent(String field){
  if (!content.containsKey(field))
  {
    return "";
  }
  return content[field];
}

void CoolE::updateContent(String field, String data)
{
  content[field] = data;
  serializeJson(content, _content_content);
}
void CoolE::updateContent(String field, const char* data)
{
  content[field] = data;
  serializeJson(content, _content_content);
}

void CoolE::publish()
{
  if (millis() - last_publish_time < 1000 && last_publish_time != 0)
  {
    delay(1000 - (millis() - last_publish_time));
  }
  serializeJson(report, _publish_content);
  String publish_data = "{\"report\":" + _publish_content + "}";
  mqttclient.publish(_topic.c_str(), publish_data.c_str());
  last_publish_time = millis();
  _publish_content = "";
  doc.clear();
}

void CoolE::publish(String field)
{
  if (millis() - last_publish_time < 1000 && last_publish_time != 0)
  {
    delay(1000 - (millis() - last_publish_time));
  }
  if (!report.containsKey(field))
  {
    // debug("无[" + field + "]字段，请先update或检查是否clear");
    return;
  }
  doc[field] = report[field];
  serializeJson(doc, _publish_content);
  String publish_data = "{\"report\":" + _publish_content + "}";
  mqttclient.publish(_topic.c_str(), publish_data.c_str());
  last_publish_time = millis();
  _publish_content = "";
  doc.clear();
}

void CoolE::publish(String key, String value, enum MessageType type)
{
  if (millis() - last_publish_time < 1000 && last_publish_time != 0)
  {
    delay(1000 - (millis() - last_publish_time));
  }
  String publish_data = type == STRING ? "{\"" + key + "\":" + "\"" + value + "\"" + "}" : "{\"" + key + "\":" + value + "}";
  mqttclient.publish(_topic.c_str(), publish_data.c_str());
}

void CoolE::publish(String field, String payload)
{
  if (millis() - last_publish_time < 1000 && last_publish_time != 0)
  {
    delay(1000 - (millis() - last_publish_time));
  }
  doc[field] = payload;
  serializeJson(doc, _publish_content);
  String publish_data = "{\"report\":" + _publish_content + "}";
  mqttclient.publish(_topic.c_str(), publish_data.c_str());
  last_publish_time = millis();
  _publish_content = "";
  doc.clear();
}
void CoolE::publish(String field, const char* payload)
{
  if (millis() - last_publish_time < 1000 && last_publish_time != 0)
  {
    delay(1000 - (millis() - last_publish_time));
  }
  doc[field] = payload;
  serializeJson(doc, _publish_content);
  String publish_data = "{\"report\":" + _publish_content + "}";
  mqttclient.publish(_topic.c_str(), publish_data.c_str());
  last_publish_time = millis();
  _publish_content = "";
  doc.clear();
}
void CoolE::publish(String field, int payload)
{
  if (millis() - last_publish_time < 1000 && last_publish_time != 0)
  {
    delay(1000 - (millis() - last_publish_time));
  }
  doc[field] = payload;
  serializeJson(doc, _publish_content);
  String publish_data = "{\"report\":" + _publish_content + "}";
  mqttclient.publish(_topic.c_str(), publish_data.c_str());
  last_publish_time = millis();
  _publish_content = "";
  doc.clear();
}
void CoolE::publish(String field, float payload)
{
  if (millis() - last_publish_time < 1000 && last_publish_time != 0)
  {
    delay(1000 - (millis() - last_publish_time));
  }
  doc[field] = payload;
  serializeJson(doc, _publish_content);
  String publish_data = "{\"report\":" + _publish_content + "}";
  mqttclient.publish(_topic.c_str(), publish_data.c_str());
  last_publish_time = millis();
  _publish_content = "";
  doc.clear();
}
void CoolE::publish(String field, byte payload)
{
  if (millis() - last_publish_time < 1000 && last_publish_time != 0)
  {
    delay(1000 - (millis() - last_publish_time));
  }
  doc[field] = payload;
  serializeJson(doc, _publish_content);
  String publish_data = "{\"report\":" + _publish_content + "}";
  mqttclient.publish(_topic.c_str(), publish_data.c_str());
  last_publish_time = millis();
  _publish_content = "";
  doc.clear();
}

String CoolE::decode(String payload, enum DecodeType type)
{
  if (payload == "")
  {
    return "";
  }
  DeserializationError error = deserializeJson(doc, payload);
  if (error)
  {
    debug("反序列化JSON时失败：");
    debug(error.f_str());
    return "";
  }
  const char *result = "";
  switch (type)
  {
  case COMMAND:
    result = doc["command"];
    break;
  case GET:
    result = doc["get"];
    break;
  case CONTENT_NAME:
    result = doc["content"]["n"];
    break;
  case CONTENT_DATA:
    result = doc["content"]["d"];
    break;
  default:
    break;
  }
  doc.clear();
  return String(result);
}

String CoolE::getDeviceId(){
  return _device_id;
}

String CoolE::getStrConfig(String field){
  // 请求设备详情接口
  httpclient.get("/device/develop/"+String(_developkey)+"/config/"+field);
  httpclient.sendHeader(HTTP_HEADER_CONTENT_TYPE, "application/x-www-form-urlencoded");
  // int statusCode = httpclient.responseStatusCode();
  String response = httpclient.responseBody();
  //解析JSON
  const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(2) + 420;
  DynamicJsonDocument res(capacity);
  DeserializationError error = deserializeJson(res, response);
  if (error) {
    debug("deserializeJson() 失败:");
    debug(error.f_str());
    return "";
  }
  debug(response);
  int req_code = res["code"];
  // const char* req_msg = res["msg"];
  JsonObject data = res["data"];
  int type = data["type"];
  if(req_code == 1000){
    if(type==1){
      String config_content_data = data["config_content"]["data"];
      debug("请求云配置"+field+"内容："+config_content_data);
      return config_content_data;
    }else{
      debug("错误的数据格式，getStrConfig()只能获取String类型云配置");
      return "";
    }
  }else{
    return "";
  }
  res.clear();
}

float CoolE::getNumConfig(String field){
  // 请求设备详情接口
  httpclient.get("/device/develop/"+String(_developkey)+"/config/"+field);
  httpclient.sendHeader(HTTP_HEADER_CONTENT_TYPE, "application/x-www-form-urlencoded");
  // int statusCode = httpclient.responseStatusCode();
  String response = httpclient.responseBody();
  //解析JSON
  const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(2) + 420;
  DynamicJsonDocument res(capacity);
  DeserializationError error = deserializeJson(res, response);
  if (error) {
    debug("deserializeJson() 失败:");
    debug(error.f_str());
    return 0;
  }
  debug(response);
  int req_code = res["code"];
  // const char* req_msg = res["msg"];
  JsonObject data = res["data"];
  int type = data["type"];
  if(req_code == 1000){
    if(type==2){
      float config_content_data = data["config_content"]["data"];
      debug("请求云配置"+field+"内容："+config_content_data);
      return config_content_data;
    }else{
      debug("错误的数据格式，getNumConfig()只能获取Number类型云配置");
      return 0;
    }
  }else{
    return 0;
  }
  res.clear();
}


void CoolE::setDebug(bool result)
{
  _is_debug = result;
}

template <typename Generic>
void CoolE::debug(Generic text)
{
  if (_is_debug)
  {
    Serial.print(F("*CoolE: "));
    Serial.println(text);
  }
}

static const char HEX_CHAR_ARRAY[17] = "0123456789ABCDEF";

#if !defined(ESP8266)
/**
* convert char array (hex values) to readable string by seperator
* buf:           buffer to convert
* length:        data length
* strSeperator   seperator between each hex value
* return:        formated value as String
*/
static String byteToHexStr(uint8_t *buf, uint8_t length, String strSeperator = "-")
{
  String dataString = "";
  for (uint8_t i = 0; i < length; i++)
  {
    byte v = buf[i] / 16;
    byte w = buf[i] % 16;
    if (i > 0)
    {
      dataString += strSeperator;
    }
    dataString += String(HEX_CHAR_ARRAY[v]);
    dataString += String(HEX_CHAR_ARRAY[w]);
  }
  dataString.toUpperCase();
  return dataString;
} // byteToHexStr

String get32ChipID()
{
  uint64_t chipid;
  chipid = ESP.getEfuseMac(); // the chip ID is essentially its MAC address (length: 6 bytes)
  uint8_t chipid_size = 6;
  uint8_t chipid_arr[chipid_size];
  for (uint8_t i = 0; i < chipid_size; i++)
  {
    chipid_arr[i] = (chipid >> (8 * i)) & 0xff;
  }
  return byteToHexStr(chipid_arr, chipid_size, "");
}
#endif
// end.
