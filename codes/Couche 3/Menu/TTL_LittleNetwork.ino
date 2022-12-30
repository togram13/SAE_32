//Utile pour les petits réseaux car ce mode permet de garder en mémoire le dernier paquet reçu et donc savoir si on nous l'a déjà envoyé ou pas.

// Emission d'un seul paquet (Le premier)
void Fonction_envoie_data_TTL_LittleNet(uint16_t freq, uint16_t valTTL, uint16_t *ipm5){
  M5.Lcd.clear(BLACK); //Permet d'effacer l'écran
  rf95.setFrequency(freq);
  sequence = sequence+1;
  printString("Emission du paquet n° ");
  sprintf(temp, "%u", sequence);
  printString(temp);
  termPutchar('\r');
  
  //Remplissage des octets de la trame :
  txbuf[0] = 255; // Premier octet reservé à l'adresse broadcast
  txbuf[1] = 255; // Deuxieme octet réservé à l'adresse broadcast
  txbuf[2] = valTTL; // troisième octet réservé au ttl (durée de vie du paquet)
  txbuf[3] = self_ip[0]; // 4ème octet réservé à l'adresse source
  txbuf[4] = self_ip[1]; // Deuxieme octet réservé à l'adresse source
  txbuf[5] = sequence; // Deuxieme octet réservé à l'adresse source

  sequence = txbuf[5];
  source = txbuf[3];

  derniere_sequence = sequence;
  derniere_source = source;

  sprintf(temp, "Adresse source : %u.%u", txbuf[3], txbuf[4]);
  printString(temp);
  termPutchar('\r');

  for (i = 6; i<8; i++) {// On insère 2 octets de données utiles dans la trame
    txbuf[i] = 128;// 2 octets à 128 de payload
  }

  // affichage pour debug
  for (i = 0; i<8; i++) {
    // Affichage de la trame envoyée (De tous les octets) :
    printString(" | ");
    sprintf(temp, "%u", txbuf[i]);
    printString(temp);
    }

    termPutchar('\r');
    termPutchar('\r');

  rf95.send(txbuf, 8);    // emission des 8 octets du paquet
  rf95.waitPacketSent();
}
