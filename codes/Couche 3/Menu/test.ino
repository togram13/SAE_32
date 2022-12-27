#include <SPI.h>
#include <RH_RF95.h>
#include <M5Stack.h>

//Fonction d'affichage
void affichage(uint8_t menu_data, uint16_t etat_menu, uint16_t freq, uint16_t choixTTL, uint16_t ipm5, uint16_t valTTL, uint16_t status_send){
  if (menu_data == 0) {
    M5.Lcd.clear();
    printString("Etat initial");
    termPutchar('\r');
    sprintf(text, "Ip du m5 stack: %d", SELF_IP);
    printString(text);
    termPutchar('\r');
  }
  else {
    M5.Lcd.clear();
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
  Serial.print("*****************************\n");
  Serial.printf("Menu data : %d \n",menu_data);
  Serial.printf("etat menu : %d\n",etat_menu);
  Serial.printf("Freq : %d\n",freq);
  Serial.printf("Choix TTL : %d\n",choixTTL);
  Serial.printf("Ipdestm5 : %d\n",ipm5);
  Serial.printf("Val TTL : %d\n",valTTL);
  Serial.printf("Status send : %d\n",status_send);
}

void message_choix_freq(uint16_t etat_menu, uint16_t freq){
  if (etat_menu == 0) {
    sprintf(text, "Frequance : <-- %d -->", freq);
    printString(text);
    termPutchar('\r');
  }
  else {
    sprintf(text, "Frequance : %d", freq);
    printString(text);
    termPutchar('\r');
  }
}

void message_choix_TTL(uint16_t etat_menu, uint16_t choixTTL){
  if (etat_menu == 1){
    if (choixTTL == 0){
      printString("Choix TTL : <-- Pas de TTL + Pas de destination -->");
      termPutchar('\r');
    }
    if (choixTTL == 1){
      printString("Choix TTL : <-- Pas de TTL + Destination -->");
      termPutchar('\r');
    }
    if (choixTTL == 2){
      printString("Choix TTL : <-- TTL + Destination -->");
      termPutchar('\r');
    }
  }
  else {
    if (choixTTL == 0){
      printString("Choix TTL : Pas de TTL + Pas de destination");
      termPutchar('\r');
    }
    if (choixTTL == 1){
      printString("Choix TTL : Pas de TTL + Destination");
      termPutchar('\r');
    }
    if (choixTTL == 2){
      printString("Choix TTL : TTL");
      termPutchar('\r');
    }
  }
}

void message_choix_dest(uint16_t etat_menu, uint16_t ipm5){
  if (etat_menu == 2) {
    sprintf(text, "Destination : <-- %d -->", ipm5);
    printString(text);
    termPutchar('\r');
  }
  else {
    sprintf(text, "Destination : %d", ipm5);
    printString(text);
    termPutchar('\r');
  }
}

void message_choix_valTTL(uint16_t etat_menu, uint16_t valTTL){
  if (etat_menu == 3) {
    sprintf(text, "Valeur TTL : <-- %d -->", valTTL);
    printString(text);
    termPutchar('\r');
  }
  else {
    sprintf(text, "Valeur TTL : %d", valTTL);
    printString(text);
    termPutchar('\r');
  }
}

void message_envoie(uint16_t etat_menu, uint16_t status_send){
  if (etat_menu == 4){
    if (status_send == 0){
      printString("Envoyer : <-- Oui -->");
      termPutchar('\r');
    }
    if (status_send == 1){
      printString("Envoyer : <-- Modifier -->");
      termPutchar('\r');
    }
    if (status_send == 2){
      printString("Envoyer : <-- Non -->");
      termPutchar('\r');
    }
  }
  else {
    if (status_send == 0){
      printString("Envoyer : Oui");
      termPutchar('\r');
    }
    if (status_send == 1){
      printString("Envoyer : Modifier");
      termPutchar('\r');
    }
    if (status_send == 2){
      printString("Envoyer : Non");
      termPutchar('\r');
    }
  }
}

