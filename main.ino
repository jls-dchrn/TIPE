#include <SPI.h>
// module for the spi bus
#include <SD.h>
// module for the SD card
#include "DHT.h"
// module for the humidity sensor
#include <Wire.h>

#include "Adafruit_SGP30.h"
// module for the SGP30 sensor (voc)

File my_csv;
// init a File type variable to open my csv file

csv const int pinBranchementCS = 4;
// The « 4 » means that the CS (SS) pin of the SD module is links to the D10 pin on your Arduino

#define DHTPIN 2
/*
The output of the humidity sensor is linked to the 2nd pin.
The value returned by the sensor are written on 5 bytes, 2 for the integer value ans 2 for the decimal value.
The last octet is used as a "checksum"
*/

#define DHTTYPE DHT11
// version of the humidity sensor in the DHT lib. We identify the sensor with his DHTPIN and his DHTTYPE.

Adafruit_SGP30 sgp;
// initialisation of the Adafruit_SGP30 as sgp to make it quick

// fonction that return the internal humidity, using the temperature and humidity values from the other sensor
uint32_t getAbsoluteHumidity(float temperature, float humidity)
{
    /*
        function used to obtain a relative intern humidity with the value of temperature and humidity from the other sensor
        The formula is given in the  datasheet
    */
    const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature));
    // [g/m^3]
    const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity); // [mg/m^3]

    return absoluteHumidityScaled;
}

#define Pin_led 7

float last_values[3] = {0.0, 0.0, 0.0};

int period = 10;
// loop after 10s (10 000 ms)

long duration = 0;
// used to add a time scale in the data

void setup()
{
    pinMode(Pin_led, OUTPUT);
    Serial.begin(9600);
    // open the communication port with the arduino (the USB cable) and allows a connexion with a frequency of 9600 bits/sec

    if (!SD.begin(pinBranchementCS))
    {
        Serial.println("problem in the connections");
    }

    // searching the sensor
    if (!sgp.begin())
    {
        Serial.println("Didn't find the sensor :(");
        while (1)
            ;
    }
    // If the sensor has been found, shows his serial number
    Serial.print("Found SGP30 serial #");

    Serial.print(sgp.serialnumber[0], HEX);

    Serial.print(sgp.serialnumber[1], HEX);

    Serial.println(sgp.serialnumber[2], HEX);

    dht.begin();
    // init the humidity sensor

    sgp.setIAQBaseline(0x9768, 0x965A);
    // download the baseline values of the Co2 and the Tvoc, you have to put your own values
}

// init the counter when the program start
int counter = 0;

void loop()
{
    float humidity = dht.readHumidity();
    // the humidity in %

    float temperature = dht.readTemperature();
    // the temperature in degree celsuis

    Serial.println("temperature:" + String(temperature) + "/humidity :" + String(humidity) + "/duration" + String(duration));

    sgp.setHumidity(getAbsoluteHumidity(temperature, humidity));

    // test the sgp30 sensor
    if (!sgp.IAQmeasure())
    {
        Serial.println("Measurement failed");
        return;
    }

    // the concentration in TVOC
    float concentration = sgp.TVOC;

    // print the values in shell
    Serial.print("TVOC ");
    Serial.print(concentration);
    Serial.print(" ppb\t");
    Serial.print("eCO2 ");

    Serial.print(sgp.eCO2);
    Serial.println(" ppm");

    // creation of an empty string
    String message = "";

    // if the csv file doesn't exist, create a header
    File test = SD.open("fichier.csv", FILE_READ);

    if (!test)
    {
        message += "time;Tvoc;Humidity;Temperature";
    }
    test.close();

    // open the file (or create it if it doesn't exist)
    my_csv = SD.open("fichier.csv", FILE_WRITE);

    // if the file can't be opened or created, raised an error
    if (!my_csv)
    {
        Serial.println("Can't open or create the file");
    }

    // if we have created a header, add it to the csv file
    if (message != "")
    {
        my_csv.println(message);
        message = "";
    }

    // download the values in the csv file, columns are separated by ";"
    message += (String(duration) + ";" + String(concentration) + ";" + String(humidity) + ";" + String(temperature));
    my_csv.println(message);

    // keep the concentration values from the last 30 sec
    last_values[counter] = concentration;

    // clear the values of my_csv in the RAM
    my_csv.flush();

    // close the file
    my_csv.close();

    /*
        we consider that if the average value of Tvoc from the 30 last seconds is greater than 200ppm,
        it is necessary to ventilate and the led turns on
    */

    if ((last_values[0] + last_values[1] + last_values[2]) / 3 > 200)
    {
        digitalWrite(Pin_led, HIGH);
    }
    else
    {
        // turns of the led
        digitalWrite(Pin_led, LOW);
    }

    // increments the counter
    counter++;
    if (counter == 3)
    {
        counter = 0;
    }

    delay(period * 1000);
    duration += period;
    // duration in sec
}