#include <WiFi.h>
#include <EEPROM.h>
#include <ESP32Time.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <ArduinoWebsockets.h>

// namespace dan objek websocket
using namespace websockets;
WebsocketsClient client;

// objek untuk time
ESP32Time rtc;

// led pin
#define wifi_status 13
#define data_status 14
#define lock_status 27

// sensor dan aktuator
#define button      26
#define buzzer      32
#define solenoid    25
#define sensor      33   

// alamat peyimpanan eeprom
#define device_status_addr  0
#define wifi_ssid_addr      1
#define wifi_pass_addr      22      
#define login_pass_addr     43
#define office_id_addr      64
#define door_id_addr        101
#define door_name_addr      138
#define door_key_addr       189
#define door_token_addr     210

// ssl websocket
const char server_root_ca_cert[] PROGMEM = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIFFjCCAv6gAwIBAgIRAJErCErPDBinU/bWLiWnX1owDQYJKoZIhvcNAQELBQAw\n" \
"TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh\n" \
"cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMjAwOTA0MDAwMDAw\n" \
"WhcNMjUwOTE1MTYwMDAwWjAyMQswCQYDVQQGEwJVUzEWMBQGA1UEChMNTGV0J3Mg\n" \
"RW5jcnlwdDELMAkGA1UEAxMCUjMwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEK\n" \
"AoIBAQC7AhUozPaglNMPEuyNVZLD+ILxmaZ6QoinXSaqtSu5xUyxr45r+XXIo9cP\n" \
"R5QUVTVXjJ6oojkZ9YI8QqlObvU7wy7bjcCwXPNZOOftz2nwWgsbvsCUJCWH+jdx\n" \
"sxPnHKzhm+/b5DtFUkWWqcFTzjTIUu61ru2P3mBw4qVUq7ZtDpelQDRrK9O8Zutm\n" \
"NHz6a4uPVymZ+DAXXbpyb/uBxa3Shlg9F8fnCbvxK/eG3MHacV3URuPMrSXBiLxg\n" \
"Z3Vms/EY96Jc5lP/Ooi2R6X/ExjqmAl3P51T+c8B5fWmcBcUr2Ok/5mzk53cU6cG\n" \
"/kiFHaFpriV1uxPMUgP17VGhi9sVAgMBAAGjggEIMIIBBDAOBgNVHQ8BAf8EBAMC\n" \
"AYYwHQYDVR0lBBYwFAYIKwYBBQUHAwIGCCsGAQUFBwMBMBIGA1UdEwEB/wQIMAYB\n" \
"Af8CAQAwHQYDVR0OBBYEFBQusxe3WFbLrlAJQOYfr52LFMLGMB8GA1UdIwQYMBaA\n" \
"FHm0WeZ7tuXkAXOACIjIGlj26ZtuMDIGCCsGAQUFBwEBBCYwJDAiBggrBgEFBQcw\n" \
"AoYWaHR0cDovL3gxLmkubGVuY3Iub3JnLzAnBgNVHR8EIDAeMBygGqAYhhZodHRw\n" \
"Oi8veDEuYy5sZW5jci5vcmcvMCIGA1UdIAQbMBkwCAYGZ4EMAQIBMA0GCysGAQQB\n" \
"gt8TAQEBMA0GCSqGSIb3DQEBCwUAA4ICAQCFyk5HPqP3hUSFvNVneLKYY611TR6W\n" \
"PTNlclQtgaDqw+34IL9fzLdwALduO/ZelN7kIJ+m74uyA+eitRY8kc607TkC53wl\n" \
"ikfmZW4/RvTZ8M6UK+5UzhK8jCdLuMGYL6KvzXGRSgi3yLgjewQtCPkIVz6D2QQz\n" \
"CkcheAmCJ8MqyJu5zlzyZMjAvnnAT45tRAxekrsu94sQ4egdRCnbWSDtY7kh+BIm\n" \
"lJNXoB1lBMEKIq4QDUOXoRgffuDghje1WrG9ML+Hbisq/yFOGwXD9RiX8F6sw6W4\n" \
"avAuvDszue5L3sz85K+EC4Y/wFVDNvZo4TYXao6Z0f+lQKc0t8DQYzk1OXVu8rp2\n" \
"yJMC6alLbBfODALZvYH7n7do1AZls4I9d1P4jnkDrQoxB3UqQ9hVl3LEKQ73xF1O\n" \
"yK5GhDDX8oVfGKF5u+decIsH4YaTw7mP3GFxJSqv3+0lUFJoi5Lc5da149p90Ids\n" \
"hCExroL1+7mryIkXPeFM5TgO9r0rvZaBFOvV2z0gp35Z0+L4WPlbuEjN/lxPFin+\n" \
"HlUjr8gRsI3qfJOQFy/9rKIJR0Y/8Omwt/8oTWgy1mdeHmmjk7j1nYsvC9JSQ6Zv\n" \
"MldlTTKB3zhThV1+XWYp6rjd5JW1zbVWEkLNxE7GJThEUG3szgBVGP7pSWTUTsqX\n" \
"nLRbwHOoq7hHwg==\n" \
"-----END CERTIFICATE-----\n";


// alamat server 
String server_url = "https://smartdoorlock.my.id/door";
String socket_url = "wss://door.smartdoorlock.my.id/app/aNmB0bkbrE1PS6K07nrt";

// notifikasi buzzer
uint8_t buzzer_count = 0;

// timeout untuk schedule
uint8_t timeout_hour   = 0;
uint8_t timeout_minute = 0;
uint8_t timeout_second = 0;

// tick interval untuk schedule
uint32_t schedule_check_interval = 0;

// tick interval untuk socket
uint32_t socket_action_interval = 0;
uint32_t socket_ping_interval = 0;

// tick interval untuk tombol
uint32_t button_pressed_time = 0;

// tick interval untuk door lock
uint32_t door_timeout = 0;
uint32_t waiting_door_interval = 0;

// data untuk touch
uint32_t touch_value = 0;
uint16_t touch_avg = 0;
uint8_t touch_interval = 0;

// websocket data
String socket_id = "";
String signature = "";

// data aktor sekarang
String actor_id  = "";

// data pesan alert
String alert_message = "";

// event status untuk wifi
bool wifi_is_connected = false;

// event status untuk client serve
bool device_is_login      = true;
bool socket_is_connected  = false;
bool device_is_subscribe  = false;
bool device_got_signature = false;

// event status untuk door lock
bool door_is_locked      = true;
bool solenoid_is_active  = false;
bool door_is_closed      = true;
bool lock_status_change  = false;
bool alert_status_change = false;
bool waiting_door_close  = false;
bool waiting_timeout     = false;

// event status untuk tombol
bool button_is_pressed = false;

// event status untuk schedule
bool schedule_is_running = false;

void setup(void)
{
    // serial config
    Serial.begin(115200);
    Serial2.begin(9600);
    delay(500);

    system_log("SYST", "mengatur peripheral ...");

    // siapkan penyimpanan eeprom
    EEPROM.begin(275);

    // konfigurasi led
    pinMode(wifi_status, OUTPUT);
    pinMode(data_status, OUTPUT);
    pinMode(lock_status, OUTPUT);

    // konfigurasi ouput
    pinMode(buzzer,     OUTPUT);
    pinMode(solenoid,   OUTPUT);

    // konfigurasi pin input
    pinMode(button, INPUT_PULLUP);
    pinMode(sensor, INPUT_PULLUP);

    // ambil aktor
    actor_id = eeprom_read(door_id_addr);

    system_log("SYST", "selesai");
    system_log("TASK", "membuat task untuk peripheral ...");

    // daftarkan task
    xTaskCreate(led_blink_task, "led_blink_task", 550, NULL, 10, NULL);
    xTaskCreate(buzzer_alert_task, "buzzer_alert_task", 600, NULL, 12, NULL);

    system_log("TASK", "selesai");
    system_log("WIFI", "memulai koneksi ...");
    
    // hapus konfigurasi lama dan daftarkan event handler
    WiFi.disconnect(true);
    WiFi.onEvent(wifi_event);

    buzzer_count = 3;

    // hubungkan ke akses point
    WiFi.begin(eeprom_read(wifi_ssid_addr).c_str(), eeprom_read(wifi_pass_addr).c_str());

    // jika tombol ditekan saat booting, masuk mode konfigurasi
    if(digitalRead(button) == LOW)
    {
        system_log("SYST", "memulai mode config ...");

        while(1)
        {
            digitalWrite(wifi_status, LOW);
            digitalWrite(data_status, LOW);
            digitalWrite(lock_status, LOW);
            
            config_loop();
            
            delay(5);
        }
    }

    // jika perangkat ini belum didaftarkan ke server, loop blink 
    else if(EEPROM.read(device_status_addr) == 'U')
    {
        while(1)
        {
            digitalWrite(wifi_status, HIGH);
            digitalWrite(data_status, HIGH);
            digitalWrite(lock_status, HIGH);
            delay(500);
            digitalWrite(wifi_status, LOW);
            digitalWrite(data_status, LOW);
            digitalWrite(lock_status, LOW);
            delay(500);
        }
    }
 
    system_log("SOCK", "mengatur event ...");

    // websocket handler
    client.onMessage(websocket_message);
    client.onEvent(websocket_event);

    // sertifikat untuk ssl
    client.setCACert(server_root_ca_cert);

    system_log("SOCK", "selesai");
    system_log("TASK", "membuat task untuk http ...");
    xTaskCreate(http_client_task, "http_client_task", 10000, NULL, 20, NULL);
    system_log("TASK", "selesai");
}

void loop(void)
{
    // websocket pooling
    if(client.available()) 
    {
        client.poll();
    }

    // bluetooth polling
    if(Serial2.available())
    {
        bt_door_command();
    }

    // semua proses yang membutuhkan koneksi wifi
    if(wifi_is_connected == true)
    {
        // konek ke webbsocket server
        if(socket_is_connected == false && (millis() - socket_action_interval) > 3000)
        {
            system_log("SOCK", "menghubungkan ...");
            digitalWrite(data_status, HIGH);
    
            // hubungkan ke server websocket
            client.close();
            client.connect(socket_url);
    
            socket_action_interval = millis();
            digitalWrite(data_status, LOW);
        }

        // subscribe ke channel
        if(socket_is_connected == true && device_got_signature == true && device_is_subscribe == false && (millis() - socket_action_interval) > 3000)
        {
            system_log("SOCK", "mencoba subscribe ...");
            digitalWrite(data_status, HIGH);

            // kirim pesan subscribe
            client.send("{\"event\":\"pusher:subscribe\",\"data\":{\"auth\":\"aNmB0bkbrE1PS6K07nrt:" + signature + "\",\"channel_data\":\"{\\\"user_id\\\":\\\"" + eeprom_read(door_id_addr) + "\\\",\\\"user_info\\\":true}\",\"channel\":\"presence-office." + eeprom_read(office_id_addr) + "\"}}");
        
            socket_action_interval = millis();
            digitalWrite(data_status, LOW);
        }

        // ping ke websocket
        if(socket_is_connected == true && (millis() - socket_ping_interval) > 30000)
        {
            digitalWrite(data_status, HIGH);
            
            client.send("{\"event\":\"pusher:ping\",\"data\":{}}");
            socket_ping_interval = millis();

            digitalWrite(data_status, LOW);
        }
    }

    // -----------------------------------------------------------
    // mekanisme penguncian
    // -----------------------------------------------------------


    // cek kondisi pintu terbuka atau tidak
    if(digitalRead(sensor) == 0)
    {
        door_is_closed = true;
        waiting_door_close = false;
    }
    else
    {  
        door_is_closed = false;
    }


    // cek apakah tombol (open) ditekan
    if(digitalRead(button) == LOW && button_is_pressed == false && waiting_door_close == false && door_is_closed == true)
    {
        actor_id = eeprom_read(door_id_addr);
        
        button_is_pressed = true;
        button_pressed_time = millis();
    }

    // jika tombol dilepas 
    if(digitalRead(button) == HIGH && button_is_pressed == true)
    {
        // jika tombol ditekan lama (lebih dari 700 ms)
        if((millis() - button_pressed_time) > 700)
        {
            // no function
        }

        // jika tombol ditekan cepat
        else
        {
            set_solenoid_active(true);

            buzzer_count = 2;
            waiting_timeout = true;
            door_timeout = millis();
        }

        button_is_pressed = false;
    }

    // jika pintu dibuka dan solenoid aktif
    if(door_is_closed == false && solenoid_is_active == true)
    {
        set_solenoid_active(false);
    }

    // jika sudah timeout
    if((millis() - door_timeout) > 10000 && waiting_timeout == true)
    {
        // matikan solenoid
        if(solenoid_is_active == true)
        {
            set_solenoid_active(false);
            buzzer_count = 2;
        }

        // jika pintu masih terbuka
        if(door_is_locked == true && door_is_closed == false)
        {
            waiting_door_close = true;
            
            // update status pintu terbuka
            door_is_locked = false;
            lock_status_change = true;
        }

        waiting_timeout = false;
    }

    // jika sedang menunggu pintu tertutup
    if(waiting_door_close == true && (millis() - waiting_door_interval) > 5000)
    {
        buzzer_count = 4;
        waiting_door_interval = millis();
    }


    // polling sensor sentuh
    touch_value += touchRead(4);

    // hitung nilai rata-rata sensor dari 20x pembacaan
    if(touch_interval < 20)
    {
        touch_interval ++;
    }
    else
    {
        touch_avg = touch_value / 20;

        touch_interval = 0;
        touch_value = 0;
    }

    // jika sensor disentuh 
    if((touch_avg <= 25) && waiting_door_close == false && door_is_closed == true && waiting_timeout == false && door_is_locked == false)
    {
        set_solenoid_active(true);

        buzzer_count = 2;
        waiting_timeout = true;
        door_timeout = millis();
    }


    // cek penjadwalan
    if(schedule_is_running == true && (millis() - schedule_check_interval) > 1000)
    {
        // jika jadwal sudah berakhir
        if(rtc.getHour(true) >= timeout_hour && rtc.getMinute() >= timeout_minute && rtc.getSecond() >= timeout_second)
        {
            system_log("LOCK", "jadwal berakhir, mengunci pintu ...");

            if(door_is_closed == false)
            {
                alert_message = "Pintu masih terbuka, menunggu pintu tertutup ...";
                alert_status_change = true;
            }
            else
            {
                door_is_locked = true; 
                lock_status_change = true;  
            }

            schedule_is_running = false;
            buzzer_count = 2;
        }

        schedule_check_interval = millis();
    }


    // cek jika pintu terbuka secara paksa
    if(door_is_closed == false && solenoid_is_active == false && door_is_locked == true && waiting_timeout == false)
    {
        // kirim peringatan
        alert_message = "Pintu terbuka tanpa autentikasi yang sah, Menunggu pintu ditutup ...";
        alert_status_change = true;
        
        // update status pintu terbuka
        door_is_locked = false;
        lock_status_change = true;

        waiting_door_close = true;
    }


    // delay untuk kestabilan
    delay(5);
}

