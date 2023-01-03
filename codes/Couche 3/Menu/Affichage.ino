#include <SPI.h>
#include <RH_RF95.h>
#include <M5Stack.h>

//Fonction d'affichage
void affichage(uint8_t menu_data, uint16_t etat_menu, uint16_t freq, uint16_t send_mode, uint16_t valTTL, uint16_t *dest_ip, uint16_t status_send){
  M5.Lcd.clear();
  if (menu_data == 0) {
    printString(" ==================================================");
    termPutchar('\r');
    printString("                    Mode ecoute");
    termPutchar('\r');
    printString(" ==================================================");
    termPutchar('\r');
    sprintf(text, "Ip du m5 stack: %d.%d", self_ip[0], self_ip[1]);
    printString(text);
    termPutchar('\r');
    sprintf(text, "Frequance d'ecoute: %d", freq);
    printString(text);
    termPutchar('\r');
    termPutchar('\r');
    termPutchar('\r');
    termPutchar('\r');
    termPutchar('\r');
    termPutchar('\r');
  }
  else {
    printString(" ==================================================");
    termPutchar('\r');
    printString("                    Mode d'edition");
    termPutchar('\r');
    printString(" ==================================================");
    termPutchar('\r');
    if (send_mode == 0){
      message_choix_freq(etat_menu, freq);
      message_choix_TTL(etat_menu, send_mode);
      message_envoie(etat_menu, status_send);
      termPutchar('\r');
      termPutchar('\r');
      termPutchar('\r');
    }
    if (send_mode == 1){
      message_choix_freq(etat_menu, freq);
      message_choix_TTL(etat_menu, send_mode);
      message_choix_valTTL(etat_menu, valTTL);
      message_envoie(etat_menu, status_send);
      termPutchar('\r');
      termPutchar('\r');
      termPutchar('\r');
    }
    if (send_mode == 2 || send_mode == 3){
      message_choix_freq(etat_menu, freq);
      message_choix_TTL(etat_menu, send_mode);
      message_choix_valTTL(etat_menu, valTTL);
      message_envoie(etat_menu, status_send);
      termPutchar('\r');
      termPutchar('\r');
    }
    if (send_mode == 4){
      message_choix_freq(etat_menu, freq);
      message_choix_TTL(etat_menu, send_mode);
      message_choix_valTTL(etat_menu, valTTL);
      //message_choix_dest(etat_menu, dest_ip);
      message_envoie(etat_menu, status_send);
      termPutchar('\r');
    }
  }
  termPutchar('\r');
  termPutchar('\r');
  termPutchar('\r');
  printString(" ======= [-] ======= [Validation] ======= [+] =======");
  termPutchar('\r');
  Serial.print("*****************************\n");
  Serial.printf("Menu data : %d \n",menu_data);
  Serial.printf("etat menu : %d\n",etat_menu);
  Serial.printf("Freq : %d\n",freq);
  Serial.printf("Mode d'envoie : %d\n",send_mode);
  Serial.printf("Ip du m5 : %d.%d\n", self_ip[0], self_ip[1]);
  Serial.printf("Ip de destintion du m5 : %d.%d\n", dest_ip[0], dest_ip[1]);
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

void message_choix_TTL(uint16_t etat_menu, uint16_t send_mode){
  if (etat_menu == 1){
    if (send_mode == 0){
      printString("Mode d'envoie : <-- Pas de TTL + Multidiffusion -->");
      termPutchar('\r');
    }
    if (send_mode == 1){
      printString("Mode d'envoie : <-- TTL + Multidiffusion -->");
      termPutchar('\r');
    }
    if (send_mode == 2){
      printString("Mode d'envoie : <-- TTL + Multidiffusion + 1 paquet memoire -->");
      termPutchar('\r');
    }
    if (send_mode == 3){
      printString("Mode d'envoie : <-- TTL + Multidiffusion + 10 paquets memoire -->");
      termPutchar('\r');
    }
    if (send_mode == 4){
      printString("Mode d'envoie : <-- TTL + Destination -->");
      termPutchar('\r');
    }
  }
  else {
    if (send_mode == 0){
      printString("Mode d'envoie : Pas de TTL + Multidiffusion");
      termPutchar('\r');
    }
    if (send_mode == 1){
      printString("Mode d'envoie : TTL + Multidiffusion");
      termPutchar('\r');
    }
    if (send_mode == 2){
      printString("Mode d'envoie : TTL + Multidiffusion + 1 paquet memoire");
      termPutchar('\r');
    }
    if (send_mode == 3){
      printString("Mode d'envoie : TTL + Multidiffusion + 10 paquets memoire");
      termPutchar('\r');
    }
    if (send_mode == 4){
      printString("Mode d'envoie : TTL + Destination");
      termPutchar('\r');
    }
  }
}

void message_choix_valTTL(uint16_t etat_menu, uint16_t valTTL){
  if (etat_menu == 2) {
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

// void message_choix_dest(uint16_t etat_menu, uint16_t *dest_ip){
//   if (etat_menu == 3) {
//     sprintf("Ip de destintion : <-- %d.%d -->\n", dest_ip[0], dest_ip[0]);
//     printString(text);
//     termPutchar('\r');
//   }
//   else {
//     sprintf("Ip de destintion : %d.%d\n", dest_ip[0], dest_ip[1]);
//     printString(text);
//     termPutchar('\r');
//   }
// }

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

