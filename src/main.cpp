#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <LittleFS.h>
#include <WiFi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/ledc.h>
#include <ctype.h>

#include "config.h"
#include "scan.h"
#include "pcap_stream.h"
#include "session_pcap.h"
#include "text_summary.h"
#include "web_dashboard.h"

namespace {

constexpr uint8_t  PIN_BUZZER   = 3;
constexpr uint8_t  PIN_NEOPIXEL = 21;
constexpr uint8_t  PIN_BOOT     = 0;
constexpr ledc_channel_t BUZZER_CH    = LEDC_CHANNEL_0;
constexpr ledc_timer_t   BUZZER_TIMER = LEDC_TIMER_0;

Adafruit_NeoPixel pixel(1, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

enum class Fault { None, LittleFS, Wifi, Scan };
volatile Fault g_fault = Fault::None;

TaskHandle_t pcap_task_h = nullptr;
TaskHandle_t led_task_h  = nullptr;

volatile uint32_t last_advert_ms = 0;

void buzzer_setup() {
    ledc_timer_config_t t = {};
    t.speed_mode      = LEDC_LOW_SPEED_MODE;
    t.duty_resolution = LEDC_TIMER_10_BIT;
    t.timer_num       = BUZZER_TIMER;
    t.freq_hz         = 1500;
    t.clk_cfg         = LEDC_AUTO_CLK;
    ledc_timer_config(&t);

    ledc_channel_config_t c = {};
    c.gpio_num   = PIN_BUZZER;
    c.speed_mode = LEDC_LOW_SPEED_MODE;
    c.channel    = BUZZER_CH;
    c.timer_sel  = BUZZER_TIMER;
    c.duty       = 0;
    c.hpoint     = 0;
    ledc_channel_config(&c);
}

void buzzer_chirp(uint16_t freq, uint16_t ms) {
    ledc_set_freq(LEDC_LOW_SPEED_MODE, BUZZER_TIMER, freq);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, BUZZER_CH, 512);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, BUZZER_CH);
    delay(ms);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, BUZZER_CH, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, BUZZER_CH);
}

void led_task(void*) {
    uint32_t last_pkt_seen = 0;
    uint32_t amber_until = 0;
    for (;;) {
        uint32_t now = millis();
        uint32_t r = 0, g = 0, b = 0;

        if (g_fault != Fault::None) {
            r = 40; g = 0; b = 0;
        } else {
            // Slow blue pulse — sniffer alive, scanning passively.
            float phase = (now % 2500) / 2500.0f;
            float pulse = 0.3f + 0.3f * sinf(phase * 6.2831853f);
            b = (uint8_t)(35 * pulse);
        }

        uint32_t pkt_ms = last_advert_ms;
        if (pkt_ms != last_pkt_seen) {
            last_pkt_seen = pkt_ms;
            if (now - amber_until > 60) amber_until = now + 20;
        }
        if (now < amber_until) {
            r = 30; g = 20; b = 0;
        }

        pixel.setPixelColor(0, pixel.Color(r, g, b));
        pixel.show();
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void pcap_writer_task(void*) {
    for (;;) {
        scan::Frame f;
        int drained = 0;
        while (drained < 16 && scan::pop_pcap(&f)) {
            last_advert_ms = millis();
            if (config::get().out_mode == config::OUT_PCAP) {
                pcap_stream::ensure_header_for_current_mode();
                pcap_stream::write_frame_pcap(f);
            } else {
                pcap_stream::write_frame_text(f);
            }
            session_pcap::append(f);
            drained++;
        }
        vTaskDelay(pdMS_TO_TICKS(drained ? 2 : 10));
    }
}

void print_banner() {
    Serial.println();
    Serial.println(F(" ██████╗ ██╗   ██╗██╗      ███████╗██████╗ ██╗   ██╗    ██████╗ ██╗     ███████╗███████╗███╗   ██╗██╗███████╗███████╗"));
    Serial.println(F("██╔═══██╗██║   ██║██║      ██╔════╝██╔══██╗╚██╗ ██╔╝    ██╔══██╗██║     ██╔════╝██╔════╝████╗  ██║██║██╔════╝██╔════╝"));
    Serial.println(F("██║   ██║██║   ██║██║█████╗███████╗██████╔╝ ╚████╔╝     ██████╔╝██║     █████╗  ███████╗██╔██╗ ██║██║█████╗  █████╗  "));
    Serial.println(F("██║   ██║██║   ██║██║╚════╝╚════██║██╔═══╝   ╚██╔╝      ██╔══██╗██║     ██╔══╝  ╚════██║██║╚██╗██║██║██╔══╝  ██╔══╝  "));
    Serial.println(F("╚██████╔╝╚██████╔╝██║      ███████║██║        ██║       ██████╔╝███████╗███████╗███████║██║ ╚████║██║██║     ██║     "));
    Serial.println(F(" ╚═════╝  ╚═════╝ ╚═╝      ╚══════╝╚═╝        ╚═╝       ╚═════╝ ╚══════╝╚══════╝╚══════╝╚═╝  ╚═══╝╚═╝╚═╝     ╚═╝     "));
    Serial.print(F("OUI-SPY BLESNIFF  fw="));
    Serial.print(config::FW_VERSION());
    Serial.print(F("  built="));
    Serial.print(F(__DATE__ " " __TIME__));
    Serial.println();
    Serial.println(F("Passive receive only. Nothing is transmitted."));
    Serial.println();
}

String upper(const String& s) { String o = s; o.toUpperCase(); return o; }

void reply_ok()               { if (config::get().out_mode == config::OUT_TEXT) Serial.println(F("OK")); }
void reply_err(const char* m) { if (config::get().out_mode == config::OUT_TEXT) { Serial.print(F("ERR ")); Serial.println(m); } }

void handle_serial_cmd(const String& raw) {
    String line = raw; line.trim();
    if (!line.startsWith("CMD:") && !line.startsWith("cmd:")) return;
    String body = line.substring(4);
    body.trim();
    String U = upper(body);

    if (U == "STATUS") {
        if (config::get().out_mode != config::OUT_TEXT) return;
        IPAddress ip = WiFi.softAPIP();
        String apmac = WiFi.softAPmacAddress();
        Serial.printf("{\"out\":\"%s\",\"scan_win\":%u,\"scan_int\":%u,\"ftmask\":\"0x%02x\","
            "\"total\":%u,\"pps\":%u,\"drop_pcap\":%u,\"drop_dash\":%u,\"fw\":\"%s\","
            "\"ap_ssid\":\"%s\",\"ap_ip\":\"%s\",\"ap_mac\":\"%s\",\"ap_stations\":%u,"
            "\"session_bytes\":%u}\n",
            config::get().out_mode == config::OUT_PCAP ? "PCAP" : "TEXT",
            (unsigned)config::get().scan_window_ms,
            (unsigned)config::get().scan_interval_ms,
            (unsigned)config::get().ft_mask,
            (unsigned)scan::total_adverts(),
            (unsigned)scan::adverts_per_sec(),
            (unsigned)scan::dropped_pcap(),
            (unsigned)scan::dropped_dash(),
            config::FW_VERSION(),
            config::get().ap_ssid, ip.toString().c_str(), apmac.c_str(),
            (unsigned)WiFi.softAPgetStationNum(),
            (unsigned)session_pcap::size());
        return;
    }
    if (U == "VERSION") {
        if (config::get().out_mode != config::OUT_TEXT) return;
        Serial.printf("OUI-SPY BLESNIFF %s built %s %s\n", config::FW_VERSION(), __DATE__, __TIME__);
        return;
    }
    if (U.startsWith("MODE ")) {
        String v = U.substring(5); v.trim();
        if (v == "PCAP") { config::set_out(config::OUT_PCAP); pcap_stream::on_mode_changed(); reply_ok(); return; }
        if (v == "TEXT") { config::set_out(config::OUT_TEXT); pcap_stream::on_mode_changed(); reply_ok(); return; }
        reply_err("bad mode"); return;
    }
    if (U.startsWith("WINDOW ")) {
        int v = U.substring(7).toInt();
        if (v < 10 || v > 2000) { reply_err("bad window"); return; }
        config::set_scan_window((uint16_t)v);
        scan::apply_scan_params();
        reply_ok(); return;
    }
    if (U.startsWith("INTERVAL ")) {
        int v = U.substring(9).toInt();
        if (v < 20 || v > 4000) { reply_err("bad interval"); return; }
        config::set_scan_interval((uint16_t)v);
        scan::apply_scan_params();
        reply_ok(); return;
    }
    reply_err("unknown");
}

void serial_pump() {
    static String line;
    while (Serial.available()) {
        int c = Serial.read();
        if (c < 0) break;
        if (c == '\n' || c == '\r') {
            if (line.length()) { handle_serial_cmd(line); line = ""; }
        } else {
            if (line.length() < 200) line += (char)c;
        }
    }
}

void boot_button_poll() {
    static uint32_t held_since = 0;
    if (digitalRead(PIN_BOOT) == LOW) {
        if (held_since == 0) held_since = millis();
        else if (millis() - held_since > 1500) {
            config::reset_defaults();
            buzzer_chirp(1500, 60);
            delay(60);
            buzzer_chirp(1000, 60);
            delay(200);
            ESP.restart();
        }
    } else {
        held_since = 0;
    }
}

bool wifi_ap_start() {
    WiFi.mode(WIFI_AP);
    // BLE + Wi-Fi coexistence: NimBLE brings up the BT controller which
    // shares the 2.4 GHz radio. AsyncWebServer + softAP work fine alongside
    // NimBLE scan on ESP32-S3 as long as we don't pin either to an aggressive
    // channel. Channel 1 default is a reasonable choice.
    return WiFi.softAP(config::get().ap_ssid, config::get().ap_pass, 1, 0, 4);
}

} // namespace

void setup() {
    Serial.begin(115200);
    pinMode(PIN_BOOT, INPUT_PULLUP);
    pixel.begin();
    pixel.setPixelColor(0, 0);
    pixel.show();

    buzzer_setup();

    if (!LittleFS.begin(true)) {
        g_fault = Fault::LittleFS;
    }

    config::load();

    if (!wifi_ap_start()) {
        g_fault = Fault::Wifi;
    }

    session_pcap::init();
    web_dashboard::init();

    if (!scan::init()) {
        g_fault = Fault::Scan;
    }

    pcap_stream::begin();

    // pcap_writer_task copies scan::Frame (~280B) into a stack local per pop;
    // 8KB is comfortable headroom.
    xTaskCreatePinnedToCore(&pcap_writer_task, "pcap_wr", 8192, nullptr, 5, &pcap_task_h, 0);
    xTaskCreatePinnedToCore(&led_task,         "led",     2048, nullptr, 1, &led_task_h,  0);

    if (g_fault == Fault::None) buzzer_chirp(1500, 40);

    if (config::get().out_mode == config::OUT_TEXT) print_banner();
}

void loop() {
    serial_pump();
    boot_button_poll();
    web_dashboard::tick();
    delay(20);
}
