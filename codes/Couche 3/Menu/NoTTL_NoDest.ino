void Fonction_envoie_data_noTTL_noDest(uint16_t send_mode, uint16_t freq){
  //uint8_t : type de donnée qui est composé d'un octet (valeurs entre 0 et 255)
  char temp[255];

  M5.Lcd.clear(BLACK);

  rf95.setFrequency(freq); //règle la fréquence d'emission

  printString("Emission du paquet : ");
  sprintf(temp, "%u", Seq);
  printString(temp);
  termPutchar('\r');
  
  //Remplissage des octets de la trame :
  txbuf[0] = send_mode; // Premier octet réservé au mode d'envoie
  txbuf[1] = 255; // Deuxième octet reservé à l'adresse broadcast
  txbuf[2] = 255; // Troisième octet réservé à l'adresse broadcast
    
  for (i = 3; i<5; i++) // On insère 2 octets de données utiles dans la trame
    {
      txbuf[i] = 1;// 2 octets à 1 de payload
    }

  // affichage pour debug
  for (i = 0; i<5; i++) {
    // Affichage de la trame envoyée (De tous les octets) :
    sprintf(temp, "%u", txbuf[i]);
    printString(temp);
    printString(" | ");
    }
    termPutchar('\r');
    termPutchar('\r');

  delay(1000);
  rf95.send(txbuf, 5);    // emission
  Seq = Seq +1;
  rf95.waitPacketSent();
}