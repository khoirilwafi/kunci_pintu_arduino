void websocket_event(WebsocketsEvent event, String data) 
{
    if(event == WebsocketsEvent::ConnectionClosed) 
    {
        system_log("SOCK", "terputus");
        socket_is_connected = false;
    }
}

void websocket_message(WebsocketsMessage websocket_message) 
{
    // ubah pesan ke bentuk json
    JSONVar message = JSON.parse(websocket_message.data());

    // event koneksi pusher berhasil
    if(strcmp(message["event"], "pusher:connection_established") == 0)
    {
        // update nilai socket_id
        JSONVar pusher_data = JSON.parse((const char*) message["data"]);
        socket_id = (const char*) pusher_data["socket_id"];

        // update status
        system_log("SOCK", "websocket terhubung");
        socket_is_connected = true;
    }

    // subscribe berhasil
    else if(strcmp(message["event"], "pusher_internal:subscription_succeeded") == 0)
    {
        system_log("SOCK", "subscribe berhasil");
        device_is_subscribe = true;
    }

    // perintah door-command diterima
    else if(strcmp(message["event"], "door-command") == 0)
    {
        system_log("DOOR", "perintah diterima");
        JSONVar command = JSON.parse((const char*) message["data"]);
        door_command(command);
    }

    // perintah door-schedule diterima
    else if(strcmp(message["event"], "door-schedule") == 0)
    {
        system_log("DOOR", "jadwal diterima");
        JSONVar schedule = JSON.parse((const char*) message["data"]);
        door_schedule(schedule);
    }

    else if(strcmp(message["event"], "door-unlink") == 0)
    {
        JSONVar unlink_data = JSON.parse((const char*) message["data"]);
        unlink(unlink_data);
    }
    
}

