#include <SPI.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WidgetRTC.h>
#include <Wire.h>
#include <ESP8266mDNS.h> // For OTA
#include <WiFiUdp.h>     // For OTA
#include <ArduinoOTA.h>  // For OTA
#define BLYNK_PRINT Serial
#define switch1 D6
#define switch2 D5
#define switch3 D0
char auth[] = "Uct_G4dtFCnAa3OYZAs3OohGvxcEp2NS";
char ssid[] = "Tenda";
char pass[] = "kamil2k19";
const int oneWireBus = 2;
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);
char currentTime[9];
char startTime[9];
char stopTime[9];
int SThour, STmin, STsec, SPhour, SPmin, SPsec;
int godz;
bool stan = 1;      //1 czyli wyłączona
bool stanpompa = 1; //1 czyli wyłączona
float t1;
float t;
bool lock = 1;  // ustabilizowanie minuty do jednej czynności na minutę
bool lock1 = 1; //alarm lock
bool lock2 = 1;
bool lock3 = 1;           // ustabilizowanie minuty do jednej czynności na minutę
bool lock4 = 0;           //CWU off
bool lock5 = 0;           //CWU on
bool lock6, lock7, lock8; //wymuszenie włączenia pieca
bool onoff = 0;
bool lockalarm1 = 1;
bool lockalarm2 = 0;
bool pressed, pressed1;
String dots;
int i;
WidgetRTC rtc;                        // TIMER
BlynkTimer timer, timerone, timertwo; // timer,timer CWU,check_connection
int CountdownRemainReset, CountdownRemain, CountdownTimer;
unsigned long actualtime = 0;
unsigned long savedtime = 0; //rgb
unsigned long delay_write = 1000;
void dot()
{
    if (i <= 4)
    {
        i++;
        dots = dots + ".";
        if (i == 4)
        {
            dots = "";
            i = 0;
        }
    }
}
void CountdownShowFormatted(int seconds)
{
    long days = 0;
    long hours = 0;
    long mins = 0;
    long secs = 0;
    String secs_o = ":";
    String mins_o = ":";
    String hours_o;
    secs = seconds;              // set the seconds remaining
    mins = secs / 60;            //convert seconds to minutes
    hours = mins / 60;           //convert minutes to hours
    days = hours / 24;           //convert hours to days
    secs = secs - (mins * 60);   //subtract the coverted seconds to minutes in order to display 59 secs max
    mins = mins - (hours * 60);  //subtract the coverted minutes to hours in order to display 59 minutes max
    hours = hours - (days * 24); //subtract the coverted hours to days in order to display 23 hours max
    if (secs < 10)
    {
        secs_o = ":0";
    }
    if (mins < 10)
    {
        mins_o = ":0";
    }
    if (hours < 10)
    {
        hours_o = "0";
    }

    Blynk.setProperty(V6, "onLabel", hours_o + hours + mins_o + mins + secs_o + secs);
    Blynk.setProperty(V6, "offLabel", hours_o + hours + mins_o + mins + secs_o + secs);
}
void CountdownTimerFunction()
{
    CountdownRemain--; // remove 1 every second
    CountdownShowFormatted(CountdownRemain);
    if (!CountdownRemain && lock4 == 1)
    {                                     // check if CountdownRemain == 0/FALSE/LOW
        timerone.disable(CountdownTimer); // if 0 stop timer
        stanpompa = 1;
        lock4 = 0;
    }
}
void sendSensor()
{
    sensors.requestTemperatures();
    t = sensors.getTempCByIndex(0);
    if (isnan(t))
    {
        return;
    }
    if (actualtime - savedtime >= delay_write)
    {
        Blynk.virtualWrite(V1, String(t, 1)); //temp z czujnika
        savedtime = actualtime;
    }
}
void TimeCheck()
{
    if (onoff == 1)
    {
        if (stan == 1)
        {
            if (actualtime - savedtime >= delay_write)
            {
                Blynk.setProperty(V4, "offBackColor", "#fe7f00");
                Blynk.setProperty(V4, "offLabel", "Waiting" + dots);
                Blynk.setProperty(V4, "offColor", "#000000");
                Blynk.setProperty(V4, "onBackColor", "#fe7f00");
                Blynk.setProperty(V4, "onLabel", "Waiting" + dots);
                Blynk.setProperty(V4, "onColor", "#000000");
            }
        }
        if (hour() == SThour)
        {
            if (minute() == STmin && lock3 == 1)
            {
                stan = 0;
                lock3 = 0;
            }
        }
        if (hour() == SPhour)
        {
            if (minute() == SPmin && lock == 1)
            {
                stan = 1;
                lock = 0;
            }
        }
        if (hour() == 0)
        {
            if (minute() == 0)
            {
                lock = 1;
                lock3 = 1;
            }
        }
    }
    else if (onoff == 0)
    {
        stan = 1;
        if (actualtime - savedtime >= delay_write)
        {
            Blynk.setProperty(V4, "offBackColor", "#000000");
            Blynk.setProperty(V4, "offLabel", "FURNACE-OFF");
            Blynk.setProperty(V4, "offColor", "#ff0066");
            Blynk.setProperty(V4, "onBackColor", "#000000");
            Blynk.setProperty(V4, "onLabel", "FURNACE-OFF");
            Blynk.setProperty(V4, "onColor", "#ff0066");
        }
    }
}
BLYNK_WRITE(V2)
{ // Called whenever setting Time Input Widget
    TimeInputParam t(param);
    SThour = t.getStartHour();
    STmin = t.getStartMinute();
    STsec = t.getStartSecond();
    SPhour = t.getStopHour();
    SPmin = t.getStopMinute();
    SPsec = t.getStopSecond();
}
BLYNK_WRITE(V3) // temperatura zadana
{
    t1 = param.asInt();
}
BLYNK_WRITE(V4)
{
    pressed = param.asInt();
}
BLYNK_WRITE(V5) // awaryjne wyłączenie
{
    onoff = param.asInt();
}
BLYNK_WRITE(V6)
{
    pressed1 = param.asInt();
}
BLYNK_WRITE(V7) //delay dla CWU
{
    if (timerone.isEnabled(CountdownTimer))
    {
        if (actualtime - savedtime >= delay_write)
        {                                         // only update if timer not running
            Blynk.virtualWrite(7, param.asInt()); // if running, refuse to let use change slider
        }
    }
    else
    {
        CountdownRemainReset = param.asInt() * 60 + 1; // + 1 set the timer to 1:00:00 instead of 00:59:59
        CountdownRemain = param.asInt() * 60;
        CountdownShowFormatted(CountdownRemain);
    }
}
BLYNK_WRITE(V8)
{ // Called whenever setting Time Input Widget
    TimeInputParam t(param);
    godz = t.getStartHour();
}
BLYNK_CONNECTED()
{
    TimeCheck(); // Initial Time Check
    Blynk.syncAll();
    Serial.println("CONNECTING...");
    Serial.println("Conected.");
}

void setup()
{
    Serial.begin(9600);
    digitalWrite(switch1, HIGH);
    digitalWrite(switch2, HIGH);
    digitalWrite(switch3, HIGH);
    pinMode(switch1, OUTPUT);
    pinMode(switch2, OUTPUT);
    pinMode(switch3, OUTPUT);
    WiFi.begin(ssid, pass); // Non-blocking if no WiFi available
    //Blynk.begin(auth, ssid, pass, "strajlak.duckdns.org", 8080);
    Blynk.begin(auth, ssid, pass, IPAddress(192, 168, 0, 105), 8080);
    sensors.begin();
    timer.setInterval(1000L, sendSensor);
    rtc.begin();
    //setSyncInterval(360);
    timer.setInterval(1000L, TimeCheck); // Update Time Check every 30 seconds
    timertwo.setInterval(1000L, dot);
    CountdownTimer = timerone.setInterval(1000L, CountdownTimerFunction);
    timerone.disable(CountdownTimer);  // disable it on boot
    ArduinoOTA.setHostname("FURNACE"); // For OTA
    //ArduinoOTA.setPassword((const char *)"kamil0021");
    ArduinoOTA.begin(); // For OTA
}
void loop()
{
    actualtime = millis();
    digitalWrite(switch1, stan);
    digitalWrite(switch2, stanpompa);
    ArduinoOTA.handle(); // For OTA
    timer.run();
    if (Blynk.connected())
    { // If connected run as normal
        Blynk.run();
    }
    else
    {
        Blynk.connect();
    }
    timertwo.run();
    timerone.run();
    if (t < 25 && lock4 == 0) //do CWU Jeśli temperatura mniejsza niż 25 stopni to (12:00) i jeśli coutdown jest wyłączony
    {
        lock2 = 1; //funkcja dla wyzerowania CWU
    }
    if (t >= t1 && hour() == godz && lock2 == 1 || t >= t1 && hour() == godz + 1 && lock2 == 1 || t >= t1 && hour() == godz + 2 && lock2 == 1)
    {
        lock2 = 0;
        lock5 = 1; // zmienna on CWU
    }

    if (lock5 == 1)
    {
        stanpompa = 0;
        CountdownRemain = CountdownRemainReset;
        timerone.enable(CountdownTimer);
        Serial.println("circulation pump-on");
        lock4 = 1; // zmienna off CWU
        lock5 = 0;
    }
    if (t >= 65 && lock4 == 0 && lockalarm1 == 1)
    {
        Serial.println("circulation pump-on");
        stanpompa = 0;
        lockalarm1 = 0;
        lockalarm2 = 1;
    }
    else if (t <= 60 && lock4 == 0 && lockalarm2 == 1)
    {
        Serial.println("circulation pump-off");
        stanpompa = 1;
        lockalarm2 = 0;
        lockalarm1 = 1;
    }
    if (stanpompa == 1)
    {
        if (actualtime - savedtime >= delay_write)
        {
            Blynk.setProperty(V6, "offBackColor", "#D3435C");
            Blynk.setProperty(V6, "offLabel", "CWU - OFF");
            Blynk.setProperty(V6, "offColor", "#000000");
            Blynk.setProperty(V6, "onBackColor", "#D3435C");
            Blynk.setProperty(V6, "onLabel", "CWU - OFF");
            Blynk.setProperty(V6, "onColor", "#000000");
        }
    }
    else if (stanpompa == 0)
    {
        if (actualtime - savedtime >= delay_write)
        {
            Blynk.setProperty(V6, "offBackColor", "#23C890");
            Blynk.setProperty(V6, "offColor", "#000000");
            Blynk.setProperty(V6, "onBackColor", "#23C890");
            Blynk.setProperty(V6, "onColor", "#000000");
        }
    }
    //-------------------------------------------------// funkcja alarmu pieca
    if (t < 63)
    {
        lock1 = 1;
    }
    if (t >= 65 && lock1 == 1)
    {
        if (actualtime - savedtime >= delay_write)
        {
            Blynk.notify("Alarm!!!\r\nTemperatura wynosi ponad 65°C\r\nPompa cyrkulacyjna włączona.");
        }
        lock1 = 0;
    }
    //-------------------------------------------------// koniec funkcji alarmu pieca
    if (stan == 0)
    {
        if (actualtime - savedtime >= delay_write)
        {
            Blynk.setProperty(V4, "offBackColor", "#23C890");
            Blynk.setProperty(V4, "offLabel", "Working" + dots);
            Blynk.setProperty(V4, "offColor", "#000000");
            Blynk.setProperty(V4, "onBackColor", "#23C890");
            Blynk.setProperty(V4, "onLabel", "Working" + dots);
            Blynk.setProperty(V4, "onColor", "#000000");
        }
    }
    /*
    if (analogRead(A0) >= 20)
    {
        Blynk.notify("Nastąpiła wewnętrzna awaria pieca\r\nNaciśęnięto zdalnie czerwony przycisk na piecu\r\n(POD OBUDOWĄ)");
        digitalWrite(switch3, LOW);
    }
    else
    {
        digitalWrite(switch3, HIGH);
    }
    */
    if (pressed == 1 && onoff == 1 && lock6 == 1 && t <= 60)
    {
        stan = !stan;
        lock6 = 0;
    }
    else if (pressed == 0)
    {
        lock6 = 1;
    }
    if (pressed == 1 && onoff == 0)
    {
        Blynk.notify("You must: TURN-ON FURNACE!!!");
    }
    if (pressed1 == 1 && lock4 == 0 && lock7 == 1)
    {

        lock5 = 1;
        lock7 = 0;
        lock8 = 0;
    }
    else if (pressed1 == 0)
    {
        lock7 = 1;
    }
    if (pressed1 == 1 && lock4 == 1 && lock8 == 1)
    {
        stanpompa = 1;
        lock4 = 0;
        timerone.disable(CountdownTimer);
        CountdownRemain = !CountdownRemain;
        lock8 = 0;
        lock7 = 0;
    }
    else if (pressed1 == 0)
    {
        lock8 = 1;
    }
}