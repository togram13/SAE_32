#include <SPI.h> //Nativement dans arduino donc pas besoin de l'installer en +
#include <RH_RF95.h> //gestion des transmissions radio en Lora
#include <M5Stack.h>
 
// on defini les différents états (cf automate TX) :
#define EMISSION 0
 

void Fonction_envoie_data_noTTL_noDest(uint16_t freq){
  //uint8_t : type de donnée qui est composé d'un octet (valeurs entre 0 et 255)
  char temp[255];

  M5.Lcd.clear(BLACK);

  rf95.setFrequency(freq); //règle la fréquence d'emission

  printString("Emission du paquet : ");
  sprintf(temp, "%u", Seq);
  printString(temp);
  termPutchar('\r');
  
  //Remplissage des octets de la trame :
  txbuf[0] = 255; // Premier octet reservé à l'adresse broadcast
  txbuf[1] = 255; // Deuxieme octet réservé à l'adresse broadcast
    
  for (i = 2; i<4; i++) // On insère 2 octets de données utiles dans la trame
    {
      txbuf[i] = 1;// 2 octets à 1 de payload
    }

  // affichage pour debug
  for (i = 0; i<4; i++) {
    // Affichage de la trame envoyée (De tous les octets) :
    sprintf(temp, "%u", txbuf[i]);
    printString(temp);
    printString(" | ");
    }
    termPutchar('\r');
    termPutchar('\r');

  delay(1000);
  rf95.send(txbuf, 4);    // emission
  Seq = Seq +1;
  rf95.waitPacketSent();
}