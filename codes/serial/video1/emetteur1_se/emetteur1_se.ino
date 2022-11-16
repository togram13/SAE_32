#include <SPI.h> //Nativement dans arduino donc pas besoin de l'installer en +
#include <RH_RF95.h> //gestion des transmissions radio en Lora
#include <M5Stack.h>

#define RFM95_CS 5 //M5Stack Lora
#define RFM95_DIO0 36
RH_RF95 rf95(RFM95_CS, RFM95_DIO0);

//uint8_t : type de donnée qui est composé d'un octet (valeurs entre 0 et 255)
uint8_t etat; //etat courant codé sur 1 octet
uint8_t data[16];
uint8_t datataille = 16; // Taille de la trame qu'on emet (2 octets)

// on defini les différents états (cf automate TX) :
#define EMISSION 0
#define ATTENTE 1

// on identifie le canal sur lequel on emet:
#define canal 0


void setup() {

M5.begin();
Serial.begin(115200);
M5.Power.begin(); //initialisation de la puissance
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

rf95.setFrequency(867.7); //règle la fréquence centrale

etat = EMISSION; // on défini l'état d'émission à 0

delay(1000);// on laisse 1 seconde pour laisser le developpeur lancer le terminal pour les messages

Serial.print("Boucle Principale\n");// On lance un message qui indique le début : tout et initialisé
Serial.print("\n");
}

//Coeur du programme qui s'execute en permanence
void loop() {

switch(etat)//Le switch permet de réagir en fonction de différents cas (ici les valeurs de etat)
  {
    case EMISSION: // etat d'emission
      Serial.print("EMISSION de la trame\n");
      Serial.print("\n");

       //On gere le format de la trame en 2 octets
       data[0] = 0x0AA; //on stocke dans le tableau de données txbuf créer plus haut
       data[1] = 0x055;
       
       rf95.send(data, 2); //emission de la trame. Le 2 correspond à 2 octets (unité : uint8_t)
       //Serial.print("data envoyée");
       rf95.waitPacketSent(); //on attent un certain délai => Le temps que la trame soit arrivée jusqu'à R : Bloque l'émission jusqu'a ce que l'émetteur ne transmette plus
       //M5.Lcd.fillScreen(RED);debeugage
       etat = ATTENTE;
       break;
       
     case ATTENTE: //etat d'attente
      Serial.print("Attente 2 seconde\n");//affichage de l'état en cours
      Serial.print("\n");
      delay(2000);//on attends 1 seconde avant d'emettre la trame suivante
      etat = EMISSION; // on passe à l'état d'émission
      break;
  }
}
