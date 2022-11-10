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
   M5.begin(9600); //règle le débit du M5 à 9600 bauds ( = 9600 b/s)
   Serial.begin(115200);
   
   Serial.print("Video DATA ACK Point a point\n");
   Serial.print("\n");
   Serial.print("On commence\n");
   Serial.print("\n");
 
   if(!rf95.init()) //init règle la puissance à 13dBm
       Serial.print("Erreur initialisation RF95\n"); // on vérifie si il y a une erreur avec le transceiver
   else
       Serial.print("Initialisation OK\n"); // Si aucune erreur, on indique que tout est OK
       Serial.print("\n");
  
//configuration des parametres Radio :
rf95.setModemConfig(RH_RF95::Bw125Cr45Sf128); //Selectionne des config de modems prédéfinis

rf95.setFrequency(867.7); //règle la fréquence centrale
 
   state = E0; //en etat initial d'emission de données
 
   delay(1000); // On attend 1 s
 
   TxSeq = 0; //Initialisation des numeros de séquence des trames envoyées = 0 car DATA 0
   credit = 5; //on fixe le crédit de retransmission à 5. Au bout de 5 problèmes de retransmission, un echec est renvoyé
 
   Serial.print("Boucle principale"); // On indique qu'on entre dans la loop boucle
   Serial.print("\n");
  
}
 
void loop(){
  switch (state){
    case E0: // Cas d'un état d'émission
   
      Serial.print(" EMMISSION\n"); // Affichage de l'état dans lequel on est
      Serial.print("\n");
      Serial.print("numero de sequence de la trame : ");
      Serial.print(TxSeq); // Affichage du numéro de séquence courant de l'état
      Serial.print("\n");
      Serial.print("\n");
  
 
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
        Serial.print("ACK non recu dans le buffer\n");
        Serial.print("\n");
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
      Serial.print("ACK_Recu\n"); // On indique qu'on a bien reçu l'aqcuitemment
      Serial.print("\n");
      state = E0; // On retourne dans l'état d'emission
      TxSeq++; // On incrémente le numéro de séquence des trames émises
      credit = 5; //on remet le crédit au max (La trame suivante , elle aussi a le droit d'être retransmise 5 fois)
      break;
 
    case E5: // Echec, car gestion des crédit de (=0)
    
      Serial.print("etat 5, gestion des credits de repetitions\n");
      Serial.print("\n");
      if (credit == 0) // Cas où les 5 tentatives sont dépassées
      { 
          Serial.print("ECHEC, credit de repetition depasse\n"); // On indique le problème de non-répétition des trames
          Serial.print("\n");
          state = E0; // On repasse en mode émission
          credit = 5; // On réinitialise le crédit à 5 pour la prochaine trame
          TxSeq++; //Incrémentation du numéro de séquence de la trame suivante
          break;
       } 
       else 
       {
           Serial.print("Nouvelle tentative d'emission de la trame\n"); // Indication que le crédit de répétition n'est pas épuisé
           Serial.print("\n");
           Serial.print("nombre de credits restant : ");
           Serial.print(credit); // affichage du nombre de crédit qu'il reste
           Serial.print("\n");
           Serial.print("\n");
           state = E0; // Mode d'émission
           Serial.println();
          break;
       }
 
    default:
       state = E0;
       break;
  }
}
