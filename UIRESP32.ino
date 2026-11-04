#include <Wire.h>
#include <U8g2lib.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <Preferences.h>
#include <ir_Panasonic.h>


const uint16_t kRecvPin = 14;
const uint16_t kSendPin = 13;
const uint16_t kButtonUpPin = 26;
const uint16_t kButtonDownPin = 25;
const uint16_t kButtonSelectPin = 27;

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
IRrecv irrecv(kRecvPin);
IRsend irsend(kSendPin);
IRPanasonicAc panasonic_ac(kSendPin);
Preferences preferences;
decode_results results;


enum class MenuState { MAIN, AC_ACTIONS, LEARN_SLOTS, LEARN_ACTIONS };
MenuState currentMenu = MenuState::MAIN;
int mainMenuSelection = 0;
int acMenuSelection = 0;
int learnSlotSelection = 0;
int learnActionSelection = 0;


const int kMaxLearnSlots = 3;
const int kMaxRawLen = 512;
struct IrCode {
  uint16_t rawbuf[kMaxRawLen];
  uint16_t rawlen;
};
IrCode learnedCodes[kMaxLearnSlots];

void displayMessage(String line1, String line2 = "", String line3 = "");
void displayMainMenu();
void displayAcMenu();
void displayLearnSlotsMenu();
void displayLearnActionsMenu();

void setup() {
  Serial.begin(115200);
  pinMode(kButtonUpPin, INPUT_PULLUP);
  pinMode(kButtonDownPin, INPUT_PULLUP);
  pinMode(kButtonSelectPin, INPUT_PULLUP);

  u8g2.begin();
  irrecv.enableIRIn();
  irsend.begin();
  panasonic_ac.begin();

  loadAllCodes();
  displayMainMenu();
}

void loop() {
  handleButtons();
}

void handleButtons() {
  switch (currentMenu) {
    case MenuState::MAIN:         handleMainMenuButtons(); break;
    case MenuState::AC_ACTIONS:   handleAcMenuButtons(); break;
    case MenuState::LEARN_SLOTS:  handleLearnSlotsButtons(); break;
    case MenuState::LEARN_ACTIONS:handleLearnActionsButtons(); break;
  }
}

void handleMainMenuButtons() {
  if (digitalRead(kButtonUpPin) == LOW)   { delay(200); mainMenuSelection = 0; displayMainMenu(); }
  if (digitalRead(kButtonDownPin) == LOW) { delay(200); mainMenuSelection = 1; displayMainMenu(); }
  if (digitalRead(kButtonSelectPin) == LOW) {
    delay(200);
    if (mainMenuSelection == 0) {
      currentMenu = MenuState::AC_ACTIONS;
      acMenuSelection = 0;
      displayAcMenu();
    } else {
      currentMenu = MenuState::LEARN_SLOTS;
      learnSlotSelection = 0;
      displayLearnSlotsMenu();
    }
  }
}

void handleAcMenuButtons() {
  const int numActions = 3;
  if (digitalRead(kButtonUpPin) == LOW)   { delay(200); acMenuSelection = (acMenuSelection - 1 + numActions) % numActions; displayAcMenu(); }
  if (digitalRead(kButtonDownPin) == LOW) { delay(200); acMenuSelection = (acMenuSelection + 1) % numActions; displayAcMenu(); }
  if (digitalRead(kButtonSelectPin) == LOW) {
    delay(200);
    switch(acMenuSelection) {
      case 0:
        displayMessage("Sending AC", "Power ON");
        panasonic_ac.on(); panasonic_ac.setPower(true); panasonic_ac.setMode(kPanasonicAcCool);
        panasonic_ac.setTemp(24);
        panasonic_ac.send();
        delay(1000);
        break;
      case 1:
        displayMessage("Sending AC", "Power OFF");
        panasonic_ac.on(); panasonic_ac.setPower(false);
        panasonic_ac.send();
        delay(1000);
        break;
      case 2:
        currentMenu = MenuState::MAIN;
        displayMainMenu();
        return;
    }
    displayAcMenu();
  }
}

void handleLearnSlotsButtons() {
  if (digitalRead(kButtonUpPin) == LOW)   { delay(200); learnSlotSelection = (learnSlotSelection - 1 + kMaxLearnSlots) % kMaxLearnSlots; displayLearnSlotsMenu(); }
  if (digitalRead(kButtonDownPin) == LOW) { delay(200); learnSlotSelection = (learnSlotSelection + 1) % kMaxLearnSlots; displayLearnSlotsMenu(); }
  if (digitalRead(kButtonSelectPin) == LOW) {
    delay(200);
    currentMenu = MenuState::LEARN_ACTIONS;
    learnActionSelection = 0;
    displayLearnActionsMenu();
  }
}

void handleLearnActionsButtons() {
  const int numActions = 3;
  if (digitalRead(kButtonUpPin) == LOW)   { delay(200); learnActionSelection = (learnActionSelection - 1 + numActions) % numActions; displayLearnActionsMenu(); }
  if (digitalRead(kButtonDownPin) == LOW) { delay(200); learnActionSelection = (learnActionSelection + 1) % numActions; displayLearnActionsMenu(); }
  if (digitalRead(kButtonSelectPin) == LOW) {
    delay(200);
    switch(learnActionSelection) {
      case 0:
        enterLearnMode();
        break;
      case 1:
        if (learnedCodes[learnSlotSelection].rawlen > 0) {
          displayMessage("Transmitting", "Slot " + String(learnSlotSelection + 1));
          irsend.sendRaw(learnedCodes[learnSlotSelection].rawbuf, learnedCodes[learnSlotSelection].rawlen, 38);
          delay(1000);
        } else {
          displayMessage("Slot is Empty");
          delay(1000);
        }
        break;
      case 2:
        currentMenu = MenuState::LEARN_SLOTS;
        displayLearnSlotsMenu();
        return;
    }
    displayLearnActionsMenu();
  }
}

void enterLearnMode() {
  displayMessage("LEARN MODE:", "Point remote &", "press button");
  irrecv.enableIRIn();
  while (true) {
    if (irrecv.decode(&results)) {
      if (results.rawlen > 20 && results.rawlen < kMaxRawLen) {
        learnedCodes[learnSlotSelection].rawlen = results.rawlen - 1;
        for (int i = 0; i < learnedCodes[learnSlotSelection].rawlen; i++) {
          learnedCodes[learnSlotSelection].rawbuf[i] = results.state[i + 1];
        }
        saveCode(learnSlotSelection);
        displayMessage("SUCCESS!", "Saved to Slot " + String(learnSlotSelection + 1));
        delay(1500);
        return;
      }
      irrecv.resume();
    }
    if (digitalRead(kButtonSelectPin) == LOW) { delay(200); displayMessage("Cancelled"); delay(1000); return; }
  }
}

void saveCode(int slot) {
  preferences.begin("ir_remote", false);
  String key = "slot" + String(slot);
  preferences.putBytes(key.c_str(), &learnedCodes[slot], sizeof(IrCode));
  preferences.end();
}

void loadAllCodes() {
  preferences.begin("ir_remote", true);
  for (int i = 0; i < kMaxLearnSlots; i++) {
    String key = "slot" + String(i);
    if (!preferences.getBytes(key.c_str(), &learnedCodes[i], sizeof(IrCode))) {
      learnedCodes[i].rawlen = 0;
    }
  }
  preferences.end();
}

void displayMainMenu() {
  const char* items[] = {"Smart AC", "Learn New Code"};
  drawMenu("Main Menu", items, 2, mainMenuSelection);
}

void displayAcMenu() {
  const char* items[] = {"Power ON", "Power OFF", "Back"};
  drawMenu("Smart AC", items, 3, acMenuSelection);
}

void displayLearnSlotsMenu() {
  const char* items[kMaxLearnSlots];
  String slotLabels[kMaxLearnSlots];
  for (int i=0; i<kMaxLearnSlots; i++) {
    slotLabels[i] = "Slot " + String(i+1);
    if (learnedCodes[i].rawlen > 0) slotLabels[i] += " [Full]";
    else slotLabels[i] += " [Empty]";
    items[i] = slotLabels[i].c_str();
  }
  drawMenu("Select Slot", items, kMaxLearnSlots, learnSlotSelection);
}

void displayLearnActionsMenu() {
  String title = "Slot " + String(learnSlotSelection + 1);
  const char* items[] = {"Save (Learn)", "Transmit", "Back"};
  drawMenu(title.c_str(), items, 3, learnActionSelection);
}

void drawMenu(const char* title, const char* items[], int count, int selection) {
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_t0_11b_tr);
    u8g2.drawStr((128 - u8g2.getStrWidth(title)) / 2, 12, title);
    u8g2.drawHLine(0, 16, 128);
    u8g2.setFont(u8g2_font_5x8_tr);
    for (int i = 0; i < count; i++) {
      if (i == selection) {
        u8g2.drawStr(10, 30 + (i * 12), ">");
      }
      u8g2.drawStr(20, 30 + (i * 12), items[i]);
    }
  } while (u8g2.nextPage());
}

void displayMessage(String line1, String line2, String line3) {
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.drawStr((128 - u8g2.getStrWidth(line1.c_str())) / 2, 20, line1.c_str());
    u8g2.setFont(u8g2_font_profont12_tr);
    u8g2.drawStr((128 - u8g2.getStrWidth(line2.c_str())) / 2, 40, line2.c_str());
    u8g2.drawStr((128 - u8g2.getStrWidth(line3.c_str())) / 2, 55, line3.c_str());
  } while (u8g2.nextPage());
}

