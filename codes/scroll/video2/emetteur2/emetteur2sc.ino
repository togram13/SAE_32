#include <M5Stack.h>

#include <SPI.h> //Nativement dans arduino donc pas besoin de l'installer en +
#include <RH_RF95.h> //gestion des transmissions radio en Lora
#include <M5Stack.h>
 
#define RFM95_CS 5 //M5Stack Lora
#define RFM95_DIO0 36
RH_RF95 rf95(RFM95_CS, RFM95_DIO0);

int  RF95_MAX_MESSAGE_LEN = 16;
uint8_t rxbuf[16]; //variable (de type tableau) de réception de data dans le buffer
uint8_t rxbuflen = RF95_MAX_MESSAGE_LEN; 
uint8_t txbuf[16]; //variable (de type tableau) d'emission de data
uint8_t txbuflen = RF95_MAX_MESSAGE_LEN; 
uint8_t rxlen = RF95_MAX_MESSAGE_LEN; //variable pour la taille Max du buffer en reception
uint8_t state,RxSeq,TxSeq, credit; //numéros de séquences
uint32_t attente; // delai d'attente du chien de garde en ms
 
#define E0 0 //Emission de data
#define E1 1 // Armer le CDG
#define E2 2 //Activer le récepteur pour recevoir le ACK
#define E3 3 // Attente de la trame d'acquittement 
#define E4 4 //affichage de la trame d'acquittement
#define E5 5 // Remontée d'un échec
 
#define canal 1 // canal commun emetteur - récepteur
#define TYPE_DATA 1 // Indique au sein de la trame que c'est une trame DATA
#define TYPE_ACK 2 // Indique au sein de la trame que c'est une trame ACK
#define TIMEOUT_ACK 40 // Valeur de chien de garde (40 ms)
 
void setup(){
   M5.Power.begin(); //initialisation de la puissance
   termInit();
   M5.begin(9600); //règle le débit du M5 à 9600 bauds ( = 9600 b/s)
   Serial.begin(115200);
   
   char str[255] = "Video DATA Point a point\r ";
   printString(str);
   termPutchar('\r');

   strcpy(str,"On commence\r");
   printString(str);
   termPutchar('\r');
 
   if(!rf95.init()){ //init règle la puissance à 13dBm
    strcpy(str,"Erreur initialisation RF95\r");  // on vérifie si il y a une erreur avec le transceiver
    printString(str);
    termPutchar('\r');
  }                           // Problèmes possibles : cablages ou drivers logiciels
  else{
    strcpy(str,"Initialisation OK\r");
    printString(str);
    termPutchar('\r');
  }
  
  //configuration des parametres Radio :
  rf95.setModemConfig(RH_RF95::Bw125Cr45Sf128); //Selectionne des config de modems prédéfinis

  rf95.setFrequency(867.7); //règle la fréquence centrale
 
   state = E0; //en etat initial d'emission de données
 
   delay(1000); // On attend 1 s
 
   TxSeq = 0; //Initialisation des numeros de séquence des trames envoyées = 0 car DATA 0
   credit = 5; //on fixe le crédit de retransmission à 5. Au bout de 5 problèmes de retransmission, un echec est renvoyé
 
   strcpy(str,"boucle principale\r"); // On indique qu'on entre dans la loop boucle
   printString(str);
   termPutchar('\r');
  
}
 
void loop(){
  char temp[500];
  char octet[50];
  char str[255] = "Video DATA Point a point\r ";

  switch (state){
    case E0: // Cas d'un état d'émission
    
      delay(3000);
      strcpy(str,"EMMISSION\r"); // Affichage de l'état dans lequel on est
      printString(str);
      termPutchar('\r');

      strcpy(str,"numero de sequence de la trame :  \r");
      printString(str);

      sprintf(temp, "%u", TxSeq); // Affichage du numéro de séquence courant de l'état
      printString(temp);
      termPutchar('\r');
      termPutchar('\r');
  
      txbuf[0] = TYPE_DATA; // type de message
      txbuf[1] = TxSeq; // numéro de trame DATA émise
      txbuf[2] = 0x0AA; // Premier octet de DATA utiles 
      txbuf[3] = 0x55; // deuxieme octet de DATA utiles 
      rf95.send(txbuf, 4); // envoie de données (une trame de 4 octets)
      rf95.waitPacketSent(); // delai d'attente pour que le RF95 ai bien fini d'emettre
 
      credit--; // on vient d'émettre une fois de + la trame (Décrémentation du crédit de réemission)
      
      state = E1; // etat durant lequel le CDG est armé
      break;
  
    case E1: 
      attente = millis()+ TIMEOUT_ACK; //armement du chien de garde (Mémorisation de l'instant présent en ms + durée du CDG)
      // Quand la valeur attente est dépassée : expiration du chien de garde (Non bloquant)
      
      state = E2;
      break;
  
    case E2: // etat de réception
    
      rf95.setModeRx(); // On passe en mode réception pour recevoire la trame d'acquittement
      state = E3;
      break;
  
    case E3: // attente d'ACK non bloquante (car test trame reçue durant le CdG)
      
      if (millis() > attente)//Vérif si watchdog expiré (Si l'heure courant est supérieure à la valeur max d'attente, alors l'ACK n'est pas arrivé assez rapidement => CDG expiré)
      { 
        strcpy(str,"ACK non recu dans le buffer\r");
        printString(str);
        termPutchar('\r');

        state = E5; //état d'érreur => attente expirée
       } 
       else //si attente en cours (CdG non expiré)
       { 
        if (rf95.recv(rxbuf, &rxlen)) //Verif si la trame est arrivée dans le buffer
        { 
          if ((rxbuf[0] == TYPE_ACK) && (rxbuf[1]) == TxSeq) // on test si la trame reçue est de type ACK est de même numéro que DATA émise
          { 
            state = E4; //si oui, on passe en mode affichage car le ACK est OK 
           } 
           else 
           {
            state = E2; // Sinon on retourne à l'état E2 (état de réception du ACK)
           }
         }
       }
       break;
 
    case E4: //affichage de la trame reçue
      strcpy(str,"ACK_Recu\r"); // On indique qu'on a bien reçu l'aqcuitemment
      printString(str);
      termPutchar('\r');

      state = E0; // On retourne dans l'état d'emission
      TxSeq++; // On incrémente le numéro de séquence des trames émises
      credit = 5; //on remet le crédit au max (La trame suivante , elle aussi a le droit d'être retransmise 5 fois)
      break;
 
    case E5: // Echec, car gestion des crédit de (=0)
      strcpy(str,"etat 5, gestion des credits de repetitions\r");
      printString(str);
      termPutchar('\r');
      if (credit == 0) // Cas où les 5 tentatives sont dépassées
      { 
          strcpy(str,"ECHEC, credit de repetition depasse\r"); // On indique le problème de non-répétition des trames
          printString(str);
          termPutchar('\r');

          state = E0; // On repasse en mode émission
          credit = 5; // On réinitialise le crédit à 5 pour la prochaine trame
          TxSeq++; //Incrémentation du numéro de séquence de la trame suivante
          break;
       } 
       else 
       {
           strcpy(str,"Nouvelle tentative d'emission de la trame\r"); // Indication que le crédit de répétition n'est pas épuisé
           printString(str);
           termPutchar('\r');

           strcpy(str,"nombre de credits restant : \r");
           printString(str);
           termPutchar('\r');

           sprintf(temp, "%u", credit); // affichage du nombre de crédit qu'il reste
           printString(temp);
           termPutchar('\r');
           termPutchar('\r');

           state = E0; // Mode d'émission
           termPutchar('\r');
          break;
       }
 
    default:
       state = E0;
       break;
  }
}
