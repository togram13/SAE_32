#include <SPI.h> //Nativement dans arduino donc pas besoin de l'installer en +
#include <RH_RF95.h> //gestion des transmissions radio en Lora
#include <M5Stack.h>
 
#define RFM95_CS 5 //M5Stack Lora
#define RFM95_DIO0 36
RH_RF95 rf95(RFM95_CS, RFM95_DIO0);

int  RF95_MAX_MESSAGE_LEN = 16;
uint8_t rxbuf[16], rxbuflen = RF95_MAX_MESSAGE_LEN;
uint8_t txbuf[16], txbuflen = RF95_MAX_MESSAGE_LEN;
uint8_t rxlen = RF95_MAX_MESSAGE_LEN;
uint8_t state,RxSeq,TxSeq, credit;
uint32_t attente; // Si on veut faire du full duplex et que chaque noeud soit emetteur et récepteur à la fois, on prépare une variable d'attente pour un potentiel CDG 
 
#define E0 0 // Activation de l'ecoute
#define E1 1 // Attente de la trame de data
#define E2 2 //Affichage de la trame reçue
#define E3 3 //Emission du ACK
// Pas d'attente d'ACK pour la trame ACK (Boucle infinie sinon) => C'est l'arrivée de la trame suivante qui correspond à la bonne récéption du ACK par l'emetteur
 
#define canal 1 // canal similaire à celui de l'emetteur
 
#define TYPE_DATA 1 // Défiinition du type de trame DATA à 1
#define TYPE_ACK 2 //Définition du type de trame ACK à 2
 
void setup(){

   M5.Power.begin(); //initialisation de la puissance
   M5.begin(9600); //règle le débit du M5 à 9600 bauds : Le même que celui de l'emetteur (Sinon, l'échange ne fonctionnerait pas)
   Serial.begin(115200);
   M5.Lcd.setTextColor(RED, TFT_BLACK);
   Serial.print("Video DATA ACK Point a point\n");
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
 
   state = E0; // Activation de l'écoute
 
   delay(1000);
   Serial.print("Boucle principale\n");
   
   RxSeq = 255; // pas encore reçu la trame 0, donc 0-1 = 255 sur 8 bits non signés.
 
}
 
void loop(){
   int j; // Variable locale qui parcoure les octets de la trame reçue à afficher
   
   switch (state){
     case E0: // Se mettre en reception de donnée
     
         Serial.print("etat 0 :  en ECOUTE\n"); // Affichage de l'état en cours
         Serial.print("\n");
         
         rf95.setModeRx(); // activation du récepteur Radio
         state = E1; // Passage de la variable à l'état d'attente
         break;
     
     case E1: // vérifier si un message reçu est diponible
         rxlen = RF95_MAX_MESSAGE_LEN; // Taille max du buffer en reception à réinitialiser pour reçevoir une prochaine plus longue
        
         if(rf95.recv(rxbuf, &rxlen)) // Si la trame de DATA est présente dans le buffer
         {
            if (rxbuf[0]== TYPE_DATA)// Si le premier octet du message est le type DATA
            { 
              state = E2; // On passe à l'état d'affichage de la trame reçue
            }    
            else // Si le premier octet n'est pas de type DATA
            {
              state = E0; // On repasse à l'état réception 
            }
         }
         
         break;
         
     case E2: //etat d'affichage de la trame
         
         if (RxSeq != rxbuf[1])// Si le num de séquence de la trame reçue est différent du numéro de séquence de la dernière trame reçue => c'est la 1er fois qu'on reçoit cette trame
         { 
             RxSeq = rxbuf[1]; // On entre le numéro de séquence dans le buffer de réception
             Serial.print("numero de sequence :  ");
             Serial.print("[");
             Serial.print(RxSeq); // Affichage du numéro de séquence de la trame
             Serial.print("]\n");
             Serial.print("Nombre d'octets :  ");
             Serial.print(rxlen); // Taille de la trame
             Serial.println();
             
             for (j = 0; j < rxlen; j++)// Boucle qui permet d'efficher tous les octets
             {
                Serial.print("Octet ");
                Serial.print(j+1);
                Serial.print(" : ");
                Serial.print(rxbuf[j]); // Affichage du contenu de la trame
                Serial.print("\n");
             }
             Serial.print("\n");
                 
         } 
         else // si on a déjà reçue cette trame
         // On estime que l'émetteur n'a pas reçu l'aqcuittement
         { 
             Serial.print("Duplication de la trame\n");
             Serial.print(" => Absorption de la trame\n");
             Serial.print("\n");
         }
         state = E3; // état d'emission du ACK
   
         break;
        
     case E3: //envoi du ACK
     
         Serial.print("Etat 3 : Emission du ACK\n");
         Serial.print("\n");
         txbuf[0] = TYPE_ACK; // octet indiquant le type_message = ACK
         txbuf[1] = rxbuf[1]; //ACK de même numéro de séquence que la trame DATA
         
         rf95.send(txbuf, 2); // Envoi de la trame
         rf95.waitPacketSent(); // Attente que le transceiver ait bien transmis la trame
   
         state = E0; // Etat d'écoute
         break;
      
   default:
       break;
   }
}
