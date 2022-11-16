#include <SPI.h>
#include <RH_RF95.h>
#include <M5Stack.h>

#define RFM95_CS 5 //M5Stack Lora
#define RFM95_DIO0 36
RH_RF95 rf95(RFM95_CS, RFM95_DIO0);

uint8_t state, i;
uint8_t rxbuf[RH_RF95_MAX_MESSAGE_LEN], rxbuflen = RH_RF95_MAX_MESSAGE_LEN;
uint8_t txbuf[RH_RF95_MAX_MESSAGE_LEN], txbuflen = RH_RF95_MAX_MESSAGE_LEN;
uint8_t rxlen = RH_RF95_MAX_MESSAGE_LEN;
uint8_t RxSeq;
uint32_t attente; // pour plus tard si full duplex

#define E0 0
#define E1 1
#define E2 2
#define E3 3

#define canal 1

#define TYPE_DATA 1
#define TYPE_ACK 2

#define MyAdr 0 //adresse locale du noeud Rx0

void setup (){

// Initialize console port 
	M5.Power.begin();
  M5.begin(9600); //règle le débit du M5 à 9600 bauds ( = 9600 b/s)
  Serial.begin(115200);

  Serial.print("Video 4\n");
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
	rf95.setFrequency(867.7);

	state = E0;
	delay(3000);
	Serial.println("Boucle principale");
	RxSeq = 255; // pas encore reçu 1ere trame n°0. 0-1 = 255 sur 8 bits non signés

}


void loop(){
	
	int j; // index pour parcourir les octets de la trame reçue à afficher

	switch(state){

		case E0: // se mettre en réception des données
			Serial.println("Etat 0 : reception de donnees");
			rf95.setModeRx();
			state = E1;
			break;

		case E1: //vérifier si un message reçu est disponible
			rxlen = RH_RF95_MAX_MESSAGE_LEN; //taille max en réception
			if (rf95.recv(rxbuf, &rxlen)){ // Si il y a quelque chose dans le buffer, alors :
				if ((rxbuf[2] == TYPE_DATA) && (rxbuf[1] == MyAdr)){
					//si message est de type DATA et moi moi
					state = E2;
				}
				else
				{
					state = E0;
				}
			}
			break;

		case E2: // afficher la trame
			if (RxSeq != rxbuf[3]) // si 1ere fois que l'on reçoit cette trame
      {
				
				RxSeq = rxbuf[3];
				Serial.printf("Numero de sequence : %d ", RxSeq);
        Serial.printf("\n");
				Serial.printf("Nombre d'octets : %d", rxlen);
				Serial.println();

				for (j=0; j<rxlen; j++) // Affichage de la trame
        {
					Serial.printf("%02x|", rxbuf[j]);
				}
				Serial.println();
			}
			else // si on a déjà reçu cette même trame
			{
				Serial.printf("Duplication");
			}
			state = E3;
			break;

		case E3: // envoyer ACK

			Serial.println("E3");
			txbuf[0] = MyAdr; // je suis la source de l'ACK
			txbuf[1] = rxbuf[0]; //j'émets l'ACK vers la source de la DATA
			txbuf[2] = TYPE_ACK; // octet indiquant le type_message (ACK)
			txbuf[3] = rxbuf[3]; // ACK de même numéro de sequence que la trame DATA reçue
			
      rf95.send(txbuf, 4);
			rf95.waitPacketSent();

			state = E0;
			break;

		default:
			break;

	}
}
