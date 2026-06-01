idf_component_register(SRCS "main.c" "mpu6050_app.c" "power_app.c" "buzzer_app.c" "display_app.c" "st7735.c" "mqtt_app.c"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES driver esp_wifi nvs_flash esp_event mqtt esp-tls
                    EMBED_TXTFILES "certs/isrgrootx1.pem")