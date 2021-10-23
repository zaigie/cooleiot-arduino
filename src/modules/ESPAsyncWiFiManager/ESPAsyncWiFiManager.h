/**************************************************************
   WiFiManager is a library for the ESP8266/Arduino platform
   (https://github.com/esp8266/Arduino) to enable easy
   configuration and reconfiguration of WiFi credentials using a Captive Portal
   inspired by:
   http://www.esp8266.com/viewtopic.php?f=29&t=2520
   https://github.com/chriscook8/esp-arduino-apboot
   https://github.com/esp8266/Arduino/tree/esp8266/hardware/esp8266com/esp8266/libraries/DNSServer/examples/CaptivePortalAdvanced
   Built by AlexT https://github.com/tzapu
   Ported to Async Web Server by https://github.com/alanswx
   Licensed under MIT license
 **************************************************************/

#ifndef ESPAsyncWiFiManager_h
#define ESPAsyncWiFiManager_h

#if defined(ESP8266)
#include <ESP8266WiFi.h> // https://github.com/esp8266/Arduino
#else
#include <WiFi.h>
#include "esp_wps.h"
#define ESP_WPS_MODE WPS_TYPE_PBC
#endif
#include "modules/ESPAsyncWebServer/ESPAsyncWebServer.h"

//#define USE_EADNS               // uncomment to use ESPAsyncDNSServer
#ifdef USE_EADNS
#include <ESPAsyncDNSServer.h> // https://github.com/devyte/ESPAsyncDNSServer
                               // https://github.com/me-no-dev/ESPAsyncUDP
#else
#include <DNSServer.h>
#endif
#include <memory>

// fix crash on ESP32 (see https://github.com/alanswx/ESPAsyncWiFiManager/issues/44)
#if defined(ESP8266)
typedef int wifi_ssid_count_t;
#else
typedef int16_t wifi_ssid_count_t;
#endif

#if defined(ESP8266)
extern "C"
{
#include "user_interface.h"
}
#else
#include <rom/rtc.h>
#endif

const char WFM_HTTP_HEAD[] PROGMEM        = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/><title>{v}</title>";
const char HTTP_STYLE[] PROGMEM           = "<style>.c{text-align: center;} div,input{padding:5px;font-size:1em;} input{width:95%;} body{text-align: center;font-family:verdana;} button{border:0;border-radius:0.3rem;background-color:#1fa3ec;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%;} .q{float: right;width: 64px;text-align: right;} .l{background: url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAMAAABEpIrGAAAALVBMVEX///8EBwfBwsLw8PAzNjaCg4NTVVUjJiZDRUUUFxdiZGSho6OSk5Pg4eFydHTCjaf3AAAAZElEQVQ4je2NSw7AIAhEBamKn97/uMXEGBvozkWb9C2Zx4xzWykBhFAeYp9gkLyZE0zIMno9n4g19hmdY39scwqVkOXaxph0ZCXQcqxSpgQpONa59wkRDOL93eAXvimwlbPbwwVAegLS1HGfZAAAAABJRU5ErkJggg==\") no-repeat left center;background-size: 1em;}</style>";
const char HTTP_SCRIPT[] PROGMEM          = "<script>function c(l){document.getElementById('s').value=l.innerText||l.textContent;document.getElementById('p').focus();}</script>";
const char HTTP_HEAD_END[] PROGMEM      = "</head><body><div style='text-align:left;display:inline-block;min-width:260px;'>";
const char HTTP_PORTAL_OPTIONS[] PROGMEM  = "<form action=\"/wifi\" method=\"get\"><button>配置WIFI</button></form><br /><form action=\"/0wifi\" method=\"get\"><button>配置WIFI(手动输入)</button></form><br /><form action=\"/i\" method=\"get\"><button>设备信息</button></form><br /><form action=\"/r\" method=\"post\"><button>重启</button></form>";
const char HTTP_ITEM[] PROGMEM            = "<div><a href='#p' onclick='c(this)'>{v}</a>&nbsp;<span class='q {i}'>{r}%</span></div>";
const char HTTP_FORM_START[] PROGMEM      = "<form method='get' action='wifisave'><input id='s' name='s' length=32 placeholder='WIFI名称'><br/><input id='p' name='p' length=64 type='password' placeholder='WIFI密码'><br/>";
const char HTTP_FORM_PARAM[] PROGMEM      = "<br/><input id='{i}' name='{n}' maxlength={l} placeholder='{p}' value='{v}' {c}>";
const char HTTP_FORM_END[] PROGMEM        = "<br/><button type='submit'>确定</button></form>";
const char HTTP_SCAN_LINK[] PROGMEM       = "<br/><div class=\"c\"><a href=\"/wifi\">扫描</a></div>";
const char HTTP_SAVED[] PROGMEM           = "<div>WIFI凭据已发送给设备并保存<br />正在尝试将设备连接到网络..<br />如果失败，设备将在10秒后重启，请重新连接到本WIFI以重试~</div>";
const char HTTP_END[] PROGMEM             = "</div></body></html>";

#define WIFI_MANAGER_MAX_PARAMS 10

class AsyncWiFiManagerParameter
{
public:
/** 
  创建可添加到WiFiManager设置网页的自定义参数
  @id用于HTTP查询，不能包含空格或其他特殊字符
*/
  AsyncWiFiManagerParameter(const char *custom);
  AsyncWiFiManagerParameter(const char *id,
                            const char *placeholder,
                            const char *defaultValue,
                            unsigned int length);
  AsyncWiFiManagerParameter(const char *id,
                            const char *placeholder,
                            const char *defaultValue,
                            unsigned int length,
                            const char *custom);

  const char *getID();
  const char *getValue();
  const char *getPlaceholder();
  unsigned int getValueLength();
  const char *getCustomHTML();

private:
  const char *_id;
  const char *_placeholder;
  char *_value;
  unsigned int _length;
  const char *_customHTML;

  void init(const char *id,
            const char *placeholder,
            const char *defaultValue,
            unsigned int length,
            const char *custom);

  friend class AsyncWiFiManager;
};

class WiFiResult
{
public:
  bool duplicate;
  String SSID;
  uint8_t encryptionType;
  int32_t RSSI;
  uint8_t *BSSID;
  int32_t channel;
  bool isHidden;

  WiFiResult()
  {
  }
};

class AsyncWiFiManager
{
public:
#ifdef USE_EADNS
  AsyncWiFiManager(AsyncWebServer *server, AsyncDNSServer *dns);
#else
  AsyncWiFiManager(AsyncWebServer *server, DNSServer *dns);
#endif

  void scan(boolean async = false);
  String scanModal();
  void loop();
  void safeLoop();
  void criticalLoop();
  String infoAsString();

  boolean autoConnect(unsigned long maxConnectRetries = 1,
                      unsigned long retryDelayMs = 1000);
  boolean autoConnect(char const *apName,
                      char const *apPassword = NULL,
                      unsigned long maxConnectRetries = 1,
                      unsigned long retryDelayMs = 1000);

  //如果您希望总是启动配置门户，而不首先尝试连接
  boolean startConfigPortal(char const *apName, char const *apPassword = NULL);
  void startConfigPortalModeless(char const *apName, char const *apPassword);

  //获取配置门户的AP名称，以便在回调中使用
  String getConfigPortalSSID();

  void resetSettings();

  //设置Web服务器循环结束和退出之前的超时，即使没有设置。
  //对于在某个点连接失败并卡在Web服务器循环中的设备非常有用
  //in seconds setConfigPortalTimeout is a new name for setTimeout
  void setConfigPortalTimeout(unsigned long seconds);
  void setTimeout(unsigned long seconds);

  //设置尝试连接的超时时间，如果有很多失败的连接，这会很有用
  void setConnectTimeout(unsigned long seconds);

  // wether or not the wifi manager tries to connect to configured access point even when
  // configuration portal (ESP as access point) is running [default true/on]
  void setTryConnectDuringConfigPortal(boolean v);

  void setDebugOutput(boolean debug);
  //如果调用，则默认不显示低于8%信号质量的任何内容
  void setMinimumSignalQuality(unsigned int quality = 8);
  //设置自定义ip/网关/子网配置
  void setAPStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn);
  //设置静态IP的配置
  void setSTAStaticIPConfig(IPAddress ip,
                            IPAddress gw,
                            IPAddress sn,
                            IPAddress dns1 = (uint32_t)0x00000000,
                            IPAddress dns2 = (uint32_t)0x00000000);
  //在AP模式和配置门户启动时调用
  void setAPCallback(void (*func)(AsyncWiFiManager *));
  //当设置已更改且连接成功时调用
  void setSaveConfigCallback(void (*func)(void));
  //添加自定义参数，失败时返回false
  void addParameter(AsyncWiFiManagerParameter *p);
  //如果设置了此选项，即使连接失败，它也会在配置后退出。
  void setBreakAfterConfig(boolean shouldBreak);
  //如果已设置，请在启动时尝试WPS设置（这将延迟配置门户最多2分钟）
  //TODO
  //如果已设置，请自定义样式
  void setCustomHeadElement(const char *element);
  //如果为true，则删除重复的访问点-默认为 true
  void setRemoveDuplicateAPs(boolean removeDuplicates);
  // sets a custom element to add to options page
  void setCustomOptionsElement(const char *element);

private:
  AsyncWebServer *server;
#ifdef USE_EADNS
  AsyncDNSServer *dnsServer;
#else
  DNSServer *dnsServer;
#endif

  boolean _modeless;
  unsigned long scannow;
  boolean shouldscan = true;
  boolean needInfo = true;

  //const int     WM_DONE                 = 0;
  //const int     WM_WAIT                 = 10;
  //const String  HTTP_HEAD = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"/><title>{v}</title>";

  void setupConfigPortal();
#ifdef NO_EXTRA_4K_HEAP
  void startWPS();
#endif
  String pager;
  wl_status_t wifiStatus;
  const char *_apName = "no-net";
  const char *_apPassword = NULL;
  String _ssid = "";
  String _pass = "";
  unsigned long _configPortalTimeout = 0;
  unsigned long _connectTimeout = 0;
  unsigned long _configPortalStart = 0;

  IPAddress _ap_static_ip;
  IPAddress _ap_static_gw;
  IPAddress _ap_static_sn;
  IPAddress _sta_static_ip;
  IPAddress _sta_static_gw;
  IPAddress _sta_static_sn;
  IPAddress _sta_static_dns1 = (uint32_t)0x00000000;
  IPAddress _sta_static_dns2 = (uint32_t)0x00000000;

  unsigned int _paramsCount = 0;
  unsigned int _minimumQuality = 0;
  boolean _removeDuplicateAPs = true;
  boolean _shouldBreakAfterConfig = false;
#ifdef NO_EXTRA_4K_HEAP
  boolean _tryWPS = false;
#endif
  const char *_customHeadElement = "";
  const char *_customOptionsElement = "";

  //String        getEEPROMString(int start, int len);
  //void          setEEPROMString(int start, int len, String string);

  uint8_t status = WL_IDLE_STATUS;
  uint8_t connectWifi(String ssid, String pass);
  uint8_t waitForConnectResult();
  void setInfo();
  void copySSIDInfo(wifi_ssid_count_t n);
  String networkListAsString();

  void handleRoot(AsyncWebServerRequest *);
  void handleWifi(AsyncWebServerRequest *, boolean scan);
  void handleWifiSave(AsyncWebServerRequest *);
  void handleInfo(AsyncWebServerRequest *);
  void handleReset(AsyncWebServerRequest *);
  void handleNotFound(AsyncWebServerRequest *);
  boolean captivePortal(AsyncWebServerRequest *);

  // DNS server
  const byte DNS_PORT = 53;

  // helpers
  unsigned int getRSSIasQuality(int RSSI);
  boolean isIp(String str);
  String toStringIp(IPAddress ip);

  boolean connect;
  boolean _debug = true;

  WiFiResult *wifiSSIDs;
  wifi_ssid_count_t wifiSSIDCount;
  boolean wifiSSIDscan;

  boolean _tryConnectDuringConfigPortal = true;

  void (*_apcallback)(AsyncWiFiManager *) = NULL;
  void (*_savecallback)(void) = NULL;

  AsyncWiFiManagerParameter *_params[WIFI_MANAGER_MAX_PARAMS];

  template <typename Generic>
  void DEBUG_WM(Generic text);

  template <class T>
  auto optionalIPFromString(T *obj, const char *s) -> decltype(obj->fromString(s))
  {
    return obj->fromString(s);
  }
  auto optionalIPFromString(...) -> bool
  {
    DEBUG_WM(F("IP地址上没有fromString方法，您需要ESP8266 core 2.1.0或更新版本才能使用自定义IP配置。"));
    return false;
  }
};

#endif
