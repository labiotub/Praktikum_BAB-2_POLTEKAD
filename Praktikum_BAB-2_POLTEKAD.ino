#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <TM1637Display.h>
#include <Keypad.h>
#include <RTClib.h>

// Inisialisasi untuk LCD I2C
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Inisialisasi untuk 7-segment display
#define CLK 13
#define DIO 15
TM1637Display display(CLK, DIO);

// Inisialisasi RTC
RTC_DS3231 rtc;

// Konfigurasi pin untuk LED
#define GREEN_LED 18
#define YELLOW_LED 19
#define RED_LED 21

// Konfigurasi pin untuk switch dan button
#define SWITCH_PIN 34
#define BUTTON_PIN 35

// Konfigurasi keypad
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'2', '1', '3', 'A'},
  {'0', '*', '#', 'D'},
  {'8', '7', '9', 'C'},
  {'5', '4', '6', 'B'}
};
byte rowPins[ROWS] = {14, 25, 26, 27}; // Pin baris
byte colPins[COLS] = {32, 33, 4, 5};   // Pin kolom

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Variabel untuk menyimpan input angka sementara dan nilai yang ditampilkan
int inputValue = 0;
int displayedValue = 0;
bool inputPending = false; // Menandai jika ada input yang belum ditampilkan

// Waktu debounce untuk input keypad
unsigned long lastKeyPressTime = 0;
const unsigned long debounceDelay = 50; // 50 ms debounce

void setup() {
  // Inisialisasi I2C dengan pin custom
  Wire.begin(22, 23);

  // Inisialisasi LCD
  lcd.begin(16, 2);
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("System Init...");

  // Inisialisasi display 7-segment
  display.setBrightness(0x0f);
  display.showNumberDec(0);

  // Inisialisasi LED sebagai output
  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  digitalWrite(RED_LED, HIGH); // Menyalakan LED merah

  // Inisialisasi switch dan button sebagai input dengan PULLUP
  pinMode(SWITCH_PIN, INPUT_PULLUP);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Inisialisasi RTC
  if (!rtc.begin()) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("RTC Not Found");
    while (1); // Berhenti jika RTC tidak terhubung
  }

  // Cek apakah RTC berjalan atau perlu disetel ulang
  if (rtc.lostPower()) {
    // Setel waktu RTC secara manual
    rtc.adjust(DateTime(2024, 10, 20, 14, 30, 0));  // Sesuaikan dengan waktu saat ini
  }

  // Menampilkan pesan awal pada LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Ready!");

  // Inisialisasi Serial Monitor
  Serial.begin(115200);
}

void loop() {
  // Membaca input dari switch dan button
  int switchState = digitalRead(SWITCH_PIN);
  int buttonState = digitalRead(BUTTON_PIN);

  // Tampilkan status switch dan button di Serial Monitor
  Serial.print("Switch state: ");
  Serial.println(switchState == HIGH ? "ON" : "OFF");
  Serial.print("Button state: ");
  Serial.println(buttonState == LOW ? "RELEASED" : "PRESSED");

  // Reset semua jika switch dalam keadaan OFF
  if (switchState == LOW) {
    // Reset nilai dan tampilan
    display.clear();
    lcd.clear();
    inputValue = 0;
    displayedValue = 0;
    inputPending = false;

    lcd.setCursor(0, 0);
    lcd.print("Ready!"); // Tampilan setelah reset
    
    // Nyalakan LED merah jika switch OFF
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    return;
  }

  // Jika switch ON, memungkinkan input dari keypad
  char key = keypad.getKey();
  
  // Menambahkan debounce untuk input keypad
  if (key && (millis() - lastKeyPressTime > debounceDelay)) {
    lastKeyPressTime = millis(); // Simpan waktu terakhir tombol ditekan
    
    // Memproses input angka saja
    if (key >= '0' && key <= '9') {
      int num = key - '0';
      // Menghindari overflow di atas 255
      if (inputValue * 10 + num <= 255) {
        inputValue = inputValue * 10 + num;
        inputPending = true; // Tandai bahwa ada input yang belum ditampilkan
      }
    } else if (key == '*') {
      // Menghapus input jika '*' ditekan
      inputValue = 0;
      inputPending = true; // Tandai bahwa ada perubahan input
    }
  }

  // Jika tombol ditekan dan ada input yang belum ditampilkan
  if (buttonState == HIGH && inputPending) {
    // Update nilai yang ditampilkan
    displayedValue = inputValue; 
    inputPending = false; // Reset status input pending

    // Tampilkan nilai pada LCD, 7-segment, dan Serial Monitor
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Value: ");
    lcd.print(displayedValue);
    display.showNumberDec(displayedValue);

    Serial.print("Value: ");
    Serial.println(displayedValue);

    // Reset input setelah ditampilkan agar input baru mengganti angka sebelumnya
    inputValue = 0;

    // Menyalakan LED sesuai dengan angka genap atau ganjil
    if (displayedValue % 2 == 0) {
      // Jika angka genap, nyalakan LED kuning
      digitalWrite(YELLOW_LED, HIGH);
      digitalWrite(GREEN_LED, LOW);
    } else {
      // Jika angka ganjil, nyalakan LED hijau
      digitalWrite(GREEN_LED, HIGH);
      digitalWrite(YELLOW_LED, LOW);
    }

    // Matikan LED merah karena switch dalam keadaan ON
    digitalWrite(RED_LED, LOW);

    delay(200); // Debouncing button press
  }

  // Menampilkan waktu dari RTC di baris kedua LCD
  DateTime now = rtc.now();
  lcd.setCursor(0, 1);
  lcd.print(now.year());
  lcd.print('/');
  lcd.print(now.month());
  lcd.print('/');
  lcd.print(now.day());
  lcd.print(' ');
  lcd.print(now.hour());
  lcd.print(':');
  lcd.print(now.minute());
  lcd.print(':');
  lcd.print(now.second());

  // Simpan jeda minimum untuk stabilitas (kurangi delay untuk meningkatkan kecepatan)
  delay(10);
}
