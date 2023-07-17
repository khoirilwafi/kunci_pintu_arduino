void http_client_task(void* pvParameter)
{
    while(1)
    {
        // semua request membutuhkan koneksi dari wifi
        if(wifi_is_connected == true)
        {       
            // request untuk login
            if(device_is_login == false)
            {
                login();
            }

            // semua proses request yang memerlukan akses login
            else if(device_is_login == true)
            {
                // request untuk mendapatkan signature pusher
                if(socket_is_connected == true && device_got_signature == false)
                {
                    get_signature();
                }

                // request untuk update status pintu ke serve
                if(socket_is_connected == true && device_is_subscribe == true && lock_status_change == true)
                {
                    update_door();
                }

                // request untuk mengirinkan alert
                if(alert_status_change == true)
                {
                    send_alert();
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

