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
#define MyAdr 1 //adresse locale du noeud Tx1 ou Tx2 ou... Tx9

void setup (){

	// Initialisation console port
	M5.Power.begin(); //initialisation de la puissance
  M5.begin(9600); //règle le débit du M5 à 9600 bauds : Le même que celui de l'emetteur (Sinon, l'échange ne fonctionnerait pas)
  Serial.begin(115200);
  M5.Lcd.setTextColor(RED, TFT_BLACK);
  Serial.print("Video 4\n");
  Serial.print("\n");
  Serial.print("On commence\n");
  Serial.print("\n");
  
  if(!rf95.init()) //init règle la puissance à 13dBm
      Serial.print("Erreur initialisation RF95\n"); // on vérifie si il y a une erreur avec le transceiver
  else
      Serial.print("Initialisation OK\n"); // On indique que l'initialisation s'est bien passée
      Serial.print("\n");

  rf95.setModemConfig(RH_RF95::Bw125Cr45Sf128);
  rf95.setFrequency(867.7); // On défini la fréquence à laquelle on emet

	state = E0;
	delay(3000);
	TxSeq = 0;
	credit = 5;
	Serial.println("Boucle principale");

	NewFrame = 1;

}


void loop(){
	
	switch(state){

		case E0:
			if (NewFrame == 1){ // loi d'arrivée aléatoire du flux de trames à envoyer
				EIT = random(5,100);
				delay(EIT); //la nouvelle trame à envoyer arrive entre 5 et 99 ms plus tard
				Serial.printf(" EIT : %d ", EIT);
				Serial.println();
			}
			Serial.printf("EMISSION %d : ", TxSeq);
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

		case E1:
			attente = millis() + TIMEOUT_ACK; //armement watchdog
			state = E2;
			break;

		case E2:
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

		case E4:
			Serial.println("ACK_RECU");
			state = E0;
			TxSeq++;
			credit = 5; // trame suivante
			break;

		case E5: //si le watchdog expire sans réception d'ACK, ECHEC si crédit épuisé
			Serial.println("Etat 5");
			if (credit == 0){

				Serial.println("ECHEC, credit epuise");
				state = E0;
				NewFrame = 1; // nouvelle trame ( prévoir EIT : flux d'arrivée)
				credit = 5;
				TxSeq++;
				break;
			}
			else
			{
				Serial.printf("Collision Nouvelle tentative n° %d\n", 5-credit);
        Serial.printf("Nombre de credit restant %d", 5-credit);
        Serial.printf("\n");
				state = E0;
				NewFrame = 0; // toujours même trame : pas de EIT de flux
				backoff = random(0,100);
				delay(backoff); //attente aléatoire ALOHA
				Serial.printf(" Backoff : %d", backoff);
        Serial.printf("\n");
				Serial.println();
				break;
			}

		default:
			state = E0;
			break;
	}

}
