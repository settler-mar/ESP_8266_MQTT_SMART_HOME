//#define ESP_01
//#define ESP_NODE_MCU
//#define ESP_2floor
//#define ESP_GARDEN //для улицы ворота
//#define ESP_KOTEL
//#define ESP_2BUTTON //для зала/детской/коридора на выключатели
//#define ESP_cherdak //чердак
//#define run_lesnica // контролер подсветка лесницы
//#define ESP_kitchen
#define ESP_test
#define wifi_ota
#define wifi_update
#define web_reboot

//#define DEF_CONFIG_ON_LOAD //Загрузка конфигурации по умолчанию при старте
//#define ESP_auth_def "admin"
//#define ESP_pass_def "admin"

//1Mb/256 - ESP-01 - черный
//512/128 - ESP-01 - синий

//4M/1M - ESP-12 - черный
#define FILE_EDIT

//#define DEBUG_ENABLE
#define UN_drebizg 2
#define T_PERIOD 10 //период кратности для опроса датчиков
#define UART_SPEAD 115200
#define pulse_perion 10

#define ESP_auth_def

#ifdef ESP_test
  #define ESP12_pins
  

 // #define DHT11_PIN D5
//#define DHTTYPE           DHT11     // DHT 11
 // #define DHTTYPE           DHT22     // DHT 22 (AM2302)
//#define DHTTYPE           DHT21     // DHT 21 (AM2301)


  //#define FILE_EDIT

  //#define ONE_WIRE_PORT D3
  //#define RC433_PORT D7

  //#define MCP
  //#define PCA
  
#endif

#ifdef ESP_test2
  #define FILE_EDIT

  #define WS_PIN  2     // 0 = GPIO0, 2=GPIO2
  #define WS_LED_COUNT 7
#endif

#ifdef ESP_01
  #define ONE_WIRE_PORT 0
//#define init_debug_port 2
//#define RDM6300
//#define RDM6300_APARAT_UART

  #define FILE_EDIT

//#define DS_1820_personal_convert
#endif

#ifdef ESP_NODE_MCU
  #define ESP12_pins
  #define DHT11_PIN D5
//#define DHTTYPE           DHT11     // DHT 11
  #define DHTTYPE           DHT22     // DHT 22 (AM2302)
//#define DHTTYPE           DHT21     // DHT 21 (AM2301)

  #define BMP180 2// 1 - Pa 2 - mm rt st
//#define analog_pin A0

//#define RDM6300
//#define RDM6300_APARAT_UART
//#define RDM6300_SOFT_UART_RX 10
//#define RDM6300_SOFT_UART_TX 11

  #define FILE_EDIT

  #define ONE_WIRE_PORT D3
  #define DS_1820_personal_convert
//#define init_debug_port D1


  #define Alarm_mode
#endif

#ifdef  ESP_2floor
  #define ESP12_pins

  #define DHT11_PIN 14
  #define DHTTYPE           DHT11     // DHT 11
//#define DHTTYPE           DHT22     // DHT 22 (AM2302)
//#define DHTTYPE           DHT21     // DHT 21 (AM2301)

  #define MCP
  #define ONE_WIRE_PORT D3
  
#endif

#ifdef ESP_kitchen
  #define ESP12_pins
  #define DHT11_PIN 14
//#define DHTTYPE           DHT11     // DHT 11
  #define DHTTYPE           DHT22     // DHT 22 (AM2302)
//#define DHTTYPE           DHT21     // DHT 21 (AM2301)

  #define BMP180 2 // 1 - Pa 2 - mm rt st
//#define analog_pin A0
//  #define BME_280 2
//  #define BME_280_ADDR 0x76

  #define FILE_EDIT

  #define ONE_WIRE_PORT D3

  #define RC433_PORT D7

#endif

#ifdef ESP_GARDEN
  #define ESP12_pins

  #define MCP
  
  #define ONE_WIRE_PORT D3
  #define DS_1820_personal_convert
//#define init_debug_port 2
  #define RDM6300
  #define RDM6300_APARAT_UART

  #define analog_pin A0

  #define FILE_EDIT
#endif

#ifdef ESP_KOTEL
//1Mb/256
  #define ONE_WIRE_PORT 0
//#define DS_1820_personal_convert
//#define init_debug_port 2
//#define RDM6300
//#define RDM6300_APARAT_UART

  #define FILE_EDIT

  #define PID_TEMP
#endif

#ifdef ESP_2BUTTON
//1Mb/256
  #define ONE_WIRE_PORT 0
//#define DS_1820_personal_convert
//#define init_debug_port 2
  #define FILE_EDIT

//#define PID_TEMP
#endif

#ifdef ESP_cherdak
  #define ESP12_pins
  //4Mb/1
  #define ONE_WIRE_PORT D3

  #define RC433_PORT D7

  #define MCP
  #define PCA
  
  #define FILE_EDIT

//#define PID_TEMP
#endif

#ifdef run_lesnica
  #define ONE_WIRE_PORT 0

  #define ws2812_run
  #define ws2812_run_up_pin D0
  #define ws2812_run_dwn_pin D1
  #define PIN_ws         D2
  #define NUMPIXELS      24
  #define ws2812_run_analog_pin A0

  #define FILE_EDIT
#endif
