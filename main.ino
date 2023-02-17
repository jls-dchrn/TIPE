#include <SPI.h>
// module pour le bus spi
#include <SD.h>
// module pour la carte SDr
#include "DHT.h"
// module pour le capteur
d'humidité
#include <Wire.h>
#include "Adafruit_SGP30.h"
    // module pour le capteur SGP30

    File mon_csv;
// initialise une variable de
type File pour ouvrir mon fichier
    csv const int pinBranchementCS =
        4;
// Le « 4 » indiquant ici que labroche CS (SS) de votre lecteur de carte SD est branché sur la pin D10 de notre Arduino
#define DHTPIN 2
/*
pin où est branché la sortie du capteur d'humidité pin digital renvoie la valeur"HIGH" ou "LOW" qui correspond à
l'etat de l'échelon envoyé par le capteur le capteur envoie 5 octets (40bits) de donnnes qui correspondent aux valeurs
entieres puis decimales de l'humidité puis de la température (2x2 octets) et le dernier octet est un checksum
*/

#define DHTTYPE DHT11
// version du capteur d'humidité, utilisé dans la librairie DHT pour l'identifier DHT dht(DHTPIN, DHTTYPE);
// creation d'un objet DHT possedant les infos sur moncapteur, permet d'utiliser les methodes de la classe DHT importée

Adafruit_SGP30 sgp;
// on initialise le nom du module Adafruit_SGP30 comme sgp pour que ce soit plus cours à ecrire

// fonction permettant d'obtenir une humidité interne par rapport aux valeurs de
// températures et d'humidité obtenues par un autre capteur

uint32_t getAbsoluteHumidity(float temperature, float humidity)
{
    // formule donnée avec le capteur
    const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature));
    // [g/m^3]
    const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity); // [mg/m^3]

    return absoluteHumidityScaled;
}

#define Pin_led 7

float dernieres_valeurs[3] = {0.0, 0.0, 0.0};

int periode = 10;
// boucle réalisée toutes les 10s (10 000 ms)

long duree = 0;
// permet d'ajouter une échelle de temps aux données

void setup()
{
    pinMode(Pin_led, OUTPUT);
    Serial.begin(9600);
    // Ouvre le port de communication avec l'arduino (lecable USB) et permet une communication de 9600 bits/seconde

    if (!SD.begin(pinBranchementCS))
    {
        Serial.println("problème au niveau du branchement");
    }

    // recherhe du capteur
    if (!sgp.begin())
    {
        Serial.println("On ne trouve pas le capteur :(");
        while (1)
            ;
    }

    // message si le capteur a été trouvé, donne son numéro de série
    Serial.print("Found SGP30 serial #");

    Serial.print(sgp.serialnumber[0], HEX);

    Serial.print(sgp.serialnumber[1], HEX);

    Serial.println(sgp.serialnumber[2], HEX);

    dht.begin();
    // initialise le capteur d'humidité

    sgp.setIAQBaseline(0x9768, 0x965A);
    // enregistre les valeurs de baseline pr le co2 et le tvoc (obtenues en air ext)
}

// initialise un compteur au lancement
int counter = 0;

void loop()
{
    float humidity = dht.readHumidity();
    // donne l'humidité en %
    float temperature = dht.readTemperature();
    // donne la temperature en degres celsuis

    Serial.println("temperature:" + String(temperature) + "/humidité :" + String(humidity) + "/durée" + String(duree));

    sgp.setHumidity(getAbsoluteHumidity(temperature, humidity));

    // teste si la mesure du sgp30 marche bien
    if (!sgp.IAQmeasure())
    {
        Serial.println("Measurement failed");
        return;
    }

    // obtient la concentration en TVOC
    float concentration = sgp.TVOC;

    // affiche les valeurs dans le terminal
    Serial.print("TVOC ");
    Serial.print(concentration);
    Serial.print(" ppb\t");
    Serial.print("eCO2 ");

    Serial.print(sgp.eCO2);
    Serial.println(" ppm");

    // création d'une chaine vide contenant le message à enregistrer
    String message = "";

    // si le fichier csv n'existe pas encore, créer une entête
    File test = SD.open("fichier.csv", FILE_READ);

    if (!test)
    {
        message += "time;Tvoc;Humidity;Temperature";
    }
    test.close();

    // ouverture du fichier (ou création s'il n'existe pas)
    mon_csv = SD.open("fichier.csv", FILE_WRITE);

    // Si le fichier ne peut être ouvert ou créé, afficher un message d'erreur
    if (!mon_csv)
    {
        Serial.println("Problème d'ouverture du fichier.");
    }

    // Si il y a une entête, l'ajoute au fichier csv
    if (message != "")
    {
        mon_csv.println(message);
        message = "";
    }

    // enregistre les valeurs des concentrations de gaz dans chaque colonne grâce au séparateur ";"
    message += (String(duree) + ";" + String(concentration) + ";" + String(humidity) + ";" + String(temperature));
    mon_csv.println(message);

    // stock la valeur de concentration pour les 30 prochaines secondes
    dernieres_valeurs[counter] = concentration;

    // nettoie les valeurs enregistrées sur la memoire vive dans la variable mon_csv
    mon_csv.flush();

    // ferme le fichier
    mon_csv.close();

    if ((dernieres_valeurs[0] + dernieres_valeurs[1] + dernieres_valeurs[2]) / 3 > 200)
    {
        digitalWrite(Pin_led, HIGH);
    }
    else
    {
        digitalWrite(Pin_led, LOW);
    }

    // incrémente le compteur
    counter++;
    if (counter == 3)
    {
        counter = 0;
    }

    delay(periode * 1000);
    duree += periode;
    // duree en secondes
}