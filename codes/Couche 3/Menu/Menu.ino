#include <SPI.h> //Nativement dans arduino donc pas besoin de l'installer en +
#include <RH_RF95.h> //gestion des transmissions radio en Lora
#include <M5Stack.h>

#define RFM95_CS 5 //M5Stack Lora
#define RFM95_DIO0 36
RH_RF95 rf95(RFM95_CS, RFM95_DIO0);

#define BASE_FREQ 867.7
#define BASE_CHOIX_TTL 0
#define BASE_IP_DEST {10,5}// Ip de destination sous forme {<réseau>,<id du m5>}
#define BASE_VAL_TTL 7

#define MAX_FREQ 1000
#define MIN_FREQ 600
#define PAS_FREQ 10 //Valeur d'ajout/de soustraction de fréquance avec les boutons B et C

#define NOMBRE_DESTINATAIRES 20 //Nombre maximum d'adresse dans le réseau

#define TTL_MAX 32 //Nombre TTL max
#define SELF_IP {10,4} //Ip du M5

char text[255], temp[255];

uint8_t menu_data = 0; //état du menu : 
  //Si 0 alors menu de base avec juste écrit l'ip 
  //Si 1 alors dans le menu d'envoie
  //Si 2 alors reception d'une donnée
uint16_t etat_menu = 0; //état de modification du menu : 
  //Si 0 alors choix de frequence 
  //Si 1 Choix TTL 
  //Si 2 Choix du nombre de TTL
  //Si 3 Choix du destinataire de TTL  
  //Si 4 envoie du packet

uint16_t freq=BASE_FREQ;
uint16_t send_mode=0;
uint16_t ipm5[2]=BASE_IP_DEST;
uint16_t valTTL=BASE_VAL_TTL;
uint16_t status_send=0;
int modifreq=0; //permet de supprimer un bug de double clic au niveau de la modification de fréquance

uint16_t self_ip[2]=SELF_IP;//{10,4}; //Valeur à changer par la vraie IP de notre M5 Stack

int Seq = 0, SeqR = 0, i, d=0;
int ttl =1 ;
 
uint8_t derniere_source, source;
int derniere_sequence, sequence;
uint8_t src, seq;
 
int id = 0;
int mode;
 
int isPacketResul;
uint8_t rxbuf[RH_RF95_MAX_MESSAGE_LEN], rxbuflen = RH_RF95_MAX_MESSAGE_LEN;
uint8_t txbuf[RH_RF95_MAX_MESSAGE_LEN], txbuflen = RH_RF95_MAX_MESSAGE_LEN;

uint8_t state; // Implémentation d'un switch pour les différents modes
#define E0 0 // mode de réception 2 (tempête de broadcast avec réémission)
#define E1 1 // limitation avec ttl (mode de réception 3)
#define E2 2 // contrôle avec adresse source + numéro de séquence par rapport au dernier paquet routé (mode de réception 5)
#define E3 3 // contrôle avec adresse source + numéro de séquence par rapport au 10 derniers paquets routé (mode de réception 6)

struct identifiant{
 
    uint16_t srcAddr;
    uint8_t sqn;
};
 
struct identifiant tableau[10];
 
int isPacketRouted(struct identifiant tab[10], uint16_t source, uint8_t sequence, int *index){
  int i;
  for(i=0; i<11; i++){
    if(tab[i].srcAddr == source && tab[i].sqn == sequence){
      return 1;
      break;
    }
    else{
      if(*index >= 10){
        printString("Index est trop grand\r");

        *index = 0;
        printString("On rentre le couple dans la case : ");
        sprintf(temp, "%u", *index);
        printString(temp);
        termPutchar('\r');

        tab[*index].srcAddr = source;
        tab[*index].sqn = sequence;
        return 0;
        break;
      }
      else{
        printString("index est OK \r");
        printString("On rentre le couple dans la case : ");
        sprintf(temp, "%u", *index);
        printString(temp);
        termPutchar('\r');

        tab[*index].srcAddr = source;
        tab[*index].sqn = sequence;

        *index = *index +1;
        return 0;
        break;
      }
    }
  }
}

void setup(){
  M5.Power.begin(); //initialisation de la puissance
  termInit();
  M5.begin(9600); //règle le débit du M5 à 9600 bauds ( = 9600 b/s)
  Serial.begin(115200);

  M5.Lcd.setTextColor(WHITE, TFT_BLACK);

  //initialisation du transceiver radio:
 
  if(!rf95.init()) //init règle la puissance à 13dBm
    printString("Erreur initialisation RF95\r"); // on vérifie si il y a une erreur avec le transceiver
                                                // Problèmes possibles : cablages ou drivers logiciels
  else
    printString("Initialisation OK\r");
  
  //configuration des parametres Radio :
  rf95.setModemConfig(RH_RF95::Bw125Cr45Sf128); //Selectionne des config de modems prédéfinis

  delay(1000);
  affichage(menu_data, etat_menu, freq, send_mode, valTTL, ipm5, status_send);
}

void loop(){
  M5.update();

  if(M5.BtnA.isPressed()){//Si le bouton A est pressé (correspond au -)
    if (menu_data == 1){//Si la création d'un packet est active
      if (etat_menu == 0){ //Changement de fréquance
        if (freq-PAS_FREQ > MIN_FREQ ){
          freq-=PAS_FREQ;
          affichage(menu_data, etat_menu, freq, send_mode, valTTL, ipm5, status_send);
        }
      }
      if (etat_menu == 1){ //Choix du type d'envoie
        if (send_mode-1 >= 0){
          send_mode-=1;
          affichage(menu_data, etat_menu, freq, send_mode, valTTL, ipm5, status_send);
        }
      }
      if (etat_menu == 2){ //Choix du TTL
        if (valTTL-1 >= 0){
          valTTL-=1;
          affichage(menu_data, etat_menu, freq, send_mode, valTTL, ipm5, status_send);
        }
      }
      if (etat_menu == 3){ //Choix du destinataire
        if (ipm5[1]-1 == self_ip[1]){
          if (ipm5[1]-2 >= 0){
            ipm5[1]-=2;
            affichage(menu_data, etat_menu, freq, send_mode, valTTL, ipm5, status_send);
          }
        }
        else{
          if (ipm5[1]-1 >= 0){
            ipm5[1]-=1;
            affichage(menu_data, etat_menu, freq, send_mode, valTTL, ipm5, status_send);
          }
        }
      }
      if (etat_menu == 4){ //Choix d'envoie de la donnée
        if (status_send-1 >= 0){
          status_send-=1;
          affichage(menu_data, etat_menu, freq, send_mode, valTTL, ipm5, status_send);
        }
      }
    }
    if (menu_data == 0){
      if (freq-PAS_FREQ > MIN_FREQ ){
        freq-=PAS_FREQ;
        affichage(menu_data, etat_menu, freq, send_mode, valTTL, ipm5, status_send);
      }
    }
    delay(1000);
  }

  if(M5.BtnB.isPressed()){ //Si le bouton B est pressé (correspond à la validation d'un action)
    if (menu_data == 0){ //Si on est sur le menu d'acceuil
      menu_data = 1;
      affichage(menu_data, etat_menu, freq, send_mode, valTTL, ipm5, status_send);
    }
    if (menu_data == 2){
      menu_data = 0;
      affichage(menu_data, etat_menu, freq, send_mode, valTTL, ipm5, status_send);
    }
    else {//sinon
      if (etat_menu == 4){ //Moment d'envoyer le paquet
        if (status_send == 0){ //Correspond à l'état "Envoyer"
          menu_data = 0;
          etat_menu = 0;
          if (send_mode == 0){
            Fonction_envoie_data_noTTL_noDest(freq);//Envoie la donnée sans TTL ni destination
          }
          if (send_mode == 1){
            Fonction_envoie_data_TTL_noDest(freq, valTTL);//Envoie la donnée avec TTL en monodiffusion
          }
          if (send_mode == 2){
            Fonction_envoie_data_TTL_LittleNet(freq, valTTL, self_ip);//Envoie la donnée avec TTL avec destination
          }
          delay(5000);
          affichage(menu_data, etat_menu, freq, send_mode, valTTL, ipm5, status_send);// Donc affichage du menu d'acceuil
        }
        if (status_send == 1){ //Correspond à l'état "modification"
          etat_menu = 0;
          affichage(menu_data, etat_menu, freq, send_mode, valTTL, ipm5, status_send);
        }
        else { //Annule les l'envoie de données
          menu_data = 0;
          etat_menu = 0;
          affichage(menu_data, etat_menu, freq, send_mode, valTTL, ipm5, status_send);// Donc affichage du menu d'acceuil
        }
      }
      else {
        if (send_mode == 0 && etat_menu == 1) {
          etat_menu=4;//Renvoie directement à l'envoie du paquet
        }
        if ((send_mode == 1 || send_mode == 2) && etat_menu == 2) {
          etat_menu=4;//Renvoie directement à l'envoie du paquet
        }
        else {
          if ((etat_menu > 0 && etat_menu < 4) || modifreq==1){
            etat_menu+=1; //On change de configuration
            modifreq=0;
          }
        }
        if (etat_menu == 0){
          if (modifreq==0){
            modifreq=1;
          }
        }
        affichage(menu_data, etat_menu, freq, send_mode, valTTL, ipm5, status_send);
      }
    }
    delay(1000);
  }

  if(M5.BtnC.isPressed()){//Si le bouton C est pressé (corrsepond au +)
    if (menu_data == 1){//Si la création d'un packet est active
      if (etat_menu == 0){ //Changement de fréquance
        if (freq+PAS_FREQ < MAX_FREQ ){
          freq+=PAS_FREQ;
          affichage(menu_data, etat_menu, freq, send_mode, valTTL, ipm5, status_send);
        }
      }
      if (etat_menu == 1){ //Choix du type d'envoie
        if (send_mode+1 <= 3){
          send_mode+=1;
          affichage(menu_data, etat_menu, freq, send_mode, valTTL, ipm5, status_send);
        }
      }
      if (etat_menu == 2){ //Choix du TTL
        if (valTTL+1 <= TTL_MAX){
          valTTL+=1;
          affichage(menu_data, etat_menu, freq, send_mode, valTTL, ipm5, status_send);
        }
      }
      if (etat_menu == 3){ //Choix du destinataire
        if (ipm5[1]+1 == self_ip[1]){
          if (ipm5[1]+2 <= NOMBRE_DESTINATAIRES){
            ipm5[1]+=2;
            affichage(menu_data, etat_menu, freq, send_mode, valTTL, ipm5, status_send);
          }
        }
        else{
          if (ipm5[1]+1 <= NOMBRE_DESTINATAIRES){
            ipm5[1]+=1;
            affichage(menu_data, etat_menu, freq, send_mode, valTTL, ipm5, status_send);
          }
        }
      }
      if (etat_menu == 4){ //Choix d'envoie de la donnée
        if (status_send+1 <= 3){
          status_send+=1;
          affichage(menu_data, etat_menu, freq, send_mode, valTTL, ipm5, status_send);
        }
      }
    } else {
      if (menu_data == 0){
        if (freq+PAS_FREQ < MAX_FREQ ){
          freq+=PAS_FREQ;
          affichage(menu_data, etat_menu, freq, send_mode, valTTL, ipm5, status_send);
        }
      }
    }
    delay(1000);
  }

  //Partie reception d'un message
  rf95.setModeRx(); //Passage en mode recepteur

  if (rf95.recv(rxbuf, &rxbuflen)) { // Si il y a quelque chose dans le buffeur, alors:
    menu_data = 2;
    printString(" ==================================================");
    termPutchar('\r');
    printString("               Reception d'une donnee");
    termPutchar('\r');
    printString(" ==================================================");
    termPutchar('\r');
    mode = rxbuf[0];
    if(mode = 0){
      state = 0; //corespond au mode 2
    }
    else if(mode = 1){//corespond au mode 3
      state = 1;
    }
    else if(mode = 2){//corespond au mode 5
      state = 2;
    }
    else if(mode = 3){//corespond au mode 6
      state = 3;
    }

    switch (state){
      case E0: //corespond au mode 2: emission d'un paquet sans TTL et avec une destination mulitcast
        printString("reception du paquet : ");
        sprintf(temp, "%u", Seq);
        printString(temp);
        termPutchar('\r');
          
        if(rxbuf[0]==255 && rxbuf[1]==255){
          
          printString("L'adresse destination est bien une adresse multicast");
          termPutchar('\r');
          termPutchar('\r');

          // affichage de la trame
          for (i = 0; i<4; i++) {

            // Affichage de la trame envoyée (De tous les octets) :
            printString(" | ");
            sprintf(temp, "%u", rxbuf[i]);
            printString(temp);

          }
          Seq = Seq +1;
          // Boucle pour créer la trame de réemission
          for (i = 0; i<4; i++) {
          // création de la trame d'envoi:
            txbuf[i] = rxbuf[i];
          }
          termPutchar('\r');
          printString("Reemission du paquet\r");

          rf95.send(txbuf, 4);    // emission
          Seq = Seq +1;  
          rf95.waitPacketSent();
        }
        else {
          printString("Probleme avec l'adresse destination, ca n'est pas une adresse multicast");
          termPutchar('\r\r');
        }
        termPutchar('\r');
        termPutchar('\r');
        break;

      case E1: //corespond au mode 3: emission d'un paquet avec TTL et avec une destination mulitcast
        printString("reception  ");
        sprintf(temp, "%u", Seq);
        printString(temp);
        termPutchar('\r');
        
        if(rxbuf[0]==255 && rxbuf[1]==255){
          
          printString("L'adresse destination est une adresse multicast");
          termPutchar('\r');
          termPutchar('\r');

          printString("Paquet recu :  ");
          termPutchar('\r');

            // affichage du paquet
            for (i = 0; i<5; i++) {

              // Affichage du paquet récupéré :
              printString(" | ");
              sprintf(temp, "%u", rxbuf[i]);
              printString(temp);
            }
          ttl = rxbuf[4];

          if(ttl >= TTL_MAX){
            termPutchar('\r');
            printString("TTL trop grand => destruction du paquet\r");
            termPutchar('\r');

            ttl = 1;
          }
          else{
            Seq = Seq +1;

            // Boucle pour créer le paquet de réemission
            for (i = 0; i<5; i++) {
            // création de la trame d'envoi:
              txbuf[i] = rxbuf[i];
            }
            termPutchar('\r');


            //incrémentation du TTL de 1 à la réemission
            
            txbuf[4] = ttl+1;

            termPutchar('\r');
            printString("TTL : ");  
            sprintf(temp, "%u", ttl);
            printString(temp);
            termPutchar('\r');
            termPutchar('\r');

            printString("Reemission du paquet :  ");
            termPutchar('\r');

            // affichage du paquet d'envoi
            for (i = 0; i<5; i++) {
            // Affichage de la trame récupérée
              printString(" | ");
              sprintf(temp, "%u", txbuf[i]);
              printString(temp);
            }

            rf95.send(txbuf, 5);    // emission
            Seq = Seq +1;  
            rf95.waitPacketSent();
          }
        }
        else{
          printString("Probleme avec l'adresse destination, ca n'est pas une adresse multicast");
          termPutchar('\r');
          termPutchar('\r');
        }
        termPutchar('\r');
        termPutchar('\r');
        break;

      case E2:
        sequence = rxbuf[5]; // Sequence du paquet actuel
        source = rxbuf[3]; // Adresse source du paquet actuel
    
        printString("reception paquet n° ");
        sprintf(temp, "%u", sequence);
        printString(temp);
        termPutchar('\r');

        printString("Adresse source : ");

        sprintf(temp, "%u", rxbuf[3]);
        printString(temp);

        sprintf(temp, "%u", rxbuf[4]);
        printString(temp);
        termPutchar('\r');

      
        if(derniere_sequence == sequence && derniere_source == source)
        {
          printString("Ce paquet a deja ete reemis => destruction du paquet\n");
          termPutchar('\r');
        }
        else{

          derniere_sequence = sequence;
          derniere_source = source;
          
          if(rxbuf[0]==255 && rxbuf[1]==255){
            
            printString("L'adresse destination est bien une adresse multicast");
            termPutchar('\r');
            termPutchar('\r');

            printString("Paquet recu:  ");
            termPutchar('\r');

              // affichage du paquet
              for (i = 0; i<8; i++) {

                // Affichage du paquet récupéré :
                printString(" | ");
                sprintf(temp, "%u", rxbuf[i]);
                printString(temp);
              }
            ttl = rxbuf[-1];

            if(ttl >= TTL_MAX){
              termPutchar('\r');
            printString("[paquet n° ");
            sprintf(temp, "%u", sequence);
            printString(temp);

            printString("] TTL :: ");
            sprintf(temp, "%u", ttl);
            printString(temp);

            termPutchar('\r');
            printString("TTL trop grand => destruction du paquet\r");
            termPutchar('\r');

              ttl = 1;
            }
            else{            

              // création du paquet d'envoi:
              for (i = 0; i<8; i++) {
                txbuf[i] = rxbuf[i];
              }
              termPutchar('\r');
  

              //incrémentation du TTL de 1 à la réemission
              
              txbuf[2] = ttl+1;

              termPutchar('\r');
              
              printString("[paquet n° ");
              sprintf(temp, "%u", sequence);
              printString(temp);

              printString("] TTL :: ");
              sprintf(temp, "%u", ttl);
              printString(temp);

              termPutchar('\r');

              printString("Reemission du paquet:  ");

              // affichage du paquet d'envoi
              for (i = 0; i<8; i++) {
              // Affichage de la trame récupérée
                printString(" | ");
                sprintf(temp, "%u", txbuf[i]);
                printString(temp);
              }

              rf95.send(txbuf, 8);    // emission  
              rf95.waitPacketSent();
            }

          }
          else{
            printString("Probleme avec l'adresse destination, ca n'est pas une adresse multicast");
            termPutchar('\r');
            termPutchar('\r');
          }
          termPutchar('\r');
          termPutchar('\r');
        }
        break;

      case E3:
        seq = rxbuf[5]; // Sequence du paquet actuel
        src = rxbuf[3]; // Adresse source du paquet actuel
      
        printString("reception paquet n° ");
        sprintf(temp, "%u", seq);
        printString(temp);
        termPutchar('\r');

        printString("Adresse source : ");
        sprintf(temp, "%u", src);
        printString(temp);
        printString(temp);
        termPutchar('\r');

        
        isPacketResul = isPacketRouted(tableau, src, seq, &id);

        if(isPacketResul == 1){
          printString("Paquet deja route => destruction du paquet");
          termPutchar('\r');
        }
        else{
          if(rxbuf[0]==255 && rxbuf[1]==255){
            
            printString("L'adresse destination est bien une adresse multicast");
            termPutchar('\r');
            termPutchar('\r');

            printString("Paquet recu:  ");
              // affichage du paquet
              for (i = 0; i<8; i++) {

                // Affichage du paquet récupéré :
                printString(" | ");
                sprintf(temp, "%u", rxbuf[i]);
                printString(temp);
              }
            ttl = rxbuf[-1];

            if(ttl >= TTL_MAX){
              termPutchar('\r');
              printString("[paquet n° ");
              sprintf(temp, "%u", seq);
              printString(temp);

              printString("] TTL :: ");
              sprintf(temp, "%u", ttl);
              printString(temp);

              termPutchar('\r');
              printString("TTL trop grand => destruction du paquet\r");
              termPutchar('\r');

              ttl = 1;
            }
            else{
              // Boucle pour créer le paquet de réemission
              for (i = 0; i<8; i++) {
              // création de la trame d'envoi:
                txbuf[i] = rxbuf[i];
              }
              termPutchar('\r');
              //incrémentation du TTL de 1 à la réemission
              
              txbuf[2] = ttl+1;

              termPutchar('\r');
              termPutchar('\r');
              printString("[paquet n° ");
              sprintf(temp, "%u", seq);
              printString(temp);

              printString("] TTL :: ");
              sprintf(temp, "%u", ttl);
              printString(temp);

              termPutchar('\r');
              printString("Reemission du paquet:  ");

              // affichage du paquet d'envoi
              for (i = 0; i<8; i++) {
              // Affichage de la trame récupérée
                printString(" | ");
                sprintf(temp, "%u", txbuf[i]);
                printString(temp);
              }

              rf95.send(txbuf, 8);    // emission  
              rf95.waitPacketSent();
            }
          }
          else{
            printString("Probleme avec l'adresse destination, ca n'est pas une adresse multicast");
            termPutchar('\r');
            termPutchar('\r');
          }
          termPutchar('\r');
          termPutchar('\r');
        }
        break;      
    }
    printString("[Bouton central pour retourner au menu principal]");
  }


}
