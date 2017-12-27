/*
 * Project Particle_RdWebServerPostTest
 * Description:
 * Author:
 * Date:
 */

// Web server
#include "Grove_LCD_RGB_Backlight.h"
#include "LocalServer.h"

STARTUP(WiFi.selectAntenna(ANT_EXTERNAL));
SYSTEM_THREAD(ENABLED);

rgb_lcd lcd;
LocalServer localServer;

void setup() {
    Serial.begin(9600);
    delay(3000);

    localServer.setup();

    lcd.begin(16, 2);
    lcd.setRGB(100, 100, 100);
    lcd.print(WiFi.localIP());

    Serial.println("...Started...");
}

void loop() {
    localServer.service();
}
