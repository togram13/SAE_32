void Fonction_envoie_data_TTL_BigNet(uint16_t send_mode, uint16_t freq, uint16_t *self_ip){
  M5.Lcd.clear(BLACK); //Permet d'effacer l'écran
  rf95.setFrequency(freq);

  // Emission d'un seul paquet
  seq_em = seq_em +1;
  printString("Emission du paquet n° ");
  sprintf(temp, "%u", seq_em);
  printString(temp);
  termPutchar('\r');

  //Remplissage des octets de la trame :
  txbuf[0] = send_mode; // Premier octet réservé au mode d'envoie
  txbuf[1] = 255; // Premier octet reservé à l'adresse broadcast
  txbuf[2] = 255; // Deuxieme octet réservé à l'adresse broadcast
  txbuf[3] = ttl; // troisième octet réservé au ttl (durée de vie du paquet)
  txbuf[4] = self_ip[0]; // 4ème octet réservé à l'adresse source
  txbuf[5] = self_ip[1]; // Deuxieme octet réservé à l'adresse source
  txbuf[6] = seq_em; 

  seq_em = txbuf[6];

  printString("Adresse source : ");
  sprintf(temp, "%u.%u", txbuf[4], txbuf[5]);
  printString(temp);
     
  termPutchar('\r');

  for (i = 7; i<9; i++) // On insère 2 octets de données utiles dans la trame
    {
      txbuf[i] = 128;// 2 octets à 128 de payload
    }

  // affichage pour debug
  for (i = 0; i<9; i++) {
    // Affichage de la trame envoyée (De tous les octets) :
    printString(" | ");
    sprintf(temp, "%u", txbuf[i]);
    printString(temp);
  }

  termPutchar('\r');
  termPutchar('\r');
  rf95.send(txbuf, 9);    // emission des 8 octets du paquet
  rf95.waitPacketSent();
}