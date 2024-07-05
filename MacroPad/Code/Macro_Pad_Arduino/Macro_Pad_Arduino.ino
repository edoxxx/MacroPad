#include <HID-Project.h>
#include <HID-Settings.h>
#define HID_CUSTOM_LAYOUT
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <qrcode.h> // Includi la libreria QR code

// Definizione delle costanti per la matrice dei pulsanti
const int ROWS = 4;
const int COLS = 3;

// Definizione dei pin di collegamento
const int rowPins[ROWS] = { 9, 8, 7, 6 }; // Riga 1, Riga 2, Riga 3, Riga 4
const int colPins[COLS] = { 10, 16, 14 }; // Colonna 1, Colonna 2, Colonna 3
const int ctrlPin = A2; // Pin per il pulsante di controllo

const int keymap[ROWS][COLS] = {
  { 0, 4, 8 },
  { 1, 5, 9 },
  { 2, 6, 10 },
  { 3, 7, 11 },
};

// Stato precedente dei pulsanti
int lastButtonState[ROWS][COLS] = {
  { HIGH, HIGH, HIGH },
  { HIGH, HIGH, HIGH },
  { HIGH, HIGH, HIGH },
  { HIGH, HIGH, HIGH }
};

// Ingressi del Rotary Encoder
#define CLK 5
#define DT 4
#define SW 15

int counter = 0;
int currentStateCLK;
int lastStateCLK;
String currentDir = "";
unsigned long lastButtonPress = 0;

#define OLED_RESET 4 // Pin di reset per alcuni display
Adafruit_SSD1306 display(OLED_RESET);

#define btchangeencoder A0

bool ctrlPressed;
int row;
int col;

String mappamatrixSTR = "Cadence PCB";
String mappaencoderSTR = "Cadence PCB";

int mappaencoder = A1;
int mappamatrix = A0;

bool encoderPressed = 0;
bool matrixPressed;

int valueEncoder = 0;
int valueMatrix = 0;

int ButtonState = HIGH;

void setup() {

  pinMode(mappaencoder, INPUT_PULLUP);
  pinMode(mappamatrix, INPUT_PULLUP);

  // Inizializzazione della tastiera virtuale
  Keyboard.begin();
  Consumer.begin();

  // Inizializzazione dei pin per le colonne come output inizialmente alti
  for (int j = 0; j < COLS; j++) {
    pinMode(colPins[j], OUTPUT);
    digitalWrite(colPins[j], HIGH);
  }

  // Inizializzazione dei pin per le righe come input con pull-up
  for (int i = 0; i < ROWS; i++) {
    pinMode(rowPins[i], INPUT_PULLUP);
  }

  // Inizializzazione del pin per il pulsante di controllo come input con pull-up
  pinMode(ctrlPin, INPUT_PULLUP);

  // Configura i pin dell'encoder come input
  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);
  pinMode(SW, INPUT_PULLUP);

  // Configura il monitor seriale
  Serial.begin(9600);

  // Leggi lo stato iniziale di CLK
  lastStateCLK = digitalRead(CLK);

  // Inizializzazione del display OLED
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Indirizzo I2C del display, può variare

  // Imposta il testo per il display
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Pulsante cambio mappa
  pinMode(btchangeencoder, INPUT_PULLUP);

  //displayQRCode();
  //delay(5000);
  oled();
}

void loop() {
  // Leggi lo stato del pulsante di controllo
  ctrlPressed = digitalRead(ctrlPin) == LOW;

  matrixPressed = digitalRead(mappamatrix) == LOW;
  encoderPressed = digitalRead(mappaencoder);

  // Scansiona i pulsanti
  for (col = 0; col < COLS; col++) {
    // Disattiva tutte le colonne
    for (int j = 0; j < COLS; j++) {
      digitalWrite(colPins[j], HIGH);
    }

    // Attiva la colonna corrente
    digitalWrite(colPins[col], LOW);

    // Scansiona le righe per vedere se un pulsante è stato premuto
    for (row = 0; row < ROWS; row++) {
      int buttonState = digitalRead(rowPins[row]);

      // Se il pulsante è stato premuto
      if (buttonState == LOW && lastButtonState[row][col] == HIGH) {
        send_command();
      }

      // Memorizza lo stato attuale del pulsante
      lastButtonState[row][col] = buttonState;
    }
  }

  ctrlPressed ? Keyboard.press(KEY_LEFT_CTRL) : Keyboard.releaseAll();

  // Leggi lo stato attuale di CLK
  currentStateCLK = digitalRead(CLK);

  // Se lo stato precedente e attuale di CLK sono diversi, è avvenuto un impulso
  // Reagisci solo a 1 cambio di stato per evitare conteggi doppi
  if (currentStateCLK != lastStateCLK && currentStateCLK == 1) {
    // Se lo stato di DT è diverso da quello di CLK, l'encoder ruota in senso antiorario, quindi decrementa
    if (digitalRead(DT) != currentStateCLK) {

      if (valueEncoder == 0) {

        if (ctrlPressed) {
          Keyboard.press(HID_KEYBOARD_F14);
        } else {
          Keyboard.press(KEY_UP_ARROW);
        }
      } else if (valueEncoder == 1) {
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.press(KEY_Z);
      } else if (valueEncoder == 2) {
        Consumer.write(MEDIA_VOLUME_DOWN);
      }
    }

    else {
      if (valueEncoder == 0) {

        if (ctrlPressed) {

          Keyboard.press(HID_KEYBOARD_F13);
        } else {
          // L'encoder ruota in senso orario, quindi incrementa
          Keyboard.press(KEY_DOWN_ARROW);
        }
      } else if (valueEncoder == 1) {

        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.press(KEY_Y);
      } else if (valueEncoder == 2) {

        Consumer.write(MEDIA_VOLUME_UP);
      }
    }
    Keyboard.releaseAll();
  }

  // Ricorda l'ultimo stato di CLK
  lastStateCLK = currentStateCLK;

  // Leggi lo stato del pulsante
  int btnState = digitalRead(SW);

  // Se rilevi un segnale LOW, il pulsante è premuto
  if (btnState == LOW) {
    // Se sono passati 50ms dall'ultimo impulso LOW, significa che il pulsante è stato premuto, rilasciato e premuto di nuovo
    if (millis() - lastButtonPress > 10) {

      if (valueEncoder == 0) {
        Keyboard.press(HID_KEYBOARD_F2);
        Keyboard.releaseAll();
      } else if (valueEncoder == 1) {

      } else if (valueEncoder == 2) {
        Consumer.write(HID_CONSUMER_MUTE);
        //delay(200);
        Keyboard.releaseAll();
      }
    }
    // Ricorda l'ultimo evento di pressione del pulsante
    lastButtonPress = millis();
    Keyboard.releaseAll();
  }

  // Inserisci un leggero ritardo per aiutare il debounce della lettura
  delay(1);
  encodermap();
}

void encodermap() {
  if (encoderPressed == LOW) {
    valueEncoder = valueEncoder + 1;
    Serial.println("click");
    if (valueEncoder == 3) {
      valueEncoder = 0;
    }
    if (valueEncoder == 0) {
      mappaencoderSTR = "Cadence PCB";
    } else if (valueEncoder == 1) {
      mappaencoderSTR = "Undo Mode";
    } else if (valueEncoder == 2) {
      mappaencoderSTR = "Volume";
    }
    oled();
    long tempo = 0;
    tempo = millis();
    while (digitalRead(mappaencoder) == LOW) {
      if (millis() - tempo >= 3500) {
        displayQRCode();
        delay(8000);
        oled();
      }
      delay(30); // Piccolo ritardo per evitare sovraccarico della CPU
      Serial.println("loop");
    }
  }
}

void send_command() {
  switch (keymap[row][col]) {
    case 0:
      if (ctrlPressed) {
        Keyboard.releaseAll();
        delay(5);
        Keyboard.press(KEY_DELETE);
        delay(5);
        Keyboard.releaseAll();
        break;
      } else {
        Keyboard.press(KEY_LEFT_SHIFT);
        break;
      }
    case 1:
      if (ctrlPressed) {
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.press(HID_KEYBOARD_F2);
        delay(5);
        Keyboard.releaseAll();
        break;
      } else {
        Keyboard.press(HID_KEYBOARD_F2);
        delay(5);
        Keyboard.releaseAll();
        break;
      }
    case 2:
      if (ctrlPressed) {
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.press(HID_KEYBOARD_F3);
        delay(5);
        Keyboard.releaseAll();
        break;
      } else {
        Keyboard.press(HID_KEYBOARD_F3);
        delay(5);
        Keyboard.releaseAll();
        break;
      }
    case 3:
      if (ctrlPressed) {
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.press(HID_KEYBOARD_F4);
        delay(5);
        Keyboard.releaseAll();
        break;
      } else {
        Keyboard.press(HID_KEYBOARD_F4);
        delay(5);
        Keyboard.releaseAll();
        break;
      }
    case 4:
      if (ctrlPressed) {
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.press(HID_KEYBOARD_F5);
        delay(5);
        Keyboard.releaseAll();
        break;
      } else {
        Keyboard.press(HID_KEYBOARD_F5);
        delay(5);
        Keyboard.releaseAll();
        break;
      }
    case 5:
      if (ctrlPressed) {
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.press(HID_KEYBOARD_F6);
        delay(5);
        Keyboard.releaseAll();
        break;
      } else {
        Keyboard.press(HID_KEYBOARD_F6);
        delay(5);
        Keyboard.releaseAll();
        break;
      }
    case 6:
      if (ctrlPressed) {
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.press(HID_KEYBOARD_F7);
        delay(5);
        Keyboard.releaseAll();
        break;
      } else {
        Keyboard.press(HID_KEYBOARD_F7);
        delay(5);
        Keyboard.releaseAll();
        break;
      }
    case 7:
      if (ctrlPressed) {
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.press(HID_KEYBOARD_F8);
        delay(5);
        Keyboard.releaseAll();
        break;
      } else {
        Keyboard.press(HID_KEYBOARD_F8);
        delay(5);
        Keyboard.releaseAll();
        break;
      }
    case 8:
      if (ctrlPressed) {
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.press(HID_KEYBOARD_F9);
        delay(5);
        Keyboard.releaseAll();
        break;
      } else {
        Keyboard.press(HID_KEYBOARD_F9);
        delay(5);
        Keyboard.releaseAll();
        break;
      }
    case 9:
      if (ctrlPressed) {
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.press(HID_KEYBOARD_F10);
        delay(5);
        Keyboard.releaseAll();
        break;
      } else {
        Keyboard.press(HID_KEYBOARD_F10);
        delay(5);
        Keyboard.releaseAll();
        break;
      }
    case 10:
      if (ctrlPressed) {
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.press(HID_KEYBOARD_F11);
        delay(5);
        Keyboard.releaseAll();
        break;
      } else {
        Keyboard.press(HID_KEYBOARD_F11);
        delay(5);
        Keyboard.releaseAll();
        break;
      }
    case 11:
      if (ctrlPressed) {
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.press(HID_KEYBOARD_F12);
        delay(5);
        Keyboard.releaseAll();
        break;
      } else {
        Keyboard.press(HID_KEYBOARD_F12);
        delay(5);
        Keyboard.releaseAll();
        break;
      }
  }
}

void oled() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("       MacroPad");
  display.println("---------------------");
  display.print("Matrice: "); // Aggiungi qui lo stato della matrice
  display.println(mappamatrixSTR); // Sostituisci con il tuo stato attuale della matrice
  display.print("Encoder: "); // Aggiungi qui lo stato dell'encoder
  display.println(mappaencoderSTR); // Sostituisci con il tuo stato attuale dell'encoder
  display.display();
}

void displayQRCode() {
  const char* qrData = "https://github.com/edoxxx/MacroPad"; // Sostituisci con il tuo URL
  const int qrVersion = 3;
  const int qrSize = 40;
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(qrVersion)];

  qrcode_initText(&qrcode, qrcodeData, qrVersion, ECC_MEDIUM, qrData);

  display.clearDisplay();
  for (int y = 0; y < qrSize; y++) {
    for (int x = 0; x < qrSize; x++) {
      int pixel = qrcode_getModule(&qrcode, x, y);
      display.drawPixel(x + 48, y + 0, pixel ? SSD1306_WHITE : SSD1306_BLACK);
    }
  }
  display.display();
}
