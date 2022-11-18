// Code de l'automate de l'émetteur

#include <SPI.h>
#include <RH_RF95.h>
#include <M5Stack.h>

#define RFM95_CS 5 //M5Stack Lora
#define RFM95_DIO0 36
RH_RF95 rf95(RFM95_CS, RFM95_DIO0);

uint8_t state;
uint8_t txbuf[RH_RF95_MAX_MESSAGE_LEN];
uint8_t txbuflen = RH_RF95_MAX_MESSAGE_LEN;

#define EMISSION 0
#define DELAI 1

#define canal 0

int i;
uint16_t S, SP;


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

	state = EMISSION;
	delay(1000);
	//randomSeed(analogRead(A3));

}


void loop()
{
	switch (state)
	{
		case EMISSION:
			Serial.println("Emission de la trame ---------------");	// trame de 20 octets utiles + 4 octets code RS
			S = 0;
      SP=0;
      
			for(i=0; i<20; i++)
			{
				txbuf[i] = 255;
        S = S + txbuf[i];
        SP = SP + txbuf[i]*(i+1);
				Serial.printf("|%02x", txbuf[i]);
			}
      Serial.printf("|");

			txbuf[20] = S & 0x00FF;
      txbuf[21] = (S & 0xFF00) >> 8;

			Serial.printf("%02x|", txbuf[20]);
      Serial.printf("%02x|", txbuf[21]);

			txbuf[22] = SP & 0x00FF;
      txbuf[23] = (SP & 0xFF00) >> 8;
			Serial.printf("%02x|", txbuf[22]); 
      Serial.printf("%02x|", txbuf[23]);

			rf95.send(txbuf, 24);		// émission
      rf95.waitPacketSent();
			state = DELAI;
			break;


		case DELAI:				// code source de l'état DELAI
		 	Serial.println("Attente...\n");
		 	delay(3000);			// 1s avant d'émettre la trame suivante
		 	state = EMISSION;		// transition vers l'état suivant
		 	break;

		 
		default:
		 	break;
	}
}