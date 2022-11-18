// Code du récepteur Scroll Reed Solomon Video 3
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
int i, erreur, rang, j;      // index, erreur, rang
uint16_t Sr, SPr, Sc, SPc;    // sommes et sommes pondérées reçues et calculées
char temp[255];

#define ECOUTE  1
#define TEST_RX 2
#define AFFFICHAGE 3

#define canal 0


void setup()
{
  M5.Power.begin();
  M5.begin(9600); //règle le débit du M5 à 9600 bauds ( = 9600 b/s)
  Serial.begin(115200);

  printString("Video DATA ACK Point a point\r");
  termPutchar('\r');
  printString("On commence\r");
  printString('\r');

  if (!rf95.init()) 
  {
    printString("Erreur initialisation RF95\r");
  }
  else{ 
    printString("RF95 initialisation OK\r");
  }
  rf95.setModemConfig(RH_RF95::Bw125Cr45Sf128);
  rf95.setFrequency(867.7);
  
  state = ECOUTE;
  rang = 0;
  delay(1000);                    // délai pour activer terminal
  rxFrames = 0;                   // Aucune trame reçue au début
}


void loop()
{
 switch (state)
 {
  case ECOUTE:
    printString("\r----------------------------\rEcoute... ");
    rf95.setModeRx();       // Mettre le transceiver en mode réception
    state = TEST_RX;        // Etat suivant
    break;


  case TEST_RX:
    //printString("TEST_RX");    // Debug
    if (rf95.available())     // Test si trame disponible en mode réception
    {
      state = AFFFICHAGE;
    }
    break;

  case AFFFICHAGE:
    if (rf95.recv(rxbuf, &rxlen)) // Récupération de la trame reçue dans rxbuf
    {
      rxFrames ++;
      sprintf(temp, "Numero de sequence : [%d]\r", rxFrames);
      sprintf(temp, "%d octets : |", rxlen);

      for (j=0; j<rxlen; j++)
      {
        sprintf(temp, "%02x|", rxbuf[j]);   // Affichage de la trame reçue 
      }
      printString();


      //rxlen = 23;   // Simulation erreur de longueur 

      //printString("Modification d'un des octets dans les DATA... (Simulation erreur)");  // Simulation erreur modifiant un octet
      //rxbuf[19] = 254; // écrire en decimal

      if (rxlen != 24)
      {
        termPutchar('\r');
        sprintf(temp, "Erreur de longueur de trame : %d !\r", rxlen);
        termPutchar('\r');
        printString();
      }
      else
      {
        Sr = rxbuf[20] + rxbuf[21] * 256;
        SPr = rxbuf[22] + rxbuf[23] * 256;

        Sc = 0;
        SPc = 0;

        for (i=0; i<20; i++)
          {
            Sc = Sc + rxbuf[i];
            SPc = SPc + rxbuf[i] * (i+1);
          }
        sprintf(temp, "Sr = %d, SPr = %d\rSc = %d, SPc = %d\r\r", Sr, SPr, Sc, SPc);
        printString(temp);
      }

      if (Sr != Sc)
      {
        printString("Une correction est necessaire !");
        erreur = Sc-Sr;
        sprintf(temp, "Valeur erreur = %d", erreur); 
        printString(temp);
        termPutchar('\r');
        rang = (SPc - SPr) / erreur;

        if (rang != 0)
        {
          sprintf(temp, "Correction de %02x en %02x au rang %d...\r", rxbuf[rang-1], rxbuf[rang-1] - erreur, rang - 1);
          printString(temp);
        }
        else
        {
          printString("Erreur sur la redondance, pas sur les DATA utiles\r");
        }
      }
      else
        if (SPr != SPc)
        {
          printString("Erreur sur la redondance, pas sur les DATA utiles\r");
        }
    }
    state = ECOUTE;
    termPutchar('\r\r');
    break;
  
    default:
      break;
 }
}
