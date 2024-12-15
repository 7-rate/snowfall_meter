/*
 * 概要：距離センサを用いて積雪をセンシングし、サーバーにデータを送信する。
 */
// 1.積雪の測定
//    VL53L1Xを用いて積雪の深さを測定する。
// 2.バッテリー電圧測定
//    ADCを用いてVSYS電圧を測定する。
// 3.時刻取得
//    NTPサーバーから現在時刻を取得する。
// 4.サーバーへ送信
//    WiFi経由でhttp通信を用いてサーバーにデータを送信する。
// 5.休眠
//   現在時刻を加味した休眠時間を計算し、休眠する。休眠後は再起動

#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <VL53L1X.h>
#include "pico/stdlib.h"
#include "env.h"

/******************************************************************/
/* Definitions                                                    */
/******************************************************************/
/***********************************/
/* Local definitions               */
/***********************************/
#define VSYS_ADC_PIN 29    // GPIO29 (ADC3)
#define ENABLE_PIN 25      // GPIO25
#define ADC_MAX_VALUE 4095 // 12ビットADCの最大値
#define VREF 3.3           // ADCのリファレンス電圧 (V)
#define SCALE_FACTOR 3.0   // VSYSは1/3スケールで分圧されている

#define DEBUG (1)
#define DBG_PRINT(__fmt__, ...)                                                                    \
    do {                                                                                           \
        if (DEBUG) {                                                                               \
            Serial.printf(__fmt__, ##__VA_ARGS__);                                                 \
        }                                                                                          \
    } while (0)

/***********************************/
/* Local Variables                 */
/***********************************/
VL53L1X sensor;
float battery_voltage;

/***********************************/
/* Global Variables                */
/***********************************/

/******************************************************************/
/* Implementation                                                 */
/******************************************************************/
/***********************************/
/* Local functions                 */
/***********************************/
static void VL53L1X_setup() {
    sensor.setTimeout(5000);
    sensor.setBus(&Wire);
    if (!sensor.init()) {
        DBG_PRINT("Failed to detect and initialize sensor!\n");
        while (1)
            ;
    }
    DBG_PRINT("VL53L1X init\n");

    // 最大測定距離で高精度で測定したいため、以下の設定とする。
    // Distance Mode Long
    // Timing Budget 200ms
    sensor.setDistanceMode(VL53L1X::Long);
    sensor.setMeasurementTimingBudget(200 * 1000);
    sensor.startContinuous(200);
}

// 1.積雪の測定
static void measure_snowfall() {
    sensor.read();
    DBG_PRINT("range: %d\tstatus: %s\tpeak signal: %f\tambient: %f\n", sensor.ranging_data.range_mm,
              VL53L1X::rangeStatusToString(sensor.ranging_data.range_status),
              sensor.ranging_data.peak_signal_count_rate_MCPS,
              sensor.ranging_data.ambient_count_rate_MCPS);
}

// 2.バッテリー電圧測定
static void measure_battery_voltage() {
    int rawValue = analogRead(VSYS_ADC_PIN);           // ADC値を読み取る
    float voltage = (rawValue * VREF) / ADC_MAX_VALUE; // ADC値を電圧に変換
    battery_voltage = (voltage * SCALE_FACTOR); // スケールファクターを掛けてVSYS電圧を計算
}

// 3.時刻取得
static void get_time() {
    static time_t now;
    time(&now);
    struct tm *timeinfo = localtime(&now);
    DBG_PRINT("Time: %s", asctime(timeinfo));
}
// 4.サーバーへ送信
static void send_data() {}
// 5.休眠
static void dormancy() {
    // ネットワークを切る
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    digitalWrite(23, LOW);

    // センサーを省電力モードにする
    sensor.stopContinuous();

    // 10MHzに設定
    set_sys_clock_khz(10000, false);

    uint32_t sleep_time;
    // TODO 夜間は15分に一度、昼間は1時間に一度起きる
    sleep_time = 15 * 60 * 1000;
    sleep_ms(sleep_time);

    rp2040.reboot();
}

/***********************************/
/* Class implementions             */
/***********************************/

/***********************************/
/* Global functions                */
/***********************************/
void setup() {
    Serial.begin(115200);
    sleep_ms(2000);

    // Wire setup
    Wire.begin();
    Wire1.begin();
    DBG_PRINT("Wire setup\n");

    // WiFi setup
    // WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    // while (WiFi.status() != WL_CONNECTED) {
    //     delay(1000);
    //     DBG_PRINT("Connecting to WiFi..\n");
    // }
    // DBG_PRINT("Connected to the WiFi network\n");

    // NTP setup
    // NTP.begin("ntp.nict.jp", "time.google.com");
    // NTP.waitSet();
    // setenv("TZ", "JST-9", 1);
    // tzset();

    // 電源電圧の測定のためのセットアップ
    // pinMode(ENABLE_PIN, OUTPUT);
    // digitalWrite(ENABLE_PIN, HIGH);
    // analogReadResolution(12); // 12ビット解像度のADCを設定

    // VL53L1Xセンサーのセットアップ
    VL53L1X_setup();
}

void loop() {
    DBG_PRINT("Start loop\n");
    // 1.積雪の測定
    measure_snowfall();
    // 2.バッテリー電圧測定
    // measure_battery_voltage();
    // 3.時刻取得
    // get_time();
    // 4.サーバーへ送信
    // send_data();
    // 5.休眠
    // DBG_PRINT("Start dormancy\n");
    // dormancy();
}
