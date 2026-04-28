#include <SoftwareSerial.h>
#include <DFRobot_PH.h>
#include <EEPROM.h>
#include <OneWire.h>
#include <DallasTemperature.h>

//capteur de ph
#define PH_PIN A6
float voltage,phValue;
DFRobot_PH ph;

//capteur de turbidite
const int turbiditySensorPin = A11;  // Broche analogique connectée au capteur
float voltage_turbidity;

//capteur de ec et temperature avec EC
#include "DFRobot_ECPRO.h"

#define EC_PIN A7
#define TE_PIN A8

DFRobot_ECPRO ec;
DFRobot_ECPRO_PT1000 ecpt;

uint16_t InputVoltage;   // Tension lue du capteur EC (mV)
float Conductivity;      // Conductivité (µS/cm)

uint16_t EC_Voltage, TE_Voltage;
float Temp;

//ecoulement de l'eau 
#define FLOW_SENSOR_PIN 3

// Variables globales
volatile int pulseCount = 0;
float flowRate_Lmin = 0.0;
float totalVolume_L = 0.0;
unsigned long previousTime = 0;

const float CALIB_FACTOR = 7.5;

// Capteur TDS
#define TDS_SENSOR_PIN A9


// Paramètres de calibration
#define VREF 5.0
#define ADC_RESOLUTION 1024.0
#define SAMPLE_INTERVAL 20
#define SAMPLE_COUNT 30

#define TDS_CONVERSION_FACTOR 0.87
#define TEMP_COEFFICIENT 0.02

//capteur de température DS18B20

float temperatureC;
// Objets pour le capteur de température
#define TEMPERATURE_SENSOR_PIN 5
OneWire oneWire(TEMPERATURE_SENSOR_PIN);
DallasTemperature tempSensor(&oneWire);

//float temperature = 25.0;
float tdsValue = 0.0;

//capteur d'oxygène dissous
#define DO_PIN A10
float Vref = 5000.0;
float SaturationDO = 8.26;
float niveauOxygene = 0.0;



#define DELAY 100


float dataSize = 0.0;
float datasize_flowRate_Lmin;
float datasize_phValue;
float datasize_voltage_turbidity;
float datasize_temperatureC;
float datasize_Conductivity;
float datasize_tdsValue;
float datasize_niveauOxygene;

int i;
//GSM
SoftwareSerial mySerial(7, 8);//RX , TX

// Variables de temporisation
unsigned long previousMillisT = 0;
const unsigned long interval = 15UL * 60UL * 1000UL; // 5 minutes

// NOUVEAU: Temporisation pour SMS toutes les 24 heures
unsigned long previousSMSMillis = 0;
const unsigned long smsInterval = 10UL * 60UL * 60UL * 1000UL; // 24 heures

// NOUVEAU: Numéro pour les SMS
const String SMS_PHONE = "+226xxxxxx";

//SIM7600
SoftwareSerial sim7600(7, 8);


void setup() {
  mySerial.begin(9600);
  Serial.begin(9600);
  //sim7600
  sim7600.begin(115200);
  delay(3000);
  Serial.println("Initialisation du module SIM7600G...");

  // Vérifier si le module répond
  sendAT("AT", 1000);

  // Désactiver l'écho
  sendAT("ATE0", 1000);

  // Vérifier la carte SIM
  sendAT("AT+CPIN?", 2000);

  // Vérifier l'enregistrement au réseau
  sendAT("AT+CREG?", 2000);

  // Définir le mode SMS en texte
  sendAT("AT+CMGF=1", 1000);


  
  
  ph.begin();
  ec.setCalibration(1.45);
  Serial.print("Calibration K = ");
  Serial.println(ec.getCalibration());

  pinMode(FLOW_SENSOR_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), countPulse, FALLING);

  pinMode(TDS_SENSOR_PIN, INPUT);
  tempSensor.begin();
  

  Serial.println("Système de monitoring eau démarré");

}


//sim7600
void sendAT(String command, unsigned long timeout) {
  sim7600.println(command);
  unsigned long t = millis();
  while (millis() - t < timeout) {
    if (sim7600.available()) {
      Serial.write(sim7600.read());
    }
  }
}

void loop() {
  datasize_flowRate_Lmin = sizeof(flowRate_Lmin);
  datasize_phValue = sizeof(phValue);
  datasize_voltage_turbidity = sizeof(voltage_turbidity);
  datasize_temperatureC = sizeof(temperatureC);
  datasize_Conductivity= sizeof(Conductivity);
  datasize_tdsValue = sizeof(tdsValue);
  datasize_niveauOxygene = sizeof(niveauOxygene);


  if (mySerial.available() > 0) {
    Serial.write(mySerial.read());
  }

  niveauOxygene = readDO();
  Serial.print("Oxygène dissous : ");
  Serial.print(niveauOxygene, 2);
  Serial.println(" mg/L");
  delay(2000);

  phsensor();
  delay(DELAY);
  turbidity_sensor();
  delay(DELAY);
  waterflow_sensor();
  delay(DELAY);
  ec_sensor();
  delay(DELAY);

  readTemperature(); 
  tdsValue = readTDSWithCompensation(); 
  displayResults();  
  delay(2000);

  // Calcul de la taille totale des données
  dataSize += String(flowRate_Lmin).length() + String(phValue).length() + String(voltage_turbidity).length() + String(temperatureC).length() + String(Conductivity).length() + String(tdsValue).length() + String(niveauOxygene).length();
  
  Serial.print("Taille cumulée des données: ");
  Serial.print(dataSize);
  Serial.println(" octets");

  // Envoi des données à ThingSpeak toutes les 5 minutes
  unsigned long currentMillisT = millis();
  if (currentMillisT - previousMillisT >= interval) {
    previousMillisT = currentMillisT;
    Serial.println("Envoi des données toutes les 5 minutes...");
      // Envoyer un SMS
  sendSMS("+226xxxxxx", "Debit:"+ String(flowRate_Lmin) + ";" + "PH:" + String(phValue) + ";" + "Turbidite:" + String(voltage_turbidity) + ";" + "Temperature:" + String(temperatureC) + ";" + "Conductivite:"+ String(Conductivity) + ";" + "TDS:"+ String(tdsValue) + ";" + "Oxygene:"+ String(niveauOxygene));
  sendSMS("+226xxxxxx", "Debit:"+ String(flowRate_Lmin) + ";" + "PH:" + String(phValue) + ";" + "Turbidite:" + String(voltage_turbidity) + ";" + "Temperature:" + String(temperatureC) + ";" + "Conductivite:"+ String(Conductivity) + ";" + "TDS:"+ String(tdsValue) + ";" + "Oxygene:"+ String(niveauOxygene));

    senddata();
    delay(3000);
  }

  
}


//sim7600
void sendSMS(String number, String message) {
  Serial.println("Envoi du SMS...");

  sim7600.print("AT+CMGS=\"");
  sim7600.print(number);
  sim7600.println("\"");
  delay(1000);

  sim7600.print(message);
  delay(500);

  sim7600.write(26); // CTRL+Z pour envoyer
  delay(3000);

  Serial.println("SMS envoyé !");
}




//fonction envoie de donnée
void senddata()
{
  // Envoyer des commandes AT au module GPRS pour configurer la connexion
  mySerial.println("AT");
  delay(DELAY);
  mySerial.println("AT+CPIN?");
  delay(DELAY);
  mySerial.println("AT+CREG?");
  delay(DELAY);
  mySerial.println("AT+CGATT?");
  delay(DELAY);
  mySerial.println("AT+CIPSHUT");
  delay(DELAY);
  mySerial.println("AT+CIPSTATUS");
  delay(DELAY);
  mySerial.println("AT+CIPMUX=0");
  delay(DELAY);
  
  ShowSerialData();

  // Configurer l'APN de l'opérateur
  mySerial.println("AT+CSTT=\"orange.com\"");
  delay(DELAY);
  ShowSerialData();

  // Établir une connexion sans fil
  mySerial.println("AT+CIICR");
  delay(DELAY);
  ShowSerialData();

  // Obtenir l'adresse IP locale
  mySerial.println("AT+CIFSR");
  delay(DELAY);
  ShowSerialData();

  mySerial.println("AT+CIPSPRT=0");
  delay(DELAY);
  ShowSerialData();

  // Démarrer une connexion TCP avec ThingSpeak
  mySerial.println("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",\"80\"");
  delay(DELAY);
  ShowSerialData();

  // Préparer l'envoi de données vers le serveur distant
  mySerial.println("AT+CIPSEND");
  delay(DELAY);
  ShowSerialData();

  String str = "GET https://api.thingspeak.com/update?api_key=xxxxxxxxxxxxxxxx&field1=" + String(flowRate_Lmin) + "&field2=" + String(phValue) + "&field3=" + String(voltage_turbidity) + "&field4=" + String(temperatureC) + "&field5=" + String(Conductivity)+ "&field6=" + String(tdsValue)+ "&field7=" + String(niveauOxygene);
  Serial.println(str);
  
  // Envoyer la requête GET au serveur
  mySerial.println(str);
  delay(DELAY);
  ShowSerialData();
  
  // Envoyer le caractère de fin de transmission
  mySerial.println((char)26); // envoyer
  delay(DELAY);
  mySerial.println();

  ShowSerialData();

  // Fermer la connexion TCP
  mySerial.println("AT+CIPSHUT");
  delay(100);
  ShowSerialData();

}


//capteur d'oxygène dissous
// ---- Fonction pour lire l’oxygène dissous ----
float readDO() {
  int adcValue = analogRead(DO_PIN);                // Lire ADC
  float voltage = (adcValue / 1023.0) * Vref;       // Convertir en mV
  float doValue = (voltage / 1000.0) * (SaturationDO / 2.0); // Conversion approx
  
  // Debug facultatif
  Serial.print("ADC: "); Serial.print(adcValue);
  Serial.print(" | Voltage: "); Serial.print(voltage, 2);
  Serial.print(" mV | DO: "); Serial.print(doValue, 2);
  Serial.println(" mg/L");

  return doValue;
}




//capteur de vitesse d'écoulement de l'eau
// Affichage des données
void waterflow_sensor() {
  if ((millis() - previousTime) > 1000) {  // Toutes les secondes
    // Section critique (désactive les interruptions)
    noInterrupts();
    // 1. Calcul du débit
    flowRate_Lmin = (pulseCount * 60.0) / CALIB_FACTOR;  // Convertit en L/min
    // 2. Calcul du volume total
    totalVolume_L += flowRate_Lmin / 60.0;  // Ajoute le volume écoulé (en L)
    pulseCount = 0;  // Réinitialisation
    interrupts();    // Réactive les interruptions
    previousTime = millis();
  }
  // Affichage du débit
  Serial.println("-> DONNEES DE DEBIT <-");
  Serial.print("Débit actuel: ");
  Serial.print(flowRate_Lmin, 2);
  Serial.println(" L/min");
  
  // Ligne de séparation
  Serial.println("------------------");
  
  // Affichage du volume
  Serial.println("-> VOLUME ECOULE <-");
  Serial.print("Total: ");
  Serial.print(totalVolume_L, 3);
  Serial.println(" litres");
  
  // Séparation pour la prochaine mesure
  Serial.println("-------------------------------");
}
// Fonction d'interruption
void countPulse() {
  pulseCount++;
}



//fonction capteur de EC et temperature avec EC
 void ec_sensor()
 {

 // === 2. Lecture tension capteur EC (conversion ADC -> mV) ===
  InputVoltage = (uint32_t)analogRead(EC_PIN) * 5000 / 1024;

  // === 3. Compensation température + conversion en µS/cm ===
  // La fonction getEC_us_cm applique la calibration, mais ici on ajuste en plus pour ramener à 25°C
  float compensationCoefficient = 1.0 + 0.02 * (temperatureC - 25.0);
  float compensatedVoltage = (float)InputVoltage / compensationCoefficient;
  Conductivity = ec.getEC_us_cm(compensatedVoltage);
  Serial.print("InputVoltage: ");
  Serial.print(InputVoltage);
  Serial.print(" mV\tConductivité (compensée): ");
  Serial.print(Conductivity);
  Serial.println(" µS/cm");

  Serial.println("-----------------------------");
  delay(1000); // Attente 1 seconde avant la prochaine mesure


  delay(1000);
 }




//fonction capteur de ph 
 void phsensor()
 {
  static unsigned long timepoint = millis();
    if(millis()-timepoint>1000U){                  //time interval: 1s
        timepoint = millis();
        //temperature = readTemperature();         // read your temperature sensor to execute temperature compensation
        voltage = analogRead(PH_PIN)/1024.0*5000;  // read the voltage
        phValue = ph.readPH(voltage,temperatureC);  // convert voltage to pH with temperature compensation
        phValue= phValue+4;
        Serial.print("temperature:");
        Serial.print(temperatureC,1);
        Serial.print("C pH:");
        Serial.println(phValue,2);
    }
   // ph.calibration(voltage,temperatureC);           // calibration process by Serail CMD

 }


//fonction turbidite

void turbidity_sensor()
{
  int sensorValue = analogRead(A11);// read the input on analog pin 0:
  voltage_turbidity = sensorValue * (5.0 / 1024.0); // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  //Serial.print(voltage); // print out the value you read:
  //Serial.println("v");
  Serial.print(voltage_turbidity);
  Serial.println("NTU");
  delay(500);
}


// Fonction pour afficher les données série du module GPRS
void ShowSerialData() {
  while (mySerial.available() != 0) {
    Serial.write(mySerial.read());
  }
  delay(5000);  // attendre 5 secondes avant de continuer
}



// Capteur TDS avec compensation de température
// Utilise un capteur de température DS18B20 pour une précision optimale

// Fonction pour lire la température
void readTemperature() {
  tempSensor.requestTemperatures();
  float tempReading = tempSensor.getTempCByIndex(0);
  
  // Vérifier si la lecture est valide
  if (tempReading != DEVICE_DISCONNECTED_C) {
    temperatureC = tempReading;
    //Serial.println(temperatureC);
    //Serial.print("C");
  } else {
    Serial.println(F("Erreur: Capteur température déconnecté"));
    temperatureC = 25.0; // Valeur par défaut
    //Serial.println(temperatureC);
  }
}

// Fonction pour lire la tension moyenne du capteur TDS
float readAverageVoltage() {
  float sum = 0;
  for (int i = 0; i < SAMPLE_COUNT; i++) {
    sum += analogRead(TDS_SENSOR_PIN);
    delay(SAMPLE_INTERVAL);
  }
  float average = sum / SAMPLE_COUNT;
  return average * (VREF / ADC_RESOLUTION);
}

// Fonction pour convertir la tension en TDS (sans compensation)
float voltageToTDS(float voltage) {
  // Formule polynomiale pour conversion tension → TDS
  return (133.42 * voltage * voltage * voltage - 255.86 * voltage * voltage + 857.39 * voltage) * TDS_CONVERSION_FACTOR;
}

// Fonction principale de lecture TDS avec compensation thermique
float readTDSWithCompensation() {
  // Lire la tension moyenne
  float voltage = readAverageVoltage();
  
  // Convertir en TDS brut
  float rawTDS = voltageToTDS(voltage);
  
  // Appliquer la compensation de température
  return rawTDS / (1.0 + TEMP_COEFFICIENT * (temperatureC - 25.0));
}

// Fonction d'affichage des résultats
void displayResults() {
  Serial.print(F("Temp: "));
  Serial.print(temperatureC, 1);
  Serial.print(F("°C | TDS: "));
  Serial.print(tdsValue, 0);
  Serial.print(F(" ppm | Voltage: "));
  Serial.print(readAverageVoltage(), 3);
  Serial.println(F("V"));
}



