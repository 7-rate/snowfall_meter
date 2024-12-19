#pragma once

// WiFi設定
const char *WIFI_SSID = "";
const char *WIFI_PASSWORD = "";

// NTP設定
const char *NTP_SERVER1 = "ntp.nict.jp";
const char *NTP_SERVER2 = "time.google.com";
const char *NTP_SERVER3 = "1.jp.pool.ntp.org";
const char *TZ = "JST-9";

// 測定設定
const int DAY_BEGIN_HOUR = 7;             // 朝が始まる時間(時)
const int NIGHT_BEGIN_HOUR = 18;          // 夜が始まる時間(時)
const int DAY_MEASURE_INTERVAL = 60ull;   // 日中の測定間隔(分)
const int NIGHT_MEASURE_INTERVAL = 15ull; // 夜間の測定間隔(分)

// デバッグ設定
#define DEBUG (1)
