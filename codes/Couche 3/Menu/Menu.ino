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
      message_choix_freq(etat_menu, freq);
      message_choix_TTL(etat_menu, choixTTL);
      message_envoie(etat_menu, status_send);
    }
    if (choixTTL == 1){
      message_choix_freq(etat_menu, freq);
      message_choix_TTL(etat_menu, choixTTL);
      message_choix_dest(etat_menu, ipm5);
      message_envoie(etat_menu, status_send);
    }
    if (choixTTL == 2){
      message_choix_freq(etat_menu, freq);
      message_choix_TTL(etat_menu, choixTTL);
      message_choix_dest(etat_menu, ipm5);
      message_choix_valTTL(etat_menu, valTTL);
      message_envoie(etat_menu, status_send);
    }
  }
}

void message_choix_freq(uint16_t etat_menu, uint16_t freq){
  if (etat_menu == 0) {
    Serial.printf("Frequance : <--",freq);
    Serial.printf("-->");
  }
  else {
    Serial.printf("Frequance",freq);
  }
}

void message_choix_TTL(uint16_t etat_menu, uint16_t choixTTL){
  if (etat_menu == 1){
    if (choixTTL == 0){
      Serial.print("Choix TTL : <-- Pas de TTL + Pas de destination -->");
    }
    if (choixTTL == 1){
      Serial.print("Choix TTL : <-- Pas de TTL + Destination -->");
    }
    if (choixTTL == 2){
      Serial.print("Choix TTL : <-- TTL -->");
    }
  }
  else {
    if (choixTTL == 0){
      Serial.print("Choix TTL : Pas de TTL + Pas de destination");
    }
    if (choixTTL == 1){
      Serial.print("Choix TTL : Pas de TTL + Destination");
    }
    if (choixTTL == 2){
      Serial.print("Choix TTL : TTL");
    }
  }
}

void message_choix_dest(uint16_t etat_menu, uint16_t ipm5){
  if (etat_menu == 2) {
    Serial.printf("Destination : <--",ipm5);
    Serial.printf("-->");
  }
  else {
    Serial.printf("Destination :",ipm5);
  }
}

void message_choix_valTTL(uint16_t etat_menu, uint16_t valTTL){
  if (etat_menu == 3) {
    Serial.printf("Valeur TTL : <--",valTTL);
    Serial.printf("-->");
  }
  else {
    Serial.printf("Valeur TTL :",valTTL);
  }
}

void message_envoie(uint16_t etat_menu, uint16_t status_send){
  if (etat_menu == 4){
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
  else {
    if (status_send == 0){
        Serial.print("Envoyer : Oui");
    }
    if (status_send == 1){
      Serial.print("Envoyer : Modifier");
    }
    if (status_send == 2){
      Serial.print("Envoyer : Non");
    }
  }
}

void Fonction_envoie_data_noTTL_noDest(uint16_t freq, uint16_t choixTTL);
void Fonction_envoie_data_noTTL_Dest(uint16_t freq, uint16_t choixTTL, uint16_t ipm5, uint16_t self_ip);
void Fonction_envoie_data_TTL_Dest(uint16_t freq, uint16_t choixTTL, uint16_t ipm5, uint16_t valTTL, uint16_t self_ip);
