#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

float n = 0;

void setup()
{
    Serial.begin(115200);
    Wire.begin(5, 6);

    lcd.init();
    lcd.backlight();
    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("Duo Duo Star >_<");
}

void loop()
{
    n += 0.25;
    lcd.setCursor(0, 1);
    lcd.print("N: ");
    lcd.print(n, 2);
}