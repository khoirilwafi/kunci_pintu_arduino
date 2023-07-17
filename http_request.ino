String get_request(String endpoint)
{
    if(wifi_is_connected == false)
    {
        return "error";
    }

    // inisialisasi data dan objek
    HTTPClient http_get;
    String get_response;

    // mulai koneksi
    digitalWrite(data_status, HIGH);
    http_get.begin(server_url + endpoint, server_root_ca_cert);

    // kirimkan request
    http_get.addHeader("Authorization", "Bearer " + eeprom_read(door_token_addr));
    http_get.addHeader("Accept", "application/json");
    int get_code = http_get.GET();

    // olah respon
    if (get_code > 0) 
    {
        get_response = http_get.getString();
    } 
    else 
    {
        get_response = "error";
    }

    // request selesai
    http_get.end();
    digitalWrite(data_status, LOW);

    return get_response;
}

String post_request(String endpoint, String payload)
{
    HTTPClient http_post;
    String post_response;

    digitalWrite(data_status, HIGH);
    http_post.begin(server_url + endpoint, server_root_ca_cert);

    // kirimkan request
    http_post.addHeader("Authorization", "Bearer " + eeprom_read(door_token_addr));
    http_post.addHeader("Content-Type", "application/x-www-form-urlencoded");
    http_post.addHeader("Accept", "application/json");
    int post_code = http_post.POST(payload);

    if (post_code > 0) 
    {
        post_response = http_post.getString();
    } 
    else 
    {
        post_response = "error";
    }

    http_post.end();
    digitalWrite(data_status, LOW);

    return post_response;
}

