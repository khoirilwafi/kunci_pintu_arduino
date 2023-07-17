void eeprom_write(uint16_t address, String text)
{
    // hitung panjang data
    uint16_t text_length = text.length();

    // tulis data
    for (uint16_t i=0; i<text_length; i++) 
    {
        EEPROM.write(address + i, text[i]);
    }

    // tulis tanda akhir data
    EEPROM.write(address + text_length, '\0');

    // commit data
    EEPROM.commit();
}

String eeprom_read(uint16_t address)
{
    String text;
    char character;
    
    while((character = EEPROM.read(address)) != '\0') 
    {
        text += character;
        address++;
    }
    
    return text;
}

void eeprom_erase_all(void)
{
    for(uint16_t addr=0; addr<275; addr++)
    {
        EEPROM.write(addr, '\0');
    }

    EEPROM.commit();
}

