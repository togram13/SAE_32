#include <SPI.h>
#include <RH_RF95.h>
#include <M5Stack.h>

//Fonction d'affichage
void affichage(bool menu_data, uint16_t etat_menu, uint16_t freq, uint16_t choixTTL, uint16_t ipm5, uint16_t valTTL, uint16_t status_send){
  if (menu_data=0) {
    Serial.print("Etat initial");
  }
  else {
    if (choixTTL == 0){
      Serial.printf("Frequance",freq);
      Serial.print("Choix TTL : <-- Pas de TTL + Pas de destination -->");
      message_envoie(status_send);
    }
    if (choixTTL == 1){
      Serial.printf("Frequance",freq);
      Serial.print("Choix TTL : <-- Pas de TTL + Destination -->");
      Serial.printf("Destination",ipm5);
      message_envoie(status_send);
    }
    if (choixTTL == 2){
      Serial.printf("Frequance",freq);
      Serial.print("Choix TTL : <-- TTL -->");
      Serial.printf("Destination",ipm5);
      Serial.printf("Valeur TTL",valTTL);
      message_envoie(status_send);
    }
  }
}

void message_envoie(uint16_t status_send){
  if (status_send == 0){
      Serial.print("Envoyer : <-- Oui -->");
  }
  if (status_send == 1){
    Serial.print("Envoyer : <-- Modifier -->");
  }
  if (status_send == 2){
    Serial.print("Envoyer : <-- Non -->");
  }
}

void Fonction_envoie_data_noTTL_noDest(uint16_t freq, uint16_t choixTTL);
void Fonction_envoie_data_noTTL_Dest(uint16_t freq, uint16_t choixTTL, uint16_t ipm5, uint16_t self_ip);
void Fonction_envoie_data_TTL_Dest(uint16_t freq, uint16_t choixTTL, uint16_t ipm5, uint16_t valTTL, uint16_t self_ip);
