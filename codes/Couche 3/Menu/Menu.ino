#include <SPI.h>
#include <RH_RF95.h>
#include <M5Stack.h>


//Selection juste if etc...
void affichage(uint16_t envoie_data , uint16_t freq, uint16_t choixTTL, uint16_t ipm5, uint16_t valTTL){
  if (envoie_data=0) {
    Serial.print("etat initial");
  }
  else {
    if (choixTTL == 0){
      Serial.printf("Frequance",freq);
    }
    if (choixTTL == 1){
      Serial.printf("");
    }
    if (choixTTL == 2){
      Serial.printf("");
    }
  }
}

//Paramètres par défault
uint16_t envoie_data = 0;

uint16_t freq=867.7;
uint16_t choixTTL=0;
uint16_t ipm5=1;
uint16_t valTTL=5;

//Affichage par défault
affichage(0,8,0,1,5);
