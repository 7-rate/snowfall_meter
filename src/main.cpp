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
#include "env.h"

/******************************************************************/
/* Definitions                                                    */
/******************************************************************/
/***********************************/
/* Local definitions               */
/***********************************/
#define ADC_BATTERY_V (A0)
#define ADC_BATTERY_V_SCALE (200) // 1/2分圧
#define ADC_BATTERY_V_BITS (12)   // 12ビットADC
#define ADC_BATTERY_V_REF (3300)  // 3.3V

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
int battery_voltage;

/***********************************/
/* Global Variables                */
/***********************************/

/******************************************************************/
/* Implementation                                                 */
/******************************************************************/
/***********************************/
/* Local functions                 */
/***********************************/
static int calculate_actual_voltage(int divider_ratio, int adc_bits, int adc_ref_voltage_mv,
                                    int adc_value) {
    int max_adc_value = (1 << adc_bits) - 1;
    int input_voltage_mv = (adc_value * adc_ref_voltage_mv) / max_adc_value;
    return (input_voltage_mv * divider_ratio) / 100;
}

static void printCurrentTime(struct tm *timeinfo, char *buffer) {
    strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", timeinfo);
}

static void dormancy() {
    // ネットワークを切る
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);

    // センサーを省電力モードにする
    sensor.stopContinuous();

    // I2Cを切る
    Wire.end();

    // deepsleep
    // TODO 時間の計算
    uint64_t sleep_time_us = 60 * 1000 * 1000; // 60秒
    esp_deep_sleep(sleep_time_us);
}

static void VL53L1X_setup() {
    sensor.setTimeout(5000);
    sensor.setBus(&Wire);
    if (!sensor.init()) {
        DBG_PRINT("Failed to detect and initialize sensor!\n");
        dormancy();
    }
    DBG_PRINT("VL53L1X init\n");

    // 最大測定距離で高精度で測定したいため、以下の設定とする。
    // Distance Mode Long
    // Timing Budget 200ms
    sensor.setDistanceMode(VL53L1X::Long);
    sensor.setMeasurementTimingBudget(200 * 1000);
    sensor.startContinuous(200);
}

static void measure_snowfall() {
    sensor.read();
    DBG_PRINT("range: %d\tstatus: %s\tpeak signal: %f\tambient: %f\n", sensor.ranging_data.range_mm,
              VL53L1X::rangeStatusToString(sensor.ranging_data.range_status),
              sensor.ranging_data.peak_signal_count_rate_MCPS,
              sensor.ranging_data.ambient_count_rate_MCPS);
}

static void measure_battery_voltage() {
    int rawValue = analogRead(ADC_BATTERY_V); // ADC値を読み取る
    battery_voltage =
        calculate_actual_voltage(ADC_BATTERY_V_SCALE, ADC_BATTERY_V_BITS, ADC_BATTERY_V_REF,
                                 rawValue); // 実際の電圧(mV)を計算する
    DBG_PRINT("Battery Voltage: %d\n", battery_voltage);
}

static void get_time() {
    tm timeinfo;
    char buffer[80];
    bool ret = getLocalTime(&timeinfo, 5000);
    if (!ret) {
        DBG_PRINT("Failed setup NTP, Start deep sleep\n");
        dormancy();
        return;
    }
    printCurrentTime(&timeinfo, buffer);
    DBG_PRINT("time %s\n", buffer);
}

static void send_data() {}

/***********************************/
/* Class implementions             */
/***********************************/

/***********************************/
/* Global functions                */
/***********************************/
void setup() {
    DBG_PRINT("Setup Serial \n");
    if (DEBUG) {
        Serial.begin(115200);
    } else {
        Serial.end();
    }
    DBG_PRINT("\n");

    DBG_PRINT("Setup I2C \n");
    Wire.begin();
    DBG_PRINT("\n");

    DBG_PRINT("Setup ADC \n");
    pinMode(ADC_BATTERY_V, INPUT);
    analogReadResolution(ADC_BATTERY_V_BITS); // 12ビット解像度のADCを設定
    DBG_PRINT("\n");

    DBG_PRINT("Setup WiFi \n");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        DBG_PRINT(".");
    }
    DBG_PRINT("\nConnected\n");
    DBG_PRINT("IP address: %s\n", WiFi.localIP().toString().c_str());
    DBG_PRINT("\n");

    DBG_PRINT("Setup NTP \n");
    configTzTime(TZ, NTP_SERVER1, NTP_SERVER2, NTP_SERVER3);
    DBG_PRINT("\n");

    DBG_PRINT("Setup VL53L1X \n");
    VL53L1X_setup();
    DBG_PRINT("\n");
}

void loop() {
    DBG_PRINT("--------Start-------\n");
    // 1.積雪の測定
    measure_snowfall();
    // 2.バッテリー電圧測定
    measure_battery_voltage();
    // 3.時刻取得
    get_time();
    // 4.サーバーへ送信
    send_data();
    // 5.DeepSleep
    dormancy();
    DBG_PRINT("---------End--------\n\n");
}
