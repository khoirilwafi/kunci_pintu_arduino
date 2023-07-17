void config_loop(void)
{
    if(Serial2.available())
    {
        String bt_message = Serial2.readStringUntil('\n');
        bt_message.trim();

        // cek format pesan
        if(bt_message.startsWith("{") && bt_message.endsWith("}"))
        { 
            JSONVar bt_package = JSON.parse(bt_message);

            // cek event
            if(bt_package.hasOwnProperty("event"))
            {
                // event update data wifi
                if(strcmp(bt_package["event"], "wifi-update") == 0)
                {
                    system_log("WIFI", "update parameter");
                    
                    String ssid = (const char*) bt_package["data"]["ssid"];
                    String password = (const char*) bt_package["data"]["password"];

                    // update wifi
                    eeprom_write(wifi_ssid_addr, ssid);
                    eeprom_write(wifi_pass_addr, password);

                    WiFi.disconnect();
                    delay(500);
                    WiFi.begin(eeprom_read(wifi_ssid_addr).c_str(), eeprom_read(wifi_pass_addr).c_str());

                    Serial2.println("{\"status\":\"success\"}");

                    system_log("WIFI", "update parameter selesai");
                }

                // event register pintu baru
                else if(strcmp(bt_package["event"], "register") == 0)
                {
                    String door_id = (const char*) bt_package["data"]["door_id"];

                    if(wifi_is_connected == false)
                    {
                        Serial2.println("{\"status\":\"wifi_not_connected\"}");
                    }
                    else
                    {
                        system_log("DOOR", "register ...");
                        digitalWrite(data_status, HIGH);

                        String register_response = post_request("/register", "id=" + door_id + "&device_name=" + WiFi.macAddress());

                        if(register_response != "error")
                        {
                            JSONVar bt_http_result = JSON.parse(register_response);

                            if(strcmp(bt_http_result["status"], "success") == 0)
                            {
                                String config_office_id   = (const char*) bt_http_result["data"]["office_id"];
                                String config_door_id     = (const char*) bt_http_result["data"]["door_id"];
                                String config_door_name   = (const char*) bt_http_result["data"]["door_name"];
                                String config_device_pass = (const char*) bt_http_result["data"]["device_pass"];
                                String config_door_key    = (const char*) bt_http_result["data"]["key"];

                                eeprom_write(office_id_addr,  config_office_id);
                                eeprom_write(door_id_addr,    config_door_id);
                                eeprom_write(door_name_addr,  config_door_name);
                                eeprom_write(login_pass_addr, config_device_pass);
                                eeprom_write(door_key_addr,   config_door_key);

                                EEPROM.write(0, 'L');
                                EEPROM.commit();

                                Serial2.println("{\"status\":\"success\"}");
                                system_log("DOOR", "register berhasil");
                            }
                            else
                            {
                                Serial2.println(register_response);
                                system_log("DOOR", "register gagal");
                            }
                        }
                        else
                        {
                            Serial2.println("{\"status\":\"request_error\"}");
                            system_log("DOOR", "register error");
                        }
                    }
                }
            }
        }
    }
}

void bt_door_command(void)
{
    if(Serial2.available())
    {
        String bt_message = Serial2.readStringUntil('\n');
        bt_message.trim();

        if(bt_message.startsWith("{") && bt_message.endsWith("}"))
        {
            JSONVar bt_command = JSON.parse(bt_message);
    
            String bt_door_id = (const char*) bt_command["data"]["door_id"];
            String bt_user_id = (const char*) bt_command["data"]["user_id"];
            String bt_key     = (const char*) bt_command["data"]["key"];
            
    
            if(strcmp(bt_command["event"], "door-unlock") == 0)
            {
                if(door_is_lock == false || door_is_open == true)
                {
                    Serial2.println("{\"status\":\"door_aleady_open\"}");
                }
                else if(bt_door_id == eeprom_read(door_id_addr) && bt_key == eeprom_read(door_key_addr))
                {
                    system_log("LOCK", "membuka pintu ...");
                    buzzer_count = 2;
                    actor_id = bt_user_id;
                    lock_open();
                    door_close_wait_time = millis();
                    Serial2.println("{\"status\":\"success\"}");
                }
                else
                {
                    Serial2.println("{\"status\":\"failed\"}");
                }
            }
        }
    }
}

