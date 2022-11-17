// Code de l'émetteur (émission simple de data) video 3 Scroll

#include <SPI.h>
#include <RH_RF95.h>
#include <M5Stack.h>

#define RFM95_CS 5 //M5Stack Lora
#define RFM95_DIO0 36
RH_RF95 rf95(RFM95_CS, RFM95_DIO0);

uint8_t rxbuf[RH_RF95_MAX_MESSAGE_LEN], rxbuflen = RH_RF95_MAX_MESSAGE_LEN;
uint8_t txbuf[RH_RF95_MAX_MESSAGE_LEN], txbuflen = RH_RF95_MAX_MESSAGE_LEN;
uint8_t rxlen = RH_RF95_MAX_MESSAGE_LEN, state, RxSeq, TxSeq, credit;
uint32_t attente;
uint32_t enBuf; //Utile pour savoir si 'buffer vide' a déjà été écrit
uint8_t FCS; // champ de contrôle d'un octet (entier non signé sur 8 bits)
int i; // index qui parcours la trame
char temp[255];

#define E0 0 // Emission de DATA (Données + XOR)
#define E1 1 // Armement du CDG
#define E2 2 // Activation du mode recepteur pour recevoir l'ACK
#define E3 3 // Attente de l'ack
#define E4 4 // Affichage de la trame correcte
#define E5 5 // Problème dans la reception du ACK => Gestion des crédits de répétitions

#define canal 1
#define TYPE_DATA 1 
#define TYPE_ACK 2
#define TIMEOUT_ACK 100

// Initialisations

void setup()
{
	M5.Power.begin();
  M5.begin(9600); //règle le débit du M5 à 9600 bauds ( = 9600 b/s)
  Serial.begin(115200);

  printString("Video DATA ACK Point a point\r\r");
  printString("On commence\r\r");

	if (!rf95.init()) 
	{
  	printString("\rErreur initialisation RF95\r\r");
  }
  else{ 
		printString("\rRF95 initialisation OK\r\r");
  }
	rf95.setModemConfig(RH_RF95::Bw125Cr45Sf128);
	rf95.setFrequency(867.7);

	state = E0;
  enBuf = 0;

	TxSeq = 0; credit = 5;

}

// Boucle sans fin : corps du programme

void loop() 
{
	switch (state)
		{
      case E0: // Emission de DATA + XOR
      delay(3000); //delai d'émission
			printString("Emission de la trame ---------------------------\r");
			printString("Numero de sequence : ");
      sprintf(temp, "%d", TxSeq);
      printString(temp);
      termPutchar('\r');
      
      //Remplissage des octets de la trame :
			txbuf[0] = TYPE_DATA; // Premier octet reservé au type de trame (DATA ou ACK)
      txbuf[1] = TxSeq; // Deuxieme octet pour le numéro de sequence

			for (i = 2;i<20;i++) // On insère 18 octets de données utiles dans la trame
			{
				txbuf[i] = 255;// 18 octets à 255 de payload (max)
			}

			// calcul du FCS : XOR

			FCS = 0;	// champ de contrôle d'un octet, initialisé à 0 (Cela ne posera pas de problème)

			for (i = 0;i<20;i++)
      {
        FCS = FCS ^ txbuf[i]; // XOR des 20 octets de la trame. Le ^ correspond au Ou exclusif en C
      }		
			txbuf[20] = FCS;

			for (i =0;i<21;i++) {
        // Affichage de la trame envoyée (De tous les octets) :
        printString("|");
        sprintf(temp, "%02x", txbuf[i]);
        printString(temp);
      }
      printString("|\r");

			rf95.send(txbuf, 21);		// emission
			rf95.waitPacketSent();
			credit--;		// on vient d'émettre une fois de plus la trame
			state = E1;
			break;

		 case E1: // Armement du CDG

		  attente = millis()+ TIMEOUT_ACK; 		//armement CdG (Début du compteur temporel)
		  state = E2;
		  break;


		 case E2: // Activation du recepteur
		  rf95.setModeRx(); 	// mettre la radio en mode réception pour l'ACK
		  state = E3;
		  break;

		 case E3: // Attente du ACK
		 	if (millis() > attente){
		 		state = E5; // Expiration du CDG pour l'attente de ack => Etat de gestion des crédits de répétions
      }
      else
		 	{
		 	if (rf95.recv(rxbuf, &rxbuflen)) // Si il y a quelque chose dans le buffeur, alors:
		 		{ // rxbuf[2] = rxbuf[2] + 1 ; 	// simulation erreur de XOR sur ACK

		 			if (( rxbuf[0]==TYPE_ACK)&&(rxbuf[1]==TxSeq)&&((rxbuf[0]^rxbuf[1])==rxbuf[2]))
		 				{
		 					// si la trame reçue est de type ACK et même numéro que la trame DATA émise et que le XOR est juste 
		 					state = E4;			// affichage de la trame reçue 
		 				}

		 			else state = E2;			// sinon on retourne à l'état E2
		 			
           if (( rxbuf[0]==TYPE_ACK)&&(rxbuf[1]==TxSeq)&&((rxbuf[0]^rxbuf[1])!=rxbuf[2])) 
		 			{ // si la trame reçue est de type ACK et même numéro que la trame Data emise mais trame erronée (XOR Faux)
		 			  state = E5;	//On affiche échec
		 			}

		 		}
        else{
          if(enBuf == 0){
            printString("Buffer vide... (Attente ACK)"); //On affiche une seule fois si le buffer est vide pour ne pas charger le terminal
            enBuf = 1;
          }
          
        }
		 	}
			break;

		 case E4:
				printString("\rACK recu ! Trame suivante\r\r"); // Affichage de la bonne reception du Ack
        enBuf = 0;
				state = E0;  // Emission de la trame suivante 
        TxSeq++; // Incrémentation du numéro de séquence de la trame
        credit = 5;	
				break;

		 case E5: 		// si le watchdog expire sans réception d'ACK, ECHEC si crédit épuisé 
		  printString("\r\rGestion des credits de repetitions (E5)");
		  if (credit ==0) // Si credit de repetitions epuisé, alors : 
		 	{
		 		printString("\rEchec, trame suivante\r\r"); // Affichage de echec
		 		state = E0; // Retour à l'etat d'emission
		 		credit = 5; TxSeq++;		// trame suivante
		 		break;
		 	}
		  else
		  	{
		  		printString("\rNouvelle tentative n° "); // Affichage du nombre de tentatives restantes
          sprintf(temp, "%d", 5 - credit);
          printString(temp);
		  		state = E0; // Retour à l'etat d'emission
          enBuf = 0;
		  		termPutchar('\r\r');
		  		break;
		  	}

		 default:
		 	state = E0;
		 	break; 
		}
}
