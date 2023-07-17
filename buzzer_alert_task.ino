void buzzer_alert_task(void* pvParameter)
{
    while(1)
    {
        for(uint8_t i=0; i<buzzer_count; i++)
        {
            digitalWrite(buzzer, HIGH);
            delay(150);
            digitalWrite(buzzer, LOW);
            delay(150);
        }

        buzzer_count = 0;
    
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

