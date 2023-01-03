//Emission d'un seul paquet
void Fonction_envoie_data_TTL_noDest(uint16_t send_mode, uint16_t freq, uint16_t valTTL){
  M5.Lcd.clear(BLACK); //Permet d'effacer l'écran

  printString("Emission du paquet : ");
  sprintf(temp, "%u", Seq);
  printString(temp);
  termPutchar('\r');
  
  //Remplissage des octets de la trame :
  txbuf[0] = send_mode; // Premier octet réservé au mode d'envoie
  txbuf[1] = 255; // Deuxième octet reservé à l'adresse broadcast
  txbuf[2] = 255; // Troisième octet réservé à l'adresse broadcast
    
  for (i = 3; i<5; i++){ // On insère 2 octets de données utiles dans la trame
    txbuf[i] = d;// 2 octets à 128 de payload
    d=d+1;
  }
  txbuf[5] = valTTL;

  // affichage pour debug
  for (i = 0; i<6; i++) {
    // Affichage de la trame envoyée (De tous les octets) :
    printString(" | ");
    sprintf(temp, "%u", txbuf[i]);
    printString(temp);
  }
  termPutchar('\r');
  termPutchar('\r');

  rf95.send(txbuf, 6);    // emission
  Seq = Seq +1;  
  rf95.waitPacketSent();
 
}
