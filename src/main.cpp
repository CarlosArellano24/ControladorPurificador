#define DEBUG
 
/**
 *  NOTE: Air purifier is refered to as "device" while phone or web app is refered to as "client"
 *  TODO:
 *  connection procedure:
 *    client initiates connection (Checking every 5 minutes)
 *    client sends uid as serial stream
 *    device updates its internal array and realtime database uid data
 *    phone sends command to restart bluetooth module
 *    if this command is not received, the connected device will restart bluetooth module on its own
 *  
 *  connection monitoring:
 *    device keeps track of every currently connected client's time of connection
 *    every device that doesn't report back after a set time of 10 minutes (through connection procedure) gets deleted from the internal array
 *    device updates its realtime database data from internal array
 *  
 *  BACKLOG:
 *  NFC support so users can see air quality stats of a device
 *  
 */

#include <Arduino.h>
#include <math.h>

const uint8_t led = 4 ;
const uint8_t buzzer = 5;
const uint8_t sensor = A0;

// calibration resistance of the sensor
const double RL = 11e3;
// sensing resistance for H2 at 1000ppm (1%)
double Ro = 21261.4446;
const float cleanAirRatio = 9.83f;
// const unsigned sensorThreshhold = 350;

// defining profiles for different gases
// format is: { x1, y1, m }
// where x1, y1 is a poing on the graph representing ppm and RS/Ro ratio respectively; and m is the logarithmic graph slope
/*
const double CO[3] = {
    2.3, 0.72,
    -0.34
};
const double LPG[3] = {
    2.3, 0.21,
    -0.47
};
const double Smoke[3] = {
    2.3, 0.53,
    -0.44
};
*/

const double CO[2]    = { 1.51, -0.34 };
const double LPG[2]   = { 1.25, -0.47 };
const double Smoke[2] = { 1.62, -0.44 };

void alarm();
double getRS(const unsigned &reading);
double getPpmValue(const double &RS, const double gasProfile[3]);


void setup()
{
    Serial.begin(115200);
    #ifdef DEBUG
    Serial.println("#### Serial comms initiated ####");
    #endif

    pinMode(led, OUTPUT);
    pinMode(buzzer, OUTPUT);
    pinMode(sensor, INPUT);

    Ro = getRS(51) / cleanAirRatio;

    // sleep device for 2 minutes to allow sensor to heat up and prevent it from throw ng false alarms
    delay(120e3);
}

void loop()
{
    unsigned reading = analogRead(sensor);

    double RS = getRS(reading);

    double ppmCO    = getPpmValue(RS, CO[1], CO[0]);
    double ppmLPG   = getPpmValue(RS, LPG[1], LPG[0]);
    double ppmSmoke = getPpmValue(RS, Smoke[1], Smoke[0]);

    #ifdef DEBUG
    Serial.print("CO: ");    Serial.print(ppmCO); Serial.print("ppm    ");
    Serial.print("LPG: ");   Serial.print(ppmLPG); Serial.print("ppm    ");
    Serial.print("Smoke: "); Serial.print(ppmSmoke); Serial.println("ppm");
    #endif


    // obtaining sensing resistance derived from voltage divider formula
    // raw readings are used to calculate this as it's not necessary to them convert to voltage values

    if (ppmCO > 500) // previously 800
        alarm();
}

void alarm()
{
    #ifdef DEBUG
    Serial.println("Aire contaminado!!!");
    #endif

    digitalWrite(led, HIGH);
    tone(buzzer, 1000, 500);
    digitalWrite(led, LOW);
    delay(500);
}

double getRS(const unsigned &reading)
{
    return RL * (1024 - reading) / reading;
}

double getPpmValue(const double &RS, const double m, const double b)
{
    /*
    double x1 = gasProfile[0];
    double y1 = gasProfile[1];
    double m = gasProfile[2];
    */

    return pow(10, (log10(RS/Ro) - b) / m);
    // return pow(10, (log10(RS / Ro) - y1)/m + x1);
}