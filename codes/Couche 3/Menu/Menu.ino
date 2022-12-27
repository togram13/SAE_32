#include <SPI.h>
#include <RH_RF95.h>
#include <M5Stack.h>

#define BASE_FREQ 867.7
#define BASE_CHOIX_TTL 0
#define BASE_IP_DEST 5
#define BASE_VAL_TTL 7

#define MAX_FREQ 1000
#define MIN_FREQ 600
#define PAS_FREQ 10 //Valeur d'ajout/de soustraction de fréquance avec les boutons B et C

#define NOMBRE_DESTINATAIRES 20 //Nombre maximum d'adresse dans le réseau

#define TTL_MAX 32 //Nombre TTL max
#define SELF_IP 4 //Ip du M5

char text[255];

uint8_t menu_data = 0; //état du menu : 
  //Si 0 alors menu de base avec juste écrit l'ip 
  //Si 1 alors dans le menu d'envoie
uint16_t etat_menu = 0; //état de modification du menu : 
  //Si 0 alors choix de frequence 
  //Si 1 Choix TTL 
  //Si 2 Choix du destinataire de TTL 
  //Si 3 Choix du nombre de TTL 
  //Si 4 envoie du packet

uint16_t freq=BASE_FREQ;
uint16_t choixTTL=0;
uint16_t ipm5=BASE_IP_DEST;
uint16_t valTTL=BASE_VAL_TTL;
uint16_t status_send=0;

uint16_t self_ip=4; //Valeur à changer par la vraie IP de notre M5 Stack

void Fonction_envoie_data_noTTL_noDest(uint16_t freq, uint16_t choixTTL){
  M5.Lcd.clear(BLACK);
}
void Fonction_envoie_data_noTTL_Dest(uint16_t freq, uint16_t choixTTL, uint16_t ipm5, uint16_t self_ip){
  M5.Lcd.clear(BLACK);
}
void Fonction_envoie_data_TTL_Dest(uint16_t freq, uint16_t choixTTL, uint16_t ipm5, uint16_t valTTL, uint16_t self_ip){
  M5.Lcd.clear(BLACK);
}

void setup(){
  // put your setup code here, to run once:
  //Paramètres par défault
  M5.begin();
  
  sprintf(text, "Ip du m5 stack: %d ", SELF_IP);
  printString(text);
  termPutchar('\r');

}

void loop(){
  M5.update();

  if(M5.BtnA.isPressed()){ //Si le bouton A est pressé
    if (menu_data == 0){ //Si on est sur le menu d'acceuil
      menu_data = 1;
      affichage(menu_data, etat_menu, freq, choixTTL, ipm5, valTTL, status_send);
    }
    else {//sinon
      if (etat_menu == 4){ //Moment d'envoyer le paquet
        if (status_send == 0){ //Correspond à l'état "Envoyer"
          menu_data = 0;
          etat_menu = 0;
          if (choixTTL == 0){
            Fonction_envoie_data_noTTL_noDest(freq, choixTTL);//Envoie la donnée sans TTL ni destination
          }
          if (choixTTL == 1){
            Fonction_envoie_data_noTTL_Dest(freq, choixTTL, ipm5, self_ip);//Envoie la donnée sans TTL avec destination
          }
          if (choixTTL == 2){
            Fonction_envoie_data_TTL_Dest(freq, choixTTL, ipm5, valTTL, self_ip);//Envoie la donnée avec TTL avec destination
          }
          affichage(menu_data, etat_menu, freq, choixTTL, ipm5, valTTL, status_send);// Donc affichage du menu d'acceuil
        }
        if (status_send == 1){ //Correspond à l'état "modification"
          etat_menu = 0;
          affichage(menu_data, etat_menu, freq, choixTTL, ipm5, valTTL, status_send);
        }
        else { //Annule les l'envoie de données
          menu_data = 0;
          etat_menu = 0;
          affichage(menu_data, etat_menu, freq, choixTTL, ipm5, valTTL, status_send);// Donc affichage du menu d'acceuil
        }
      }
      else {
        if (choixTTL == 0 && etat_menu == 1) {
          etat_menu=4;//Renvoie directement à l'envoie du paquet
        }
        if (choixTTL == 1 && etat_menu == 2) {
          etat_menu=4;//Renvoie directement à l'envoie du paquet
        }
        else {
          if (etat_menu < 4){
            etat_menu+=1; //On change de configuration
          }
        }
        affichage(menu_data, etat_menu, freq, choixTTL, ipm5, valTTL, status_send);
      }
    }
    delay(1000);
  }

  if(M5.BtnB.isPressed()){//Si le bouton B est pressé
    if (menu_data == 1){//Si la création d'un packet est active
      if (etat_menu == 0){ //Changement de fréquance
        if (freq+PAS_FREQ < MAX_FREQ ){
          freq+=PAS_FREQ;
          affichage(menu_data, etat_menu, freq, choixTTL, ipm5, valTTL, status_send);
        }
      }
      if (etat_menu == 1){ //Choix du type d'envoie
        if (choixTTL+1 <= 2){
          choixTTL+=1;
          affichage(menu_data, etat_menu, freq, choixTTL, ipm5, valTTL, status_send);
        }
      }
      if (etat_menu == 2){ //Choix du destinataire
        if (ipm5+1 == self_ip){
          if (ipm5+2 <= NOMBRE_DESTINATAIRES){
            ipm5+=2;
            affichage(menu_data, etat_menu, freq, choixTTL, ipm5, valTTL, status_send);
          }
        }
        else{
          if (ipm5+1 <= NOMBRE_DESTINATAIRES){
            ipm5+=1;
            affichage(menu_data, etat_menu, freq, choixTTL, ipm5, valTTL, status_send);
          }
        }
      }
      if (etat_menu == 3){ //Choix du TTL
        if (valTTL+1 <= TTL_MAX){
          valTTL+=1;
          affichage(menu_data, etat_menu, freq, choixTTL, ipm5, valTTL, status_send);
        }
      }
      if (etat_menu == 4){ //Choix d'envoie de la donnée
        if (status_send+1 <= 3){
          status_send+=1;
          affichage(menu_data, etat_menu, freq, choixTTL, ipm5, valTTL, status_send);
        }
      }
    }
    delay(1000);
  }

  if(M5.BtnC.isPressed()){//Si le bouton C est pressé
    if (menu_data == 1){//Si la création d'un packet est active
      if (etat_menu == 0){ //Changement de fréquance
        if (freq-PAS_FREQ > MIN_FREQ ){
          freq-=PAS_FREQ;
          affichage(menu_data, etat_menu, freq, choixTTL, ipm5, valTTL, status_send);
        }
      }
      if (etat_menu == 1){ //Choix du type d'envoie
        if (choixTTL-1 >= 0){
          choixTTL-=1;
          affichage(menu_data, etat_menu, freq, choixTTL, ipm5, valTTL, status_send);
        }
      }
      if (etat_menu == 2){ //Choix du destinataire
        if (ipm5-1 == self_ip){
          if (ipm5-2 >= 0){
            ipm5-=2;
            affichage(menu_data, etat_menu, freq, choixTTL, ipm5, valTTL, status_send);
          }
        }
        else{
          if (ipm5-1 >= 0){
            ipm5-=1;
            affichage(menu_data, etat_menu, freq, choixTTL, ipm5, valTTL, status_send);
          }
        }
      }
      if (etat_menu == 3){ //Choix du TTL
        if (valTTL-1 >= 0){
          valTTL-=1;
          affichage(menu_data, etat_menu, freq, choixTTL, ipm5, valTTL, status_send);
        }
      }
      if (etat_menu == 4){ //Choix d'envoie de la donnée
        if (status_send-1 >= 0){
          status_send-=1;
          affichage(menu_data, etat_menu, freq, choixTTL, ipm5, valTTL, status_send);
        }
      }
    }
    delay(1000);
  }
}
