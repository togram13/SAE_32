
#include <SPI.h> //Nativement dans arduino donc pas besoin de l'installer en +
#include <RH_RF95.h> //gestion des transmissions radio en Lora
#include <M5Stack.h>

#define RFM95_CS 5 //M5Stack Lora
#define RFM95_DIO0 36
RH_RF95 rf95(RFM95_CS, RFM95_DIO0);

uint8_t etat; //etat courant codé sur 1 octet
uint8_t rxdata[16]; //Tableau du buffer de reception
uint8_t rxdatataille = 16; // Taille Max du buffer en reception
uint8_t rxlendata = 16; //Taille de la trame reçue
int rxNbTrame; //nombre de trames reçues

#define ECOUTE 1
#define TEST_RX 2
#define AFFICHAGE 3

#define canal 1 // Identification du même canal que pour l'émission, pour la réception

void setup() {
  M5.begin();
  
  Serial.print(115200);
  
  M5.Power.begin(); //initialisation de la puissance
  M5.Lcd.setTextFont(4);
  Serial.print("Video DATA Point a point\n");
  Serial.print("\n");
  Serial.print("On commence\n");
  Serial.print("\n");

  //initialisation du transceiver radio:

  if(!rf95.init()) //init règle la puissance à 13dBm
    Serial.print("Erreur initialisation RF95\n"); // on vérifie si il y a une erreur avec le transceiver
                                                 // Problèmes possibles : cablages ou drivers logiciels
  else
    Serial.print("Initialisation OK\n");
    Serial.print("\n");

      //configuration des parametres Radio :
  rf95.setModemConfig(RH_RF95::Bw125Cr45Sf128); //Selectionne des config de modems prédéfinis
  rf95.setFrequency(867.7); //règle la fréquence centrale Bande

  etat = ECOUTE; //état de départ(cf Automate récepteur)
  delay(1000);
  Serial.print("boucle principale\n"); // Message du début, qui nous indique la fin des config  de début
  Serial.print("\n");
  delay(1000);
  rxNbTrame = 0; // Initialisation à 0 du nombre de trames reçues
}

void loop() {
  switch(etat)//Le switch permet de réagir en fonction de différents cas (ici les valeurs de etat)
  {
      case ECOUTE: // etat d'ecoute(etat de départ)
        Serial.print("\n");
        Serial.print("En ECOUTE\n");
        Serial.print("\n");

        rf95.setModeRx(); // On met le transceiver en mode réception
        
        etat = TEST_RX; // On passe à l'état d'après (Etat 2)
        break;
         
       case TEST_RX: //etat de test : boucle qui test si il y des données dans le buffer
        //delay(1000);
          //Serial.print("Test du buffer\n");//affichage de l'état en cours
          //Serial.print("\n");
          
          //Serial.print(rf95.available());
          if(rf95.available()) // Si il y a des choses dans le buffer => execute le code du if
          {
            Serial.print("Il y a quelque chose dans le buffer\n");
            Serial.print("\n");
            etat = AFFICHAGE;
          }
          break;
        
      case AFFICHAGE : //On passe en état d'affichage 
        if(rf95.recv(rxdata, &rxlendata)) // On récupère les datas dans le buffer, en lui donnant en paramètres le buffer de réception (rxdata) et on passe par reference rxlendata qui contient la taille Max de la trame reçue (j'ai mis 16) 
        //La valeur de rxlendata changera en 2 car le buffer ne contiendra que 2 octets quelque soit l'envoi
        {
          rxNbTrame++; // on incrémente le nombre de trames reçues de 1
          Serial.print("Nombre de trame: "); // Affichage du nombre de trames reçues 
          Serial.print(rxNbTrame);
          Serial.print("\n");
          Serial.print("\n");
          Serial.print("nombre d'octets reçu : "); // Affichage du nombres d'octets reçus
          Serial.print(rxlendata);
          Serial.print("\n");
          for(int i=0; i<rxlendata; i++)
          {
            //Serial.print("%02xI\n"); 
            Serial.print("Octet ");
            Serial.print(i);
            Serial.print(" : ");
            Serial.print(rxdata[i]); // Affichage du contenu de la trame
            Serial.print("\n");
          }
        Serial.print("");
        Serial.print("");
      }
      etat = ECOUTE;
      break;

      default: // Par précaution, on prévoit au switch un cas par défaut
        break;
  }
}
