// Code du récepteur serial Video 3

#include <SPI.h>
#include <RH_RF95.h>
#include <M5Stack.h>

#define RFM95_CS 5 //M5Stack Lora
#define RFM95_DIO0 36
RH_RF95 rf95(RFM95_CS, RFM95_DIO0);

uint8_t state;
uint8_t rxbuf[RH_RF95_MAX_MESSAGE_LEN];
uint8_t rxbuflen = RH_RF95_MAX_MESSAGE_LEN;
uint8_t txbuf[RH_RF95_MAX_MESSAGE_LEN];
uint8_t txbuflen = RH_RF95_MAX_MESSAGE_LEN;
uint8_t rxlen = RH_RF95_MAX_MESSAGE_LEN;
uint8_t RxSeq;
uint32_t attente; 
uint8_t FCSc, FCSr;		// champ de contrôle d'un octet calculé et reçu
int i;
int j;

#define E0 0 // Ecoute
#define E1 1 // Attente de reception de la trame
#define E2 2 // Affichage de la trame
#define E3 3 // Emission de ACK + XOR

#define canal 1

#define TYPE_DATA 1
#define TYPE_ACK 2


void setup()
{
	// Initialize console port 
	M5.Power.begin();
  M5.begin(9600); //règle le débit du M5 à 9600 bauds ( = 9600 b/s)
  Serial.begin(115200);

  Serial.print("Video DATA ACK Point a point\n");
  Serial.print("\n");
  Serial.print("On commence\n");
  Serial.print("\n");

	if (!rf95.init()){
		Serial.println("Erreur initialisation RF95");
  }
  else
  {
		Serial.println("RF95 initialisation OK");
    Serial.printf("\n");
  }

	rf95.setModemConfig(RH_RF95::Bw125Cr45Sf128);
	rf95.setFrequency(900);


	state = E0; // Etat ecoute
	Serial.println("Boucle principale");
  Serial.println();
	RxSeq = 255;		// pas encore reçu 1ere trame n°0. 0-1 = 255 sur 8 bits non signés
}


void loop()
{
 switch (state)
 {
 	case E0:	// Mode  réception des données
 		//Serial.println("Etat 0 : reception de data");
 		rf95.setModeRx();
 		state = E1;
 		break;

 	case E1:	// vérifier si un message reçu est disponible 
 		rxlen = RH_RF95_MAX_MESSAGE_LEN;	// taille max en réception
 		if (rf95.recv(rxbuf, &rxlen)) // Si il y a quelque chose dans le buffer, alors :
 		{
      Serial.println("Paquet reçu ---------------------------");
 			if (rxbuf[0]==TYPE_DATA)	// si le premier octet du message est de type DATA
 			{
 				//test si FCS est juste 
 				FCSr = rxbuf[20];	// champ de contrôle reçu
 				FCSc = 0;	// calcul du FCS de la part du recepteur, que l'on comparera avec celui reçu dans un champs de la trame FCSr

 				for (i=0;i<20;i++){ 
          FCSc = FCSc ^ rxbuf[i];	// code = XOR des 20 octets de payload
         }
          if (FCSc == FCSr){	// même FCS calculé et reçu : trame juste
            state = E2; // etat d'affichage de la trame
          }
 					else {
            Serial.println("FCS erroné");
 						state = E0;		// trame fausse => FCS calculé et reçu différents => etat de reception de données
          }
       }
 		}
 		break;

 	case E2:	// afficher la trame
 		if (RxSeq != rxbuf[1])	// si 1ere fois que l'on reçoit cette trame
 		{
 			RxSeq = rxbuf[1];
 			Serial.printf("Numero de sequence : %d ", RxSeq);
      Serial.println();
 			Serial.printf("Nombre d'octets : %d", rxlen);
 			Serial.println();

 			for (j=0; j<rxlen; j++)
 			{
 				Serial.printf("%02x|", rxbuf[j]);
 			}
 		}
 		else	// si on a déjà reçu cette même trame
 		{
 			Serial.printf("Duplication de la trame => absorption");
 		}
 		state = E3; // Envois du ACK
 		break;

 	case E3:	// envoyer ACK
 			Serial.printf("\n");
      Serial.println("Emission de ACK (E3)");
      Serial.printf("\n");

 			txbuf[0] = TYPE_ACK;	// octet indiquant le type_message ACK
 			txbuf[1] = rxbuf[1];	// ACK de même numéro que le numero de sequence de la trame data reçue
 			txbuf[2] = txbuf[0] ^ txbuf[1];	// Envois du XOR de la trame (Calculé uniquement sur les 2 premiers champs)

 			rf95.send(txbuf, 3);
 			rf95.waitPacketSent();

 			state = E0; // Retour à l'état d'écoute (de reception de trame) 
 		break;

 	default:
 		break;
 }
}


