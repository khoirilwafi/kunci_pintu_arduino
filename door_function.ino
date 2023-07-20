void set_solenoid_active(bool active)
{
    if(active == true)
    {
        digitalWrite(solenoid, HIGH);
        solenoid_is_active = true;

        system_log("LOCK", "solenoid aktif (terbuka)");
    }
    else
    {
        digitalWrite(solenoid, LOW);
        solenoid_is_active = false;

        system_log("LOCK", "solenoid non-aktif (tertutup)");
    }
}

void update_door(void)
{
    system_log("HTTP", "update status pintu ...");
    digitalWrite(data_status, HIGH);

    // kirim request
    String door_status = (door_is_locked == true) ? "1" : "0";
    String status_response = post_request("/update-status", "door_id=" + eeprom_read(door_id_addr) + "&office_id=" + eeprom_read(office_id_addr) + "&socket_id=" + socket_id + "&lock_status=" + door_status + "&user_id=" + actor_id);

    if(status_response != "error")
    {
        JSONVar status_result = JSON.parse(status_response);

        // jika login ditolak
        if(status_result.hasOwnProperty("message"))
        {
            device_is_login = false;
            system_log("HTTP", "akses login ditolak");
        }

        // jika update sukses
        else if(strcmp(status_result["status"], "success") == 0)
        {
            eeprom_write(door_key_addr, status_result["data"]["key"]);
            lock_status_change = false;
            system_log("HTTP", "update status berhasil");
        }
        else
        {
            system_log("HTTP", "update status gagal");
        }
    }
    else
    {
        system_log("HTTP", "request error");
    }

    digitalWrite(data_status, LOW);
}

void get_signature(void)
{
    system_log("HTTP", "request signature ...");
    digitalWrite(data_status, HIGH);

    // kirim request
    String signature_response = post_request("/get-signature", "socket_id=" + socket_id + "&office_id=" + eeprom_read(office_id_addr) + "&channel_data={\"user_id\":\"" + eeprom_read(door_id_addr) + "\",\"user_info\":true}");

    if(signature_response != "error")
    {
        JSONVar signature_result = JSON.parse(signature_response);

        // jika login ditolak
        if(signature_result.hasOwnProperty("message"))
        {
            device_is_login = false;
            system_log("HTTP", "akses login ditolak");
        }

        // jika signature sukses
        else if(strcmp(signature_result["status"], "success") == 0)
        {
            signature = (const char*) signature_result["data"]["signature"];
            device_got_signature = true;

            system_log("HTTP", "request signature berhasil");
        }
        else
        {
            system_log("HTTP", "request signature gagal");
        }
    }
    else
    {
        system_log("HTTP", "request error");
    }

    digitalWrite(data_status, LOW);
}

void login(void)
{
    system_log("HTTP", "mencoba login ...");
    digitalWrite(data_status, HIGH);
    
    // logout semua token
    get_request("/logout");

    // login 
    String login_response = post_request("/login", "device_name=" + WiFi.macAddress() + "&device_pass=" + eeprom_read(login_pass_addr));

    if(login_response != "error")
    {
        JSONVar login_result = JSON.parse(login_response);

        // cek jika login sukses
        if(strcmp(login_result["status"], "success") == 0)
        { 
            eeprom_write(office_id_addr,  (const char*) login_result["data"]["office_id"]);
            eeprom_write(door_id_addr,    (const char*) login_result["data"]["door_id"]);
            eeprom_write(door_name_addr,  (const char*) login_result["data"]["door_name"]);
            eeprom_write(door_token_addr, (const char*) login_result["token"]);

            system_log("HTTP", "login berhasil");
            device_is_login = true;
        }
        else
        {
            system_log("HTTP", "login gagal");
        }
    }
    else
    {
        system_log("HTTP", "request error");
    }

    digitalWrite(data_status, LOW);
}

void send_alert(void)
{
    system_log("HTTP", "mengirim alert ...");
    digitalWrite(data_status, HIGH);
             
    // kirim request
    String alert_response = post_request("/alert", "office_id=" + eeprom_read(office_id_addr) + "&door_id=" + eeprom_read(door_id_addr) + "&message=" + alert_message);
    
    if(alert_response != "error")
    {
        JSONVar alert_result = JSON.parse(alert_response);

        // jika login ditolak
        if(alert_result.hasOwnProperty("message"))
        {
            device_is_login = false;
            system_log("HTTP", "akses login ditolak");
        }

        // jika signature sukses
        else if(strcmp(alert_result["status"], "success") == 0)
        {
            alert_status_change = false;
            system_log("HTTP", "alert berhasil terkirim");
        }
        else
        {
            system_log("HTTP", "alert gagal");
        }
    }
    else
    {
        system_log("HTTP", "request error");
    }

    digitalWrite(data_status, LOW);
}

void door_command(JSONVar command)
{
    system_log("LOCK", "memproses perintah ...");
    
    String door_id = (const char*)command["door_id"];
    String locking = (const char*)command["locking"];
    String user_id = (const char*)command["user_id"];
    String key     = (const char*)command["key"];
    
    // cek apakah perintah untuk pintu ini
    if(door_id == eeprom_read(door_id_addr) && key == eeprom_read(door_key_addr))
    {
        actor_id = user_id;
        
        if(locking == "open")
        {
            door_is_locked = false;
            lock_status_change = true;
        }
        else if(locking == "lock")
        {
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
        }

        buzzer_count = 2;
    }
}

uint8_t get_time_integer(String time_string, uint8_t index)
{
    uint8_t time_integer[3] = {0, 0, 0};
    
    uint8_t first_separator = time_string.indexOf(":");
    uint8_t last_separator  = time_string.lastIndexOf(":");

    time_integer[0] = time_string.substring(0, first_separator).toInt();
    time_integer[1] = time_string.substring(first_separator+1, last_separator).toInt();
    time_integer[2] = time_string.substring(last_separator+1).toInt();

    return time_integer[index];
}

void door_schedule(JSONVar schedule)
{
    system_log("LOCK", "memproses jadwal ...");

    String door_id = (const char*)schedule["door_id"];
    String command = (const char*)schedule["status"];
    String user_id = (const char*)schedule["user_id"];
    String key     = (const char*)schedule["key"];

    // jika jadwal untuk pintu ini
    if(door_id == eeprom_read(door_id_addr) && key == eeprom_read(door_key_addr))
    {
        // jika perintah jadwal berjalan
        if(command == "run")
        {
            String server_time  = (const char*) schedule["time_now"];
            String schedule_end = (const char*) schedule["time_end"];

            timeout_hour   = get_time_integer(schedule_end, 0);
            timeout_minute = get_time_integer(schedule_end, 1);
            timeout_second = get_time_integer(schedule_end, 2);

            rtc.setTime(get_time_integer(server_time, 2), get_time_integer(server_time, 1), get_time_integer(server_time, 0), 21, 5, 2023);

            // buka kunci pintu
            door_is_locked = false;
            lock_status_change = true;
            
            // jalankan penjadwalan
            schedule_is_running = true;

            system_log("LOCK", "jadwal dimulai, kunci pintu terbuka");
        }
        else if(command == "stop")
        {
            // kunci pintu
            door_is_locked = false;
            lock_status_change = true;
            
            schedule_is_running = false;

            system_log("LOCK", "jadwal dibatalkan, mengunci pintu ...");
        }

        buzzer_count = 3;
    }
}

void unlink(JSONVar door_data)
{
    String id  = (const char*) door_data["data"]["door_id"];
    String key = (const char*) door_data["data"]["key"];

    if(id == eeprom_read(door_id_addr) && key == eeprom_read(door_key_addr))
    {
        system_log("DOOR", "unlink pintu ...");
        
        EEPROM.write(0, 'U');
        EEPROM.commit();

        ESP.restart();
    }
}

void door_register(String door_id)
{
    system_log("HTTP", "register ...");
    
    String register_response = post_request("/register", "id=" + door_id + "&device_name=" + WiFi.macAddress());

    if(register_response != "error")
    {
        JSONVar register_result = JSON.parse(register_response);

        if(strcmp(register_result["status"], "success") == 0)
        {
            system_log("DOOR", "register berhasil");

            String config_office_id   = (const char*) register_result["data"]["office_id"];
            String config_door_id     = (const char*) register_result["data"]["door_id"];
            String config_door_name   = (const char*) register_result["data"]["door_name"];
            String config_device_pass = (const char*) register_result["data"]["device_pass"];
            String config_door_key    = (const char*) register_result["data"]["key"];

            eeprom_write(office_id_addr,  config_office_id);
            eeprom_write(door_id_addr,    config_door_id);
            eeprom_write(door_name_addr,  config_door_name);
            eeprom_write(login_pass_addr, config_device_pass);
            eeprom_write(door_key_addr,   config_door_key);

            EEPROM.write(0, 'L');
            EEPROM.commit();
        }
        else
        {
            system_log("DOOR", "register gagal");
        }
    }
    else
    {
        system_log("HTTP", "request error");
    }
}

