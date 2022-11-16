#include <SPI.h>
#include <RH_RF95.h>
#include <M5Stack.h>

#define RFM95_CS 5 //M5Stack Lora
#define RFM95_DIO0 36
RH_RF95 rf95(RFM95_CS, RFM95_DIO0);

uint8_t rxbuf[RH_RF95_MAX_MESSAGE_LEN], rxbuflen = RH_RF95_MAX_MESSAGE_LEN;
uint8_t txbuf[RH_RF95_MAX_MESSAGE_LEN], txbuflen = RH_RF95_MAX_MESSAGE_LEN;
uint8_t rxlen = RH_RF95_MAX_MESSAGE_LEN;
uint8_t state, RxSeq, TxSeq, credit, backoff, NewFrame, EIT;
uint32_t attente;

#define E0 0 // Emission de Flux Data
#define E1 1 // Armer CDG
#define E2 2 // Activation du reception
#define E3 3 //Attente
#define E4 4 //Affichage de la trame
#define E5 5 // Echec 


#define canal 1
#define TYPE_DATA 1
#define TYPE_ACK 2
#define TIMEOUT_ACK 40
#define MyAdr 1 //adresse locale du noeud Tx1 ou Tx2 ou... Tx9

void setup (){

	// Initialisation console port
	M5.begin();
  termInit();
  M5.Power.begin(); //initialisation de la puissance

  char str[255] = "Video 4 \r ";
  printString(str);
  termPutchar('\r');

  strcpy(str,"On commence\r");
  printString(str);
  termPutchar('\r');

  //initialisation du transceiver radio:

  if(!rf95.init()){ //init règle la puissance à 13dBm
    strcpy(str,"Erreur initialisation RF95\r");  // on vérifie si il y a une erreur avec le transceiver
    printString(str);
    termPutchar('\r');
  }                           // Problèmes possibles : cablages ou drivers logiciels
  else{
    strcpy(str,"Initialisation OK\r");
    printString(str);
    termPutchar('\r');
    termPutchar('\r');
  }

      //configuration des parametres Radio :
  rf95.setModemConfig(RH_RF95::Bw125Cr45Sf128); //Selectionne des config de modems prédéfinis
  rf95.setFrequency(867.7); //règle la fréquence centrale Bande

	state = E0;
	delay(3000);
	TxSeq = 0;
	credit = 5;

  strcpy(str,"Boucle principale\r");
  printString(str);
  termPutchar('\r');
	NewFrame = 1;

}


void loop(){
	char temp[500];
  char octet[50];
  char str[255] = "Video 4 \r ";

	switch(state){

		case E0: // Emission de Flux Data
			if (NewFrame == 1){ // loi d'arrivée aléatoire du flux de trames à envoyer
				EIT = random(5,100);
				delay(EIT); //la nouvelle trame à envoyer arrive entre 5 et 99 ms plus tard
				
        strcpy(str,"Initialisation OK\r");
        printString(str);
        termPutchar('\r');

        strcpy(str," EIT : ");
        printString(str);

        sprintf(temp, "%d", EIT); // Affichage du numéro de séquence courant de l'état
        printString(temp);
        termPutchar('\r');
			}
      delay(3000);

      strcpy(str," EMISSION\r");
      printString(str);
      termPutchar('\r');

      strcpy(str," Numero de sequence : ");
      printString(str);

      sprintf(temp, "%d", TxSeq); // Affichage du numéro de séquence courant de l'état
      printString(temp);
      termPutchar('\r');

			txbuf[0] = MyAdr; // @S : Moi
			txbuf[1] = 0; // @D : le puits
			txbuf[2] = TYPE_DATA; //le type de message
			txbuf[3] = TxSeq;
			txbuf[4] = 'Ox0AA';
			txbuf[5] = 'Ox55';
			rf95.send(txbuf, 6); //envoie de données (une trame de 6 octets)
			rf95.waitPacketSent();
			credit--; // On vient d'émettre une fois de + la trame
			state = E1;
			break;

		case E1: // Armer CDG
			attente = millis() + TIMEOUT_ACK; //armement watchdog
			state = E2;
			break;

		case E2: // Activation du mode reception
			rf95.setModeRx(); // Mettre la radio en mode réception pour l'ACK
			state = E3;
			break;

		case E3: //attente d'ACK non bloquante ( car test trame reçue durant le watchdog )
			if(millis() > attente){ // vérification si le watchdog est expiré
				state = E5; // si attente expirée : problème !
			}
			else // si attente en cours (watchdog non expiré)
			{
				if(rf95.recv(rxbuf, &rxlen)){
					//vérifier si une trame est disponible
					if((rxbuf[2] == TYPE_ACK) && (rxbuf[3] == TxSeq) && (rxbuf[1] == MyAdr)){ 
						//si la trame reçue est de type ACK, et même numero, et pour moi
						state = E4;
					}
					else
					{
						state = E2;
					}
				}
			}
			break;

		case E4: //Affichage de la trame

      strcpy(str,"ACK_RECU \r");
      printString(str);
      termPutchar('\r');

			state = E0;
			TxSeq++;
			credit = 5; // trame suivante
			break;

		case E5: //si le watchdog expire sans réception d'ACK, ECHEC si crédit épuisé
			
      strcpy(str,"Etat 5 : \r");
      printString(str);
      termPutchar('\r');

			if (credit == 0){

        strcpy(str,"ECHEC, credit epuise\r");
        printString(str);
        termPutchar('\r');

				state = E0;
				NewFrame = 1; // nouvelle trame ( prévoir EIT : flux d'arrivée)
				credit = 5;
				TxSeq++;
				break;
			}
			else
			{

        strcpy(str,"Collision\r");
        printString(str);
        termPutchar('\r');

        strcpy(str,"Nouvelle tentative n° \r");
        printString(str);
        termPutchar('\r');
        
        sprintf(temp, "%d", 5-credit); // Affichage du numéro de séquence courant de l'état
        printString(temp);
        termPutchar('\r');

        strcpy(str,"Nombre de credit restant \r");
        printString(str);
        termPutchar('\r');
        
        sprintf(temp, "%d", 5-credit); // Affichage du numéro de séquence courant de l'état
        printString(temp);
        termPutchar('\r');

				state = E0;
				NewFrame = 0; // toujours même trame : pas de EIT de flux
				backoff = random(0,100);
				delay(backoff); //attente aléatoire ALOHA

        strcpy(str," Backoff : \r");
        printString(str);
        termPutchar('\r');
        
        sprintf(temp, "%d", backoff); // Affichage du numéro de séquence courant de l'état
        printString(temp);
        termPutchar('\r');
        termPutchar('\r');

				break;
			}

		default:
			state = E0;
			break;
	}

}
