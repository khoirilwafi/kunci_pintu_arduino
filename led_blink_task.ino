void led_blink_task(void* pvParameters)
{
    while(1)
    {
        if(wifi_is_connected == false)
        {
            digitalWrite(wifi_status, !digitalRead(wifi_status));
        }
        else
        {
            digitalWrite(wifi_status, HIGH);
        }

        if(door_is_locked == false)
        {
            digitalWrite(lock_status, !digitalRead(lock_status));
        }
        else
        {
            digitalWrite(lock_status, HIGH);
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
