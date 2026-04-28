/************Capteur d'Oxygène**************/
#include "DFRobot_OxygenSensor.h"

/**
 * i2c slave Address, The default is ADDRESS_3.
 * ADDRESS_0   0x70  i2c device address.
 * ADDRESS_1   0x71
 * ADDRESS_2   0x72
 * ADDRESS_3   0x73
 */
#define Oxygen_IICAddress ADDRESS_3
#define COLLECT_NUMBER  10             // collect number, the collection range is 1-100.
DFRobot_OxygenSensor oxygen;
//il faut dire ici que SCL se branche sur A5 et SDA sur A4



/***********capteur de CO2*********/
/************************ Parametres de Calibration ************************/
#define MG_PIN              (A0)
#define DC_GAIN             (8.5)    // Gain de l'amplificateur interne

// MODIFIEZ CETTE VALEUR ICI APRES VOS 48H : (Tension mesurée / 8.5)
#define ZERO_POINT_VOLTAGE  (0.288) 

#define REACTION_VOLTAGE    (0.030)  // Chute de tension standard pour 1000ppm

/***************************** Courbe de Calcul *****************************/
// Format : { x (log400), y (Zero voltage), pente }
float CO2Curve[3] = {2.602, ZERO_POINT_VOLTAGE, (REACTION_VOLTAGE/(2.602-3.0))};


/*****************Capteur d'humidité*******************/
#include <dht.h>

  dht DHT;
  #define DHT22_PIN A1



#include <SoftwareSerial.h>

// RX, TX
SoftwareSerial sim7600(7,8);

String phoneNumber = "+226xxxxxx";  // numéro destinataire

// Timer pour 5 minutes
unsigned long previousMillis = 0;
const unsigned long interval = 900000; // 5 minutes = 300000 ms


float oxygene=0, co2 = 0, humidite = 0, temperature = 0;


void setup(void)
{
  Serial.begin(9600);
  Serial.println("initialisation du capteur d Oxygene");
  while(!oxygen.begin(Oxygen_IICAddress)){
    Serial.println("I2c device number error !");
    delay(1000);
  }
  Serial.println("I2c connect success !");
  delay(1000);

  Serial.println("Lecture du CO2 lancee...");
  delay(1000);

  Serial.println("lecture de l'humidte lancée");
  Serial.println("DHT TEST PROGRAM ");
  Serial.print("LIBRARY VERSION: ");
  Serial.println(DHT_LIB_VERSION);
  Serial.println();
  Serial.println("Type,\tstatus,\tHumidity (%),\tTemperature (C)");


sim7600.begin(9600);

delay(3000);

Serial.println("Initialisation SIM7600");

// Test communication
sendAT("AT",1000);

// Désactiver echo
sendAT("ATE0",1000);

// Vérifier carte SIM
sendAT("AT+CPIN?",2000);

// Vérifier réseau
sendAT("AT+CREG?",2000);

// Mode SMS texte
sendAT("AT+CMGF=1",1000);

}

void loop(void)
{
  capteurOxygene();
  delay(2000);
  capteurCo2();
  delay(2000);
  capteurHumidite();

unsigned long currentMillis = millis();

// Si 5 minutes sont passées
if(currentMillis - previousMillis >= interval){

previousMillis = currentMillis;

Serial.println("Envoi SMS toutes les 5 minutes");

//sendSMS(phoneNumber,"Test SMS automatique toutes les 5 minutes 2");
sendSMS("+226xxxxxx", "Oxygene:"+ String(oxygene) + ";" + "CO2:" + String(co2) + ";" + "Humidite:" + String(humidite) + ";" + "Temperature:" + String(temperature) + ";" );
sendSMS("+226xxxxxx", "Oxygene:"+ String(oxygene) + ";" + "CO2:" + String(co2) + ";" + "Humidite:" + String(humidite) + ";" + "Temperature:" + String(temperature) + ";" );


}


}







// Fonction pour envoyer commande AT
void sendAT(String command, int timeout){

sim7600.println(command);

unsigned long time = millis();

while((millis()-time) < timeout){

while(sim7600.available()){

Serial.write(sim7600.read());

}

}

}

// Fonction envoi SMS
void sendSMS(String number,String message){

Serial.println("Envoi SMS...");

sim7600.print("AT+CMGS=\"");
sim7600.print(number);
sim7600.println("\"");

delay(1000);

sim7600.print(message);

delay(500);

// CTRL+Z pour envoyer
sim7600.write(26);

delay(5000);

Serial.println("SMS envoyé");

}


void capteurOxygene(){
  float oxygenData = oxygen.getOxygenData(COLLECT_NUMBER);
  oxygene=oxygenData;
  Serial.print(" oxygen concentration is ");
  Serial.print(oxygenData);
  Serial.println(" %vol");
  delay(1000);
}




void capteurCo2() {
    float volts = readVoltage(MG_PIN);
    int ppm = getPPM(volts, CO2Curve);
    co2= ppm;
    Serial.print("Concentration CO2 : ");
    if (ppm == -1) {
        Serial.print("< 400");
    } else {
        Serial.print(ppm);
    }
    Serial.println(" ppm");

    delay(2000); // Mise a jour toutes les 2 secondes
}

// Fonction pour lire la tension moyenne
float readVoltage(int pin) {
    float v = 0;
    for (int i = 0; i < 10; i++) {
        v += analogRead(pin);
        delay(50);
    }
    return (v / 10.0) * 5.0 / 1024.0;
}

// Fonction de calcul des PPM
int getPPM(float volts, float *pcurve) {
   float v_amplified = volts / DC_GAIN;
   if (v_amplified >= pcurve[1]) {
      return -1;
   } else {
      // Formule logarithmique inverse
      return pow(10, (v_amplified - pcurve[1]) / pcurve[2] + pcurve[0]);
   }
}



void capteurHumidite()
  {
    Serial.print("DHT22, \t");
    int chk = DHT.read22(DHT22_PIN);  
    switch (chk)
    {
      case DHTLIB_OK:
                  Serial.print("OK,\t");
                  break;
      case DHTLIB_ERROR_CHECKSUM:
                  Serial.print("Checksum error,\t");
                  break;
      case DHTLIB_ERROR_TIMEOUT:
                  Serial.print("Time out error,\t");
                  break;
      default:
                  Serial.print("Unknown error,\t");
                  break;
    }

    Serial.print(DHT.humidity, 1);
    Serial.print(",\t");
    Serial.println(DHT.temperature, 1);

    humidite = (DHT.humidity);
    temperature = (DHT.temperature);
      delay(1000);
  }







