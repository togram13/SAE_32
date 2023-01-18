void Fonction_envoie_data_TTL_Dest(uint16_t send_mode, uint16_t freq, uint16_t *self_ip, uint16_t *dest_ip){
  M5.Lcd.clear(BLACK); //Permet d'effacer l'écran
  rf95.setFrequency(freq);

  printString("Emission du paquet : ");
  sprintf(temp, "%u", Seq);
  printString(temp);
  termPutchar('\r');
  
  //Remplissage des octets de la trame :
  txbuf[0] = send_mode; // Premier octet réservé au mode d'envoie
  txbuf[1] = dest_ip[0]; // Deuxième octet reservé à l'adresse destination
  txbuf[2] = dest_ip[1]; // Troisième octet réservé à l'adresse destination
  txbuf[3] = self_ip[0]; // Quatrième octet reservé à l'adresse source
  txbuf[4] = self_ip[1]; // Cinquième octet réservé à l'adresse source
    
  for (i = 5; i<7; i++){ // On insère 2 octets de données utiles dans la trame
    txbuf[i] = d;// 2 octets à 128 de payload
    d=d+1;
  }
  txbuf[8] = ttl;

  // affichage pour debug
  for (i = 0; i<9; i++) {
    // Affichage de la trame envoyée (De tous les octets) :
    printString(" | ");
    sprintf(temp, "%u", txbuf[i]);
    printString(temp);
  }
  termPutchar('\r');
  termPutchar('\r');

  rf95.send(txbuf, 9);    // emission
  Seq = Seq +1;
  rf95.waitPacketSent();
 
}