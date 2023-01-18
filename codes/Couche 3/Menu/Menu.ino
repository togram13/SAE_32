// -----------------------------------------------------------------------------------------------------------------------
// | Pour faire fonctionner le programe, une fois le programme téléversé sur les m5,                                     |
// | il faut dans un premier temps envoyer un paquet quelconque (permier mode de transmission par exemple) sur chaque M5,|
// | Normalement, quand on fait la manipulation sur le deuxième M5,                                                      |
// | on devrait voir apparaître la réception d'un paquet sur le premier m5.                                              |
// -----------------------------------------------------------------------------------------------------------------------
#include <SPI.h> //Nativement dans arduino donc pas besoin de l'installer en +
#include <RH_RF95.h> //gestion des transmissions radio en Lora
#include <M5Stack.h>

#define RFM95_CS 5 //M5Stack Lora
#define RFM95_DIO0 36
RH_RF95 rf95(RFM95_CS, RFM95_DIO0);

#define BASE_FREQ 800
#define BASE_IP_DEST {10,5}// Adresse de destination initiale sous forme {<réseau>,<id du m5>}
#define BASE_VAL_TTL 7

#define MAX_FREQ 868 //Valeur maximum de la bande ISM
#define MIN_FREQ 433 //Valeur minimum de la bande ISM
#define PAS_FREQ 10 //Valeur d'ajout/de soustraction de fréquence avec les boutons B et C

#define NOMBRE_DESTINATAIRES 20 //Nombre maximum d'adresse dans le réseau

#define TTL_MAX 32 //Nombre TTL maximum
#define SELF_IP {10,6} //Adresse du M5 sous forme {<réseau>,<id du m5>}

char text[255], temp[255];

uint8_t menu_data = 0; //état du menu : 
  //Si 0 alors menu de base avec juste écrit l'adresse et la fréquence 
  //Si 1 alors dans le menu d'envoie
  //Si 2 alors reception d'une donnée
uint16_t etat_menu = 0; //état de modification du menu : 
  //Si 0 alors choix de frequence 
  //Si 1 Choix TTL 
  //Si 2 Choix du nombre de TTL
  //Si 3 Choix du destinataire 
  //Si 4 envoie du packet

uint16_t freq=BASE_FREQ;
uint16_t send_mode=0;
uint16_t dest_ip[2]=BASE_IP_DEST;
uint16_t max_TTL=BASE_VAL_TTL;
uint16_t status_send=0;
int modifreq=0; //permet de supprimer un bug de double clic au niveau de la modification de fréquance

uint16_t self_ip[2]=SELF_IP; //Valeur à changer par la vraie IP de notre M5 Stack

int Seq = 0, SeqR = 0, i, d=0;
int ttl =1 ;
 
uint8_t derniere_source[2], source[2];
int derniere_sequence, sequence;
uint8_t src[2], src_em;
uint8_t seq, seq_em;
 
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
#define E4 4 // limitation avec ttl et envoie un paquet vers une destination

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
  rf95.setFrequency(freq); //règle la fréquence d'emission avec la valeure de base

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
  affichage(menu_data, etat_menu, freq, send_mode, max_TTL, dest_ip, status_send);
}

void loop(){
  M5.update();

  if(M5.BtnA.isPressed()){//Si le bouton A est pressé (Correspond au -)
    if (menu_data == 1){//Si la création d'un packet est active
      if (etat_menu == 0){ //Changement de fréquance
        if (freq-PAS_FREQ > MIN_FREQ ){
          freq-=PAS_FREQ;
          affichage(menu_data, etat_menu, freq, send_mode, max_TTL, dest_ip, status_send);
        }
      }
      if (etat_menu == 1){ //Choix du type d'envoie
        if (send_mode-1 >= 0){
          send_mode-=1;
          affichage(menu_data, etat_menu, freq, send_mode, max_TTL, dest_ip, status_send);
        }
      }
      if (etat_menu == 2){ //Choix du TTL
        if (max_TTL-1 >= 0){
          max_TTL-=1;
          affichage(menu_data, etat_menu, freq, send_mode, max_TTL, dest_ip, status_send);
        }
      }
      if (etat_menu == 3){ //Choix du destinataire
        if (dest_ip[1]-1 == self_ip[1]){
          if (dest_ip[1]-2 >= 0){
            dest_ip[1]-=2;
            affichage(menu_data, etat_menu, freq, send_mode, max_TTL, dest_ip, status_send);
          }
        }
        else{
          if (dest_ip[1]-1 >= 0){
            dest_ip[1]-=1;
            affichage(menu_data, etat_menu, freq, send_mode, max_TTL, dest_ip, status_send);
          }
        }
      }
      if (etat_menu == 4){ //Choix d'envoie de la donnée
        if (status_send-1 >= 0){
          status_send-=1;
          affichage(menu_data, etat_menu, freq, send_mode, max_TTL, dest_ip, status_send);
        }
      }
    }
    if (menu_data == 0){
      if (freq-PAS_FREQ > MIN_FREQ ){
        freq-=PAS_FREQ;
        affichage(menu_data, etat_menu, freq, send_mode, max_TTL, dest_ip, status_send);
      }
    }
    delay(1000);
  }

  if(M5.BtnB.isPressed()){ //Si le bouton B est pressé (Correspond à la validation d'un action)
    if (menu_data == 0){ //Si on est sur le menu d'acceuil
      menu_data = 1;
      affichage(menu_data, etat_menu, freq, send_mode, max_TTL, dest_ip, status_send);
    }
    if (menu_data == 2){
      menu_data = 0;
      affichage(menu_data, etat_menu, freq, send_mode, max_TTL, dest_ip, status_send);
    }
    else {//sinon
      if (etat_menu == 4){ //Moment d'envoyer le paquet
        if (status_send == 0){ //Correspond à l'état "Envoyer"
          menu_data = 0;
          etat_menu = 0;
          if (send_mode == 0){
            Fonction_envoie_data_noTTL_noDest(send_mode, freq);//Envoie la donnée sans TTL en multidiffusion
          }
          if (send_mode == 1){
            Fonction_envoie_data_TTL_noDest(send_mode, freq);//Envoie la donnée avec TTL en multidiffusion
          }
          if (send_mode == 2){
            Fonction_envoie_data_TTL_LittleNet(send_mode, freq, self_ip);//Envoie la donnée avec TTL en multidiffusion (1 paquet retenu)
          }
          if (send_mode == 3){
            Fonction_envoie_data_TTL_BigNet(send_mode, freq, self_ip);//Envoie la donnée avec TTL en multidiffusion (10 paquets retenus)
          }
          if (send_mode == 4){
            Fonction_envoie_data_TTL_Dest(send_mode, freq, self_ip, dest_ip);//Envoie la donnée avec TTL en monodiffusion
          }
          delay(5000);
          affichage(menu_data, etat_menu, freq, send_mode, max_TTL, dest_ip, status_send);// Donc affichage du menu d'acceuil
        }
        if (status_send == 1){ //Correspond à l'état "modification"
          etat_menu = 0;
          affichage(menu_data, etat_menu, freq, send_mode, max_TTL, dest_ip, status_send);
        }
        else { //Annule les l'envoie de données
          menu_data = 0;
          etat_menu = 0;
          affichage(menu_data, etat_menu, freq, send_mode, max_TTL, dest_ip, status_send);// Donc affichage du menu d'acceuil
        }
      }
      else {
        if (send_mode == 0 && etat_menu == 1) {
          etat_menu=4;//Renvoie directement à l'envoie du paquet
        }
        if ((send_mode == 1 || send_mode == 2 || send_mode == 3) && etat_menu == 2) {
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
        affichage(menu_data, etat_menu, freq, send_mode, max_TTL, dest_ip, status_send);
      }
    }
    delay(1000);
  }

  if(M5.BtnC.isPressed()){//Si le bouton C est pressé (corrsepond au +)
    if (menu_data == 1){//Si la création d'un packet est active
      if (etat_menu == 0){ //Changement de fréquance
        if (freq+PAS_FREQ < MAX_FREQ ){
          freq+=PAS_FREQ;
          affichage(menu_data, etat_menu, freq, send_mode, max_TTL, dest_ip, status_send);
        }
      }
      if (etat_menu == 1){ //Choix du type d'envoie
        if (send_mode+1 <= 4){
          send_mode+=1;
          affichage(menu_data, etat_menu, freq, send_mode, max_TTL, dest_ip, status_send);
        }
      }
      if (etat_menu == 2){ //Choix du TTL
        if (max_TTL+1 <= TTL_MAX){
          max_TTL+=1;
          affichage(menu_data, etat_menu, freq, send_mode, max_TTL, dest_ip, status_send);
        }
      }
      if (etat_menu == 3){ //Choix du destinataire
        if (dest_ip[1]+1 == self_ip[1]){
          if (dest_ip[1]+2 <= NOMBRE_DESTINATAIRES){
            dest_ip[1]+=2;
            affichage(menu_data, etat_menu, freq, send_mode, max_TTL, dest_ip, status_send);
          }
        }
        else{
          if (dest_ip[1]+1 <= NOMBRE_DESTINATAIRES){
            dest_ip[1]+=1;
            affichage(menu_data, etat_menu, freq, send_mode, max_TTL, dest_ip, status_send);
          }
        }
      }
      if (etat_menu == 4){ //Choix d'envoie de la donnée
        if (status_send+1 <= 3){
          status_send+=1;
          affichage(menu_data, etat_menu, freq, send_mode, max_TTL, dest_ip, status_send);
        }
      }
    } else {
      if (menu_data == 0){
        if (freq+PAS_FREQ < MAX_FREQ ){
          freq+=PAS_FREQ;
          affichage(menu_data, etat_menu, freq, send_mode, max_TTL, dest_ip, status_send);
        }
      }
    }
    delay(1000);
  }

  //Partie reception d'un message
  rf95.setModeRx(); //Passage en mode recepteur
  delay(500);

  if (rf95.recv(rxbuf, &rxbuflen)) { // Si il y a quelque chose dans le buffeur, alors:
    menu_data = 2;
    printString(" ==================================================");
    termPutchar('\r');
    printString("               Reception d'une donnee !");
    termPutchar('\r');
    printString(" ==================================================");
    termPutchar('\r');
    mode = rxbuf[0];
    if(mode == 0){
      state = E0; //Correspond au code du TP-NWK-Etudiant 4.2
    }
    if(mode == 1){//Correspond au code du TP-NWK-Etudiant 4.3
      state = E1;
    }
    if(mode == 2){//Correspond au code du TP-NWK-Etudiant 4.5
      state = E2;
    }
    if(mode == 3){//Correspond au code du TP-NWK-Etudiant 4.6
      state = E3;
    }
    if(mode == 4){//Correspond a un mode monodiffusion sur les bases du code du TP-NWK-Etudiant 4.3
      state = E4;
    }
    switch (state){
      case E0: //Correspond au code du TP-NWK-Etudiant 4.2: emission d'un paquet sans TTL et avec une destination mulitcast
        sprintf(temp, "Reception du paquet : %u", Seq);
        printString(temp);
        termPutchar('\r');
        termPutchar('\r');
          
        if(rxbuf[1]==255 && rxbuf[2]==255){
          
          printString("L'adresse destination est bien une adresse multicast");
          termPutchar('\r');
          termPutchar('\r');

          // affichage de la trame
          for (i = 0; i<5; i++) {
            // Affichage de la trame envoyée (De tous les octets) :
            printString(" | ");
            sprintf(temp, "%u", rxbuf[i]);
            printString(temp);
          }
          Seq = Seq +1;
          // Boucle pour créer la trame de réemission
          for (i = 0; i<5; i++) {
          // création de la trame d'envoi:
            txbuf[i] = rxbuf[i];
          }
          termPutchar('\r');
          printString("Reemission du paquet\r");
          termPutchar('\r');
          termPutchar('\r');
          rf95.send(txbuf, 5);    // emission
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

      case E1: //corespond au code du TP-NWK-Etudiant 4.3: emission d'un paquet avec TTL et avec une destination mulitcast
        sprintf(temp, "Reception du paquet : %u", Seq);
        printString(temp);
        termPutchar('\r');
        
        if(rxbuf[1]==255 && rxbuf[2]==255){
          
          printString("L'adresse destination est une adresse multicast");
          termPutchar('\r');
          termPutchar('\r');

          printString("Paquet recu :  ");
          termPutchar('\r');

            // affichage du paquet
            for (i = 0; i<6; i++) {

              // Affichage du paquet récupéré :
              printString(" | ");
              sprintf(temp, "%u", rxbuf[i]);
              printString(temp);
            }
          ttl = rxbuf[5];

          if(ttl >= max_TTL){
            termPutchar('\r');
            printString("TTL trop grand => destruction du paquet\r");
            termPutchar('\r');

            ttl = 1;
          }
          else{
            Seq = Seq +1;

            // Boucle pour créer le paquet de réemission
            for (i = 0; i<6; i++) {
            // création de la trame d'envoi:
              txbuf[i] = rxbuf[i];
            }
            termPutchar('\r');

            //incrémentation du TTL de 1 à la réemission
            
            txbuf[5] = ttl+1;

            termPutchar('\r');
            printString("TTL : ");  
            sprintf(temp, "%u", ttl);
            printString(temp);
            termPutchar('\r');
            termPutchar('\r');

            printString("Reemission du paquet :  ");
            termPutchar('\r');

            // affichage du paquet d'envoi
            for (i = 0; i<6; i++) {
            // Affichage de la trame récupérée
              printString(" | ");
              sprintf(temp, "%u", txbuf[i]);
              printString(temp);
            }
            rf95.send(txbuf, 6);    // emission
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

      case E2: //Correspond au code du TP-NWK-Etudiant 4.5
        sequence = rxbuf[6]; // Sequence du paquet actuel
        source[0] = rxbuf[4]; // Adresse source du paquet actuel
        source[1] = rxbuf[5]; // Adresse source du paquet actuel
    
        sprintf(temp, "Reception paquet n° %u", sequence);
        printString(temp);
        termPutchar('\r');

        sprintf(temp, "Adresse source : %u.%u", source[0], source[1]);
        printString(temp);
        termPutchar('\r');

        if(derniere_sequence == sequence && derniere_source[0] == source[0] && derniere_source[1] == source[1]) {
          printString("Ce paquet a deja ete reemis => destruction du paquet\n");
          termPutchar('\r');
          termPutchar('\r');
          termPutchar('\r');
          termPutchar('\r');
          termPutchar('\r');
          termPutchar('\r');
          termPutchar('\r');
        }
        else{

          derniere_sequence = sequence;
          derniere_source[0] = source[0];
          derniere_source[1] = source[1];
          
          if(rxbuf[1]==255 && rxbuf[2]==255){
            
            printString("L'adresse destination est bien une adresse multicast");
            termPutchar('\r');

            printString("Paquet recu:");
            termPutchar('\r');
            termPutchar('\r');
            // affichage du paquet
            for (i = 0; i<9; i++) {

              // Affichage du paquet récupéré :
              printString(" | ");
              sprintf(temp, "%u", rxbuf[i]);
              printString(temp);
            }
            ttl = rxbuf[3];

            if(ttl >= max_TTL){
            sprintf(temp, "[paquet n°%u] TTL :: %u", sequence, ttl);
            printString(temp);

            termPutchar('\r');
            printString("TTL trop grand => destruction du paquet\r");
            termPutchar('\r');

              ttl = 1;
            }
            else{            
              
              // création du paquet d'envoi:
              for (i = 0; i<9; i++) {
                txbuf[i] = rxbuf[i];
              }
              termPutchar('\r');
  

              //incrémentation du TTL de 1 à la réemission
              
              txbuf[3] = ttl+1;

              termPutchar('\r');
              
              sprintf(temp, "[paquet n°%u] TTL :: %u", sequence, ttl);
              printString(temp);

              termPutchar('\r');

              printString("Reemission du paquet:");
              termPutchar('\r');
              // affichage du paquet d'envoi
              for (i = 0; i<9; i++) {
              // Affichage de la trame récupérée
                printString(" | ");
                sprintf(temp, "%u", txbuf[i]);
                printString(temp);
              }

              rf95.send(txbuf, 9);    // emission  
              rf95.waitPacketSent();
            }

          }
          else{
            printString("Probleme avec l'adresse destination, ce n'est pas une adresse multicast");
            termPutchar('\r');
            termPutchar('\r');
          }
          termPutchar('\r');
          termPutchar('\r');
        }
        break;

      case E3: //Correspond au code du TP-NWK-Etudiant 4.5
        seq = rxbuf[6]; // Sequence du paquet actuel
        src[0] = rxbuf[4]; // Adresse source du paquet actuel
        src[1] = rxbuf[5]; // Adresse source du paquet actuel
        
        sprintf(temp, "Reception paquet n°%u", seq);
        printString(temp);
        termPutchar('\r');

        sprintf(temp, "Adresse source : %u.%u", src[0],src[1]);
        printString(temp);
        termPutchar('\r');

        
        isPacketResul = isPacketRouted(tableau, src[1], seq, &id);

        if(isPacketResul == 1){
          printString("Paquet deja route => destruction du paquet");
          termPutchar('\r');
        }
        else{
          if(rxbuf[1]==255 && rxbuf[2]==255){
            
            printString("L'adresse destination est bien une adresse multicast");
            termPutchar('\r');
            termPutchar('\r');

            printString("Paquet recu:  ");
              // affichage du paquet
              for (i = 0; i<9; i++) {

                // Affichage du paquet récupéré :
                printString(" | ");
                sprintf(temp, "%u", rxbuf[i]);
                printString(temp);
              }
            ttl = rxbuf[3];

            if(ttl >= max_TTL){
              termPutchar('\r');
              sprintf(temp, "[paquet n°%u] TTL :: %u", seq, ttl);
              printString(temp);
              termPutchar('\r');

              printString("TTL trop grand => destruction du paquet\r");
              termPutchar('\r');

              ttl = 1;
            }
            else{
              // Boucle pour créer le paquet de réemission
              for (i = 0; i<9; i++) {
              // création de la trame d'envoi:
                txbuf[i] = rxbuf[i];
              }
              termPutchar('\r');
              //incrémentation du TTL de 1 à la réemission
              
              txbuf[3] = ttl+1;

              termPutchar('\r');
              sprintf(temp, "[paquet n°%u] TTL :: %u", seq, ttl);
              printString(temp);

              termPutchar('\r');
              printString("Reemission du paquet:  ");

              // affichage du paquet d'envoi
              for (i = 0; i<9; i++) {
              // Affichage de la trame récupérée
                printString(" | ");
                sprintf(temp, "%u", txbuf[i]);
                printString(temp);
              }

              rf95.send(txbuf, 9);    // emission  
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

      case E4:
        sprintf(temp, "Reception du paquet : %u", Seq);
        printString(temp);
        termPutchar('\r');
        
        if(rxbuf[1]==self_ip[0] && rxbuf[2]==self_ip[1]){

          sprintf(temp, "L'adresse Correspond à l'ip de ce m5stack (%d.%d)", rxbuf[1],rxbuf[2]);
          termPutchar('\r');
          termPutchar('\r');

          ttl = 1;

          printString("Paquet recu :  ");
          termPutchar('\r');
          
          // affichage du paquet
          for (i = 0; i<9; i++) {

            // Affichage du paquet récupéré :
            printString(" | ");
            sprintf(temp, "%u", rxbuf[i]);
            printString(temp);
          }
          sprintf(temp, "Source : %d.%d", rxbuf[3],rxbuf[4]);
          printString(temp);
          sprintf(temp, "TTL : %d", rxbuf[5]);
          printString(temp);
        }
        else{
          printString("L'adresse ne Correspond pas a l'ip de ce m5stack");
          termPutchar('\r');
          termPutchar('\r');

          printString("Paquet recu :  ");
          termPutchar('\r');

            // affichage du paquet
            for (i = 0; i<9; i++) {

              // Affichage du paquet récupéré :
              printString(" | ");
              sprintf(temp, "%u", rxbuf[i]);
              printString(temp);
            }
          ttl = rxbuf[5];

          if(ttl >= max_TTL){
            termPutchar('\r');
            printString("TTL trop grand => destruction du paquet\r");
            termPutchar('\r');

            ttl = 1;
          }
          else{
            Seq = Seq +1;

            // Boucle pour créer le paquet de réemission
            for (i = 0; i<9; i++) {
            // création de la trame d'envoi:
              txbuf[i] = rxbuf[i];
            }
            termPutchar('\r');


            //incrémentation du TTL de 1 à la réemission
            
            txbuf[5] = ttl+1;

            termPutchar('\r');
            printString("TTL : ");  
            sprintf(temp, "%u", ttl);
            printString(temp);
            termPutchar('\r');
            termPutchar('\r');

            printString("Reemission du paquet :  ");
            termPutchar('\r');

            // affichage du paquet d'envoi
            for (i = 0; i<9; i++) {
            // Affichage de la trame récupérée
              printString(" | ");
              sprintf(temp, "%u", txbuf[i]);
              printString(temp);
            }
            rf95.send(txbuf, 9); // emission
            Seq = Seq +1;  
            rf95.waitPacketSent();
          }
        }
        termPutchar('\r');
        termPutchar('\r');
        break;

    }
    printString("[Bouton central pour retourner au menu principal]");
  }
}
