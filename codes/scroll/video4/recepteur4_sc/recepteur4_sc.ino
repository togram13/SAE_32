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

#define E0 0 // Ecoute => Reception de données
#define E1 1 //Attente 
#define E2 2 // Affichage de la trame
#define E3 3 // Emission du ACK

#define canal 1

#define TYPE_DATA 1
#define TYPE_ACK 2

#define MyAdr 0 //adresse locale du noeud Rx0

void setup (){
  char str[255] = "Video 4 \r ";

	// Initialisation console port
	M5.Power.begin(); //initialisation de la puissance
  M5.begin(9600); //règle le débit du M5 à 9600 bauds : Le même que celui de l'emetteur (Sinon, l'échange ne fonctionnerait pas)
  Serial.begin(115200);
  termInit();

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
    termPutchar('\r');
  }

      //configuration des parametres Radio :
  rf95.setModemConfig(RH_RF95::Bw125Cr45Sf128); //Selectionne des config de modems prédéfinis
  rf95.setFrequency(867.7); //règle la fréquence centrale Bande

	state = E0;
	delay(3000);

	strcpy(str,"Boucle principale\r");
  printString(str);
  termPutchar('\r');
	RxSeq = 255; // pas encore reçy 1ere trame n°0. 0-1 = 255 sur 8 bits non signés

}


void loop(){
	
  char temp[500];
  char octet[50];
  char str[255];
	int j; // index pour parcourir les octets de la trame reçue à afficher

	switch(state){

		case E0: // se mettre en réception des données

      strcpy(str,"Etat 0 : Ecoute\r");
      printString(str);
      termPutchar('\r');
			
      rf95.setModeRx();
			state = E1; // On va à l'etat d'attente de la trame
			break;

		case E1: //vérifier si un message reçu est disponible
			rxlen = RH_RF95_MAX_MESSAGE_LEN; //taille max en réception
			if (rf95.recv(rxbuf, &rxlen)){
				if ((rxbuf[2] == TYPE_DATA) && (rxbuf[1] == MyAdr)){
					//si message est de type DATA et moi moi
					state = E2;
				}
				else
				{
					state = E0; // On passe en mode reception de données
				}
			}
			break;

		case E2: // afficher la trame
			if (RxSeq != rxbuf[3]){
				//si 1ere fois que l'on reçoit cette trame
				RxSeq = rxbuf[3];

        strcpy(str,"Numero de sequence : \r");
        printString(str);
        termPutchar('\r');

        sprintf(temp, "%d", RxSeq); 
        printString(temp);
        termPutchar('\r');

        strcpy(str,"Nombre d'octets : \r");
        printString(str);
        termPutchar('\r');

        sprintf(temp, "%d", rxlen); 
        printString(temp);
        termPutchar('\r');
        termPutchar('\r');

        strcpy(str,"Data : \r");
        printString(str);
        termPutchar('\r');

				for (j=0; j<rxlen; j++){

          strcpy(str," | ");
          printString(str);

          sprintf(temp, "%u", rxbuf[j]); 
          printString(temp);

          strcpy(str," | \r");
          printString(str);
          termPutchar('\r');
				}
        termPutchar('\r');
			}
			else // si on a déjà reçu cette même trame
			{
        strcpy(str,"Duplication");
        printString(str);
        termPutchar('\r');
			}
			state = E3;
			break;

		case E3: // envoyer ACK

      strcpy(str,"Etat 3 : Envois du ACK");
      printString(str);
      termPutchar('\r');

			txbuf[0] = MyAdr; // je suis la source de l'ACK
			txbuf[1] = rxbuf[0]; //j'émets l'ACK vers la source de la DATA
			txbuf[2] = TYPE_ACK; // octer indiquant le type_message ACK
			txbuf[3] = rxbuf[3]; // ACK de même numéro que DATA
      
			rf95.send(txbuf, 4);
			rf95.waitPacketSent();

			state = E0;
			break;

		default:
			break;

	}
}
