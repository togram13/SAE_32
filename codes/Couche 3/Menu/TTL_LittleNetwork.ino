//Utile pour les petits réseaux car ce mode permet de garder en mémoire le dernier paquet reçu et donc savoir si on nous l'a déjà envoyé ou pas.

// Emission d'un seul paquet (Le premier)
void Fonction_envoie_data_TTL_LittleNet(uint16_t send_mode, uint16_t freq, uint16_t *self_ip){
  M5.Lcd.clear(BLACK); //Permet d'effacer l'écran
  rf95.setFrequency(freq);
  sequence = sequence+1;
  printString("Emission du paquet n° ");
  sprintf(temp, "%u", sequence);
  printString(temp);
  termPutchar('\r');
  
  //Remplissage des octets de la trame :
  txbuf[0] = send_mode; // Premier octet réservé au mode d'envoie
  txbuf[1] = 255; // // Deuxième octet reservé à l'adresse broadcast
  txbuf[2] = 255; // Troisième octet réservé à l'adresse broadcast
  txbuf[3] = ttl; // 4ème octet réservé au ttl (durée de vie du paquet)
  txbuf[4] = self_ip[0]; // 5ème octet réservé à l'adresse source
  txbuf[5] = self_ip[1]; // Sixième octet réservé à l'adresse source
  txbuf[6] = sequence; // Septième octet réservé à la séquance

  source[0] = txbuf[4];
  source[1] = txbuf[5];

  derniere_sequence = sequence;
  derniere_source[0] = source[0];
  derniere_source[1] = source[1];

  sprintf(temp, "Adresse source : %u.%u", self_ip[0], self_ip[1]);
  printString(temp);
  termPutchar('\r');

  for (i = 7; i<9; i++) {// On insère 2 octets de données utiles dans la trame
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

  rf95.send(txbuf, 9);    // emission des 9 octets du paquet
  rf95.waitPacketSent();
}
