//#define ESP_01
//#define ESP_NODE_MCU
//#define ESP_2floor
//#define ESP_GARDEN //для улицы ворота
//#define ESP_KOTEL
#define ESP_2BUTTON //для зала/детской/коридора на выключатели
//#define ESP_cherdak //чердак 
//#define run_lesnica // контролер подсветка лесницы
//#define ESP_kitchen

//1Mb/256 - ESP-01 - черный
//512/128 - ESP-01 - синий

//#define DEBUG_ENABLE
#define UN_drebizg 2
#define T_PERIOD 10 //период кратности для опроса датчиков
#define UART_SPEAD 115200
//#define DEF_CONFIG_ON_LOAD //Загрузка конфигурации по умолчанию при старте

#ifdef ESP_01
  #define ONE_WIRE_PORT 0
  //#define init_debug_port 2
  //#define RDM6300
  //#define RDM6300_APARAT_UART

  #define FILE_EDIT

  #define DS_1820_personal_convert
#endif

#ifdef ESP_NODE_MCU
  #define DHT11_PIN D5
  //#define DHTTYPE           DHT11     // DHT 11 
  #define DHTTYPE           DHT22     // DHT 22 (AM2302)
  //#define DHTTYPE           DHT21     // DHT 21 (AM2301)

  #define BMP180 // 1 - Pa 2 - mm rt st
  //#define analog_pin A0

  //#define RDM6300
  //#define RDM6300_APARAT_UART
  //#define RDM6300_SOFT_UART_RX 10
  //#define RDM6300_SOFT_UART_TX 11
  
  #define FILE_EDIT

  #define ONE_WIRE_PORT D3
  #define DS_1820_personal_convert
  //#define init_debug_port D1

  #define wifi_firmware_update

  #define Alarm_mode
#endif

#ifdef  ESP_2floor
  #define DHT11_PIN D4
  #define DHTTYPE           DHT11     // DHT 11 
  //#define DHTTYPE           DHT22     // DHT 22 (AM2302)
  //#define DHTTYPE           DHT21     // DHT 21 (AM2301)

  #define ONE_WIRE_PORT D3
  #define wifi_firmware_update
#endif

#ifdef ESP_kitchen
  #define DHT11_PIN D5
  //#define DHTTYPE           DHT11     // DHT 11 
  #define DHTTYPE           DHT22     // DHT 22 (AM2302)
  //#define DHTTYPE           DHT21     // DHT 21 (AM2301)

  #define BMP180 // 1 - Pa 2 - mm rt st
  //#define analog_pin A0
  
  #define FILE_EDIT

  #define ONE_WIRE_PORT D3

  #define wifi_firmware_update
#endif

#ifdef ESP_GARDEN
  #define ONE_WIRE_PORT 0
  #define DS_1820_personal_convert
  //#define init_debug_port 2
  #define RDM6300
  #define RDM6300_APARAT_UART

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
  //1Mb/256
  #define ONE_WIRE_PORT 0
  //#define DS_1820_personal_convert
  //#define init_debug_port 2
  //#define RDM6300
  //#define RDM6300_APARAT_UART

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
