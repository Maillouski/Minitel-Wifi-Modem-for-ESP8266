/******************************************
* Code réinterprété ou carrément récupéré de la librarie Minitel1B_Hard
* d'Éric Sérandour (https://github.com/eserandour/Minitel1B_Hard)
*
*/


unsigned long getKeyCode(bool unicode) {
  // Renvoie le code brut émis par le clavier (unicode = false)
  // ou sa conversion unicode si applicable (unicode = true, choix par défaut)
  unsigned long code = 0;
  // Code unique
  if (Serial.available()>0) {
    code = Serial.read();
  }
  // Séquences de deux ou trois codes (voir p.118)
  if (code == 0x19) {  // SS2
    while (!Serial.available()>0) yield();  // Indispensable
    code = (code << 8) + Serial.read();
    // Les diacritiques (3 codes)
    if ((code == 0x1941) || (code == 0x1942) || (code == 0x1943) || (code == 0x1948) || (code == 0x194B)) {  // Accents, tréma, cédille
      // Bug 1 : Pour éviter de compter un caractère lorsqu'on appuie plusieurs fois de suite sur une touche avec accent ou tréma
      byte caractere = 0x19;
      while (caractere == 0x19) {  
        while (!Serial.available()>0) yield();  // Indispensable
        caractere = Serial.read();
        if (caractere == 0x19) {
          while (!Serial.available()>0) yield();  // Indispensable
          caractere = Serial.read();
          caractere = 0x19;
        }
      }
      // Bug 2 : Pour éviter de compter un caractère lorsqu'on appuie sur les touches de fonction après avoir appuyé sur une touche avec accent ou tréma
      if (caractere == 0x13) {  // Les touches RETOUR REPETITION GUIDE ANNULATION SOMMAIRE CORRECTION SUITE CONNEXION_FIN ont un code qui commence par 0x13
          while (!Serial.available()>0) yield();  // Indispensable
          caractere = Serial.read();  // Les touches de fonction sont codées sur 2 octets (0x13..)
          caractere = 0;
          code = 0;
      }
      code = (code << 8) + caractere;
      if (unicode) {
        switch (code) {  // On convertit le code reçu en unicode
          case 0x194161 : code = 0xE0; break;  // à
          case 0x194165 : code = 0xE8; break;  // è
          case 0x194175 : code = 0xF9; break;  // ù
          case 0x194265 : code = 0xE9; break;  // é
          case 0x194361 : code = 0xE2; break;  // â
          case 0x194365 : code = 0xEA; break;  // ê
          case 0x194369 : code = 0xEE; break;  // î
          case 0x19436F : code = 0xF4; break;  // ô
          case 0x194375 : code = 0xFB; break;  // û
          case 0x194861 : code = 0xE4; break;  // ä
          case 0x194865 : code = 0xEB; break;  // ë
          case 0x194869 : code = 0xEF; break;  // ï
          case 0x19486F : code = 0xF6; break;  // ö
          case 0x194875 : code = 0xFC; break;  // ü
          case 0x194B63 : code = 0xE7; break;  // ç
          default : code = caractere; break;
        }
      }
    }
    // Les autres caractères spéciaux disponibles sous Arduino (2 codes)
    else {
      if (unicode) {
        switch (code) {  // On convertit le code reçu en unicode
          case 0x1923 : code = 0xA3; break;    // Livre
          case 0x1927 : code = 0xA7; break;    // Paragraphe
          case 0x192C : code = 0x2190; break;  // Flèche gauche
          case 0x192E : code = 0x2192; break;  // Flèche droite
          case 0x192F : code = 0x2193; break;  // Flèche bas
          case 0x1930 : code = 0xB0; break;    // Degré
          case 0x1931 : code = 0xB1; break;    // Plus ou moins
          case 0x1938 : code = 0xF7; break;    // Division
          case 0x196A : code = 0x0152; break;  // Ligature OE
          case 0x197A : code = 0x0153; break;  // Ligature oe
          case 0x197B : code = 0x03B2; break;  // Bêta
        }
      }
    }
  }
  // Touches de fonction (voir p.123)
  else if (code == 0x13) {
    while (!Serial.available()>0) yield();  // Indispensable
    code = (code << 8) + Serial.read();
  }  
  // Touches de gestion du curseur lorsque le clavier est en mode étendu (voir p.124)
  // Pour passer au clavier étendu manuellement : Fnct C + E
  // Pour revenir au clavier vidéotex standard  : Fnct C + V
  else if (code == 0x1B) {
    delay(20);  // Indispensable. 0x1B seul correspond à la touche Esc,
                // on ne peut donc pas utiliser la boucle while (!available()>0).           
    if (Serial.available()>0) {
      code = (code << 8) + Serial.read();
      if (code == 0x1B5B) {
        while (!Serial.available()>0) yield();  // Indispensable
        code = (code << 8) + Serial.read();
        if ((code == 0x1B5B34) || (code == 0x1B5B32)) {
          while (!Serial.available()>0) yield();  // Indispensable
          code = (code << 8) + Serial.read();
        }
      }
    }
  }
  else {
    if (unicode) {  // On convertit les codes uniques en unicode
      switch (code) {
        case 0x5E : code = 0x2191; break;  // Flèche haut
        case 0x60 : code = 0x2014; break;  // Tiret cadratin
      }
    }
  }
// Pour test
/*
  if (code != 0) {
    Serial.print(code,HEX);
    Serial.print(" ");
    Serial.write(code);
    Serial.println("");
  }
*/
  return code;
}

void sendConnexionFinSequence()
{
  sendConnexion();
  yield();
  delay(500);
  yield();
  sendDeconnexion();
  
}

void setTeleinfo(){
  Serial.write(ESC);
  Serial.write(PRO2);
  Serial.write(TELINFO >> 8);
  Serial.write(TELINFO & 0xFF);
  delay(500);
  while (!Serial);yield();
  while(Serial.available() > 0)
  {
    Serial.read();
  }
}

void setVideotex(){
  if (MinitelMode != "VIDEOTEX")
  {
    MinitelMode = "VIDEOTEX";
    Serial.write(CSI >> 8);     // Décale les bits de 8 positions et envoie le byte de poids fort (MSB)
    Serial.write(CSI & 0xFF);   // Envoie le byte de poids faible (LSB)
    Serial.write(0x3F);
    Serial.write(0x7B);
    delay(500);
    while (!Serial);yield();
    while(Serial.available() > 0)
    {
      Serial.read();
    }
  }
}

void clearMinitelScreen(){

  //Move cursor to top left
  Serial.write(CSI >> 8);     // Décale les bits de 8 positions et envoie le byte de poids fort (MSB)
  Serial.write(CSI & 0xFF);   // Envoie le byte de poids faible (LSB)
  Serial.write(0x31); // 0x30 + pos. Colonne
  Serial.write(0x3B);
  Serial.write(0x31); // 0x30 + pos. Ligne
  Serial.write(0x48); 
  delay(500);
  while (!Serial);yield();
  while(Serial.available() > 0)
  {
    Serial.read();
  }

  // Clear the Screen
  Serial.write(CSI >> 8);     // Décale les bits de 8 positions et envoie le byte de poids fort (MSB)
  Serial.write(CSI & 0xFF);   // Envoie le byte de poids faible (LSB)
  Serial.write(0x32);
  Serial.write(0x4A);
  delay(500);
  while (!Serial);yield();
  while(Serial.available() > 0)
  {
    Serial.read();
  }
}

void sendConnexion(){
  Serial.write(ESC);
  Serial.write(PRO1);
  Serial.write(CONNEXION);
}

void sendDeconnexion(){
  Serial.write(ESC);
  Serial.write(PRO1);
  Serial.write(DECONNEXION);
  delay(500);
}

void SetMinitelEchoOff(){
  Serial.write(ESC);
  Serial.write(PRO3);
  Serial.write(AIGUILLAGE_OFF);
  Serial.write(CODE_RECEPTION_MODEM);
  Serial.write(CODE_EMISSION_CLAVIER);
  delay(500);
  while (!Serial);yield();
  while(Serial.available() > 0)
  {
    Serial.read();
  }
  
}

void setMinitelBaud(int baudRate)
{
  Serial.write(ESC);
  Serial.write(PRO2);
  Serial.write(PROG);
  Serial.write(baudRate);
  delay(500);
  while (!Serial);yield();
  while(Serial.available() > 0)
  {
    Serial.read();
  }
}

void setModeMixte() {  // Voir p.144
  // Passage du standard Télétel mode Vidéotex au standard Télétel mode Mixte
  // Commande
  if (MinitelMode != "MIXTE")
  {
    MinitelMode = "MIXTE";
    Serial.write(ESC);
    Serial.write(PRO2);   // 0x1B 0x3A
    Serial.write(MIXTE1 >> 8);     // Décale les bits de 8 positions et envoie le byte de poids fort (MSB)
    Serial.write(MIXTE1 & 0xFF); // 0x32 0x7D
    delay(500);
    while (!Serial);yield();
    while(Serial.available() > 0)
    {
      Serial.read();
    }
  }
}

void minitelNewScreen(){
  Serial.write(ESC);
  Serial.write(0xFF);
  delay(500);
  while (!Serial);yield();
  while(Serial.available() > 0)
  {
    Serial.read();
  }
}

void setModeVideotexFromMixte() {  // Voir p.144
  // Passage du standard Télétel mode Mixte au standard Télétel mode Vidéotex
  // Commande
  if (MinitelMode != "VIDEOTEX")
  {
    MinitelMode = "VIDEOTEX";
    Serial.write(ESC);
    Serial.write(PRO2);   // 0x1B 0x3A
    Serial.write(MIXTE2 >> 8);     // Décale les bits de 8 positions et envoie le byte de poids fort (MSB)
    Serial.write(MIXTE2 & 0xFF);  // 0x32 0x7E
    delay(500);
    while (!Serial);yield();
    while(Serial.available() > 0)
    {
      Serial.read();
    }
  }
  // Acquittement
}

void cursorOn(){
  Serial.write(ESC);
  Serial.write(CON);
  delay(500);
  while (!Serial);yield();
  while(Serial.available() > 0)
  {
    Serial.read();
  }
}

void newXY(){
  Serial.write(ESC);
  Serial.write(RS);
  Serial.write(US);
  Serial.write(0x40);
  Serial.write(0x40);
  Serial.write(0x18);
  delay(500);
  while (!Serial);yield();
  while(Serial.available() > 0)
  {
    Serial.read();
  }
}

void capitalMode() {
  // Commande
  Serial.write(ESC);
  Serial.write(PRO2);   // 0x1B 0x3A
  Serial.write(STOP);        // 0x6A
  Serial.write(MINUSCULES);  // 0x45
  delay(500);
  while (!Serial);yield();
  while(Serial.available() > 0)
  {
    Serial.read();
  }
}

void clearLine() {  // Voir p.95
  Serial.write(CSI >> 8);     // Décale les bits de 8 positions et envoie le byte de poids fort (MSB)
  Serial.write(CSI & 0xFF);   // Envoie le byte de poids faible (LSB)
  Serial.write(0x32);
  Serial.write(0x4B);
  delay(500);
  while (!Serial);yield();
  while(Serial.available() > 0)
  {
    Serial.read();
  }
}

void deleteLines(int n) {  // Voir p.95
  Serial.write(CSI >> 8);     // Décale les bits de 8 positions et envoie le byte de poids fort (MSB)
  Serial.write(CSI & 0xFF);   // Envoie le byte de poids faible (LSB)
  Serial.write(0x31);  // Voir section Private ci-dessous
  Serial.write(0x4D);
  delay(500);
  while (!Serial);yield();
  while(Serial.available() > 0)
  {
    Serial.read();
  }
}

void resetMinitel() {  // Voir p.145
  // Commande
  Serial.write(ESC);
  Serial.write(PRO1);
  Serial.write(RESET);  // 0x7F
  delay(500);
  while (!Serial);yield();
  while(Serial.available() > 0)
  {
    Serial.read();
  }
}

void pageMode() {
  // Commande
  Serial.write(ESC);
  Serial.write(PRO2);
  Serial.write(STOP); 
  Serial.write(ROULEAU);  // 0x43
  delay(500);
  while (!Serial);yield();
  while(Serial.available() > 0)
  {
    Serial.read();
  }
}

void scrollMode() {
  // Commande
  Serial.write(ESC);
  Serial.write(PRO2);
  Serial.write(START);    // 0x69
  Serial.write(ROULEAU);  // 0x43
  delay(500);
  while (!Serial);yield();
  while(Serial.available() > 0)
  {
    Serial.read();
  }
}

void testMode(){
  // Mettre le clavier en mode etendu, obligatoire ?
  Serial.write(ESC);
  Serial.write(PRO3);                   // 0x1B 0x3B
  Serial.write(START);                   // 0x69
  Serial.write(CODE_RECEPTION_CLAVIER);  // 0x59
  Serial.write(ETEN);                    // 0x41
  delay(500);
  while (!Serial);yield();
  while(Serial.available() > 0)
  {
    Serial.read();
  }
  Serial.println("---------------- DEBUT -----------------");
  Serial.write(ESC);
  Serial.write(0x40);
  Serial.println("OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO");
  
  Serial.write(ESC);
  Serial.write("A");
  Serial.println("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
  
  Serial.write(ESC);
  Serial.write("B");
  Serial.println("BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB");
  
  Serial.write(ESC);
  Serial.write("C");
  Serial.println("CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC");
  
  Serial.write(ESC);
  Serial.write("D");
  Serial.println("DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD");
  
  Serial.write(ESC);
  Serial.write("E");
  Serial.println("EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE");
  
  Serial.write(ESC);
  Serial.write("F");
  Serial.println("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");
  
  Serial.write(ESC);
  Serial.write("G");
  Serial.println("GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");

}
