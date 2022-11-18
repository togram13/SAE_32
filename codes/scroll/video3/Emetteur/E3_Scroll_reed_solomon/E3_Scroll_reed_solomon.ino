// Code de l'émetteur (Reed-Solomon)

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
char temp[255];


void setup()
{
  
  M5.Power.begin();
  M5.begin(9600); //règle le débit du M5 à 9600 bauds ( = 9600 b/s)
  Serial.begin(115200);

  printString("Video DATA ACK Point a point\r\r");
  printString("On commence\r\r");

  if (!rf95.init()) 
  {
    printString("Erreur initialisation RF95\r");
  }
  else{ 
    printString("RF95 initialisation OK\r");
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
      printString("Emission de la trame ---------------\r"); // trame de 20 octets utiles + 4 octets code RS
      S = 0;
      SP=0;
      
      for(i=0; i<20; i++)
      {
        txbuf[i] = 255;
        S = S + txbuf[i];
        SP = SP + txbuf[i]*(i+1);
        sprintf(temp, "|%02x", txbuf[i]);
      }
      printString("|");

      txbuf[20] = S & 0x00FF;
      txbuf[21] = (S & 0xFF00) >> 8;

      sprintf(temp, "%02x|", txbuf[20]);
      printString(temp);
      sprintf(temp, "%02x|", txbuf[21]);
      printString(temp);

      txbuf[22] = SP & 0x00FF;
      txbuf[23] = (SP & 0xFF00) >> 8;
      sprintf(temp, "%02x|", txbuf[22]);
      printString(temp); 
      sprintf(temp, "%02x|", txbuf[23]);
      printString(temp);

      rf95.send(txbuf, 24);   // émission
      rf95.waitPacketSent();
      state = DELAI;
      break;


    case DELAI:       // code source de l'état DELAI
      printString("\rAttente...\r");
      delay(3000);      // 1s avant d'émettre la trame suivante
      state = EMISSION;   // transition vers l'état suivant
      termPutchar('\r\r');
      break;
     
    default:
      break;
  }
}
