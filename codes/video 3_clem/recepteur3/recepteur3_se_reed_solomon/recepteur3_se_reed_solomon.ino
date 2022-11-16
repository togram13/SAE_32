// Code de l'automate du récepteur
#include <SPI.h>
#include <RH_RF95.h>
#include <M5Stack.h>

#define RFM95_CS 5 //M5Stack Lora
#define RFM95_DIO0 36
RH_RF95 rf95(RFM95_CS, RFM95_DIO0);

uint8_t state;
uint8_t rxbuf[RH_RF95_MAX_MESSAGE_LEN];
uint8_t rxbuflen = RH_RF95_MAX_MESSAGE_LEN;
uint8_t rxlen = RH_RF95_MAX_MESSAGE_LEN;
int rxFrames;
int i, erreur, rang, j;			// index, erreur, rang
uint16_t Sr, SPr, Sc, SPc;		// sommes et sommes pondérées reçues et calculées

#define ECOUTE	1
#define TEST_RX 2
#define AFFFICHAGE 3

#define canal 0


void setup()
{
	M5.Power.begin();
  M5.begin(9600); //règle le débit du M5 à 9600 bauds ( = 9600 b/s)
  Serial.begin(115200);

  Serial.print("Video DATA ACK Point a point\n");
  Serial.print("\n");
  Serial.print("On commence\n");
  Serial.print("\n");

	if (!rf95.init()) 
	{
  	Serial.println("Erreur initialisation RF95");
  }
  else{ 
		Serial.println("RF95 initialisation OK");
  }
	rf95.setModemConfig(RH_RF95::Bw125Cr45Sf128);
	rf95.setFrequency(867.7);
  
	state = ECOUTE;
	delay(1000);										// délai pour activer terminal
	Serial.println("Boucle principale");				// Message de début
	rxFrames = 0;										// Aucune trame reçue au début
}


void loop()
{
 switch (state)
 {
 	case ECOUTE:
 		Serial.println("ECOUTE");
 		rf95.setModeRx();				// Mettre le transceiver en mode réception
 		state = TEST_RX;				// Etat suivant
 		break;


 	case TEST_RX:
 		Serial.println("TEST_RX");		// Debug
 		if (rf95.available())			// Test si trame disponible en mode réception
 		{
 			state = AFFFICHAGE;
 		}
 		break;

 	case AFFFICHAGE:
 		if (rf95.recv(rxbuf, &rxlen))	// Récupération de la trame reçue dans rxbuf
 		{
 			rxFrames ++;
 			Serial.printf("[%d] ", rxFrames); Serial.printf(" %d bytes: |", rxlen);
 			for (j=0; j<rxlen; j++)
 			{
 				Serial.printf("%02x|", rxbuf[j]);		// Affichage de la trame reçue 
 			}
 			Serial.println();

 			// rxlen =23;		// Simulation erreur de longueur 

 			// rxbuf[19] = 48	// Simulation erreur modifiant un octet

 			if (rxlen != 24)
 			{
 				Serial.printf("Erreur de longueur de trame : %d !", rxlen);
 				Serial.println();
 			}
 			
 			else
 			{
 				Sr = rxbuf[20] + rxbuf[21] * 256;
 				SPr = rxbuf[22] + rxbuf[23] * 256;

 				Sc = 0; SPc = 0;

 				for (i=0; i<20; i++)
 					{
 						Sc = Sc + rxbuf[i];
 						SPc = SPc + rxbuf[i] * (i+1);
 					}
 				Serial.printf("Sr=%d  SPr =%d     Sc=%d  SPc=%d",Sr, SPr, Sc, SPc);
 				Serial.println();
 			}

 			if (Sr != Sc)
 			{
 				Serial.println("Une correction est utile !");
 				erreur = Sc-Sr;
 				Serial.printf("Valeur erreur =%d ", erreur); 
 				Serial.println();

 				if (rang != 0)
 				{
 					Serial.printf("Correction de %02X en %O2X" ,rxbuf[rang-1], rxbuf[rang-1] - erreur);
					Serial.println();
 				}

 				else
 				{
 					Serial.println("Erreur sur la redondance, pas sur les DATA utiles");
 				}
 			}
 			else
 				if (SPr != SPc)
        {
 				  Serial.println("Erreur sur la redondance, pas sur les DATA utiles");
        }
 		}
 		state = ECOUTE;
 		break;
  
  
    default:
      break;
 }
}