// Code de l'émetteur (émission simple de data)

#include <SPI.h>
#include <RH_RF95.h>
#include <M5Stack.h>

uint8_t rxbuf[RH_RF95_MAX_MESSAGE_LEN], rxbuflen = RH_RF95_MAX_MESSAGE_LEN;
uint8_t txbuf[RH_RF95_MAX_MESSAGE_LEN], txbuflen = RH_RF95_MAX_MESSAGE_LEN;
uint8_t rxlen = RH_RF95_MAX_MESSAGE_LEN, state, RxSeq, TxSeq, credit;
uint32_t attente;
uint_t FCS;					// champ de contrôle d'un octet (entier non signé sur 8 bits)
int i;						// index

#define E0 0
#define E1 1
#define E2 2
#define E3 3
#define E4 4
#define E5 5

#define canal 1
#define TYPE_DATA 1 
#define TYPE_ACK 2
#define TIMEOUT_ACK 40

// Initialisations

void setup()
{
	M5.Power.begin(9600);

	if (!rf95.init()) 
		Serial.println("RF95 init failed");
	else			  
		Serial.println("RF95 init OK");

	rf95.setTxPower(RH_RF95_TXPOW_8DBM);
	rf95.setModemConfig(RH_RF95::GFSK_Rb125Fd125);
	rf95.setFrequency(433.1+canal*0.1, 0.05);

	state = E0;

	delay(1000);

	TxSeq = 0; credit = 5;

	Serial.println("Boucle principale");	
}


// Boucle sans fin : corps du programme

void loop() 
{
	switch (state)
		{case E0: 
			Serial.printf("EMISSION %d : ",TxSeq);
			txbuf[0] = TYPE_DATA; txbuf[1] = TxSeq;
			for (i = 2;i<20;i++)
			{
				txbuf[i] = 255;				// 18 octets à 255 de payload (max)
			}
			// calcul du FCS : XOR
			FCS = 0;						// champ de contrôle d'un octet 
			for (i = 0;i<20;i++) FCS = FCS ^txbuf[i];		// XOR des 20 octets de la trame
			txbuf[20] = FCS;
			// affichage pour debug 
			for (i =0;i<21;i++) Serial.printf("|%02X",txbuf[i]);
			Serial.println("|");
			rf95.send(txbuf, 21);		// emission
			rf95.waitPacketSent();
			credit--;		// on vient d'émettre une fois de plus la trame
			state = E1;
			break;

		 case E1:
		  attente = millis()+ TIMEOUT_ACK; 		//armement CdG
		  state = E2;
		  break;


		 case E2:
		  rf95.setModeRx(); 	// mettre la radio en mode réception pour l'ACK
		  state = E3;
		  break;


		 case E3:
		 	if (millis() > attente)
		 		state = E5;
		 	else
		 	{
		 	if (rf95.recv(rxbuf, &rxbuflen))
		 		{ // rxbuf[2] = rxbuf[2] + 1 ; 	// simulation erreur de XOR sur ACK
		 			if (( rxbuf[0]==TYPE_ACK)&&(rxbuf[1]==TxSeq)&&((rxbuf[0]^rxbuf[1])==rxbuf[2]))
		 				{
		 					// si la trame reçue est ACK et même numéro que DATA émise et trame juste 
		 					state = E4;			// si oui on  affiche la trame reçue 
		 				}
		 			else state = E2;			// sinon on retourne à l'état E2
		 			if (( rxbuf[0]==TYPE_ACK)&&(rxbuf[1]==TxSeq)&&((rxbuf[0]^rxbuf[1])!=rxbuf[2])) 
		 			{ // si la trame reçue est de type ACK et même numéro mais trame erronée
		 			  state = E5;	// si oui affiche échec
		 			}
		 		}
		 	}
			break;

		 case E4:
				Serial.println("ACK_RECU");
				state = E0; TxSeq++; credit = 5;	// trame suivante
				break;

		 case E5: 		// si le watchdog expire sans réception d'ACK, ECHEC si crédit épuisé 
		  Serial.println("E5");
		  if (credit ==0)
		 	{
		 		Serial.println("ECHEC");
		 		state = E0;
		 		credit = 5; TxSeq++;		// trame suivante
		 		break;
		 	}
		  else
		  	{
		  		Serial.printf("Nouvelle tentative n° %d",5-credit);
		  		state = E0;
		  		Serial.println();
		  		break;
		  	}
		 default:
		 	state = E0;
		 	break; 
		}
}