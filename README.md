# Universal-IR-Remote
A DIY ESP32 universal remote with an OLED display and button controls. Features a hybrid system: "Smart Mode" uses native protocols for reliable AC control, while "Learn Mode" captures, saves, and replays raw IR signals from any TV or other appliance. Codes are stored persistently on the device.
# ESP32 Hybrid Universal IR Remote

**Version:** 1.0
**Author:** Rudra Arora
**Status:** Stable

## **Project Overview**

This project creates a standalone, ESP32-based universal infrared remote control with a hybrid operational model. It addresses the common failure of "learning remotes" to control complex, stateful appliances like air conditioners, while retaining universal learning capabilities for simpler devices.

The remote is fully operated by three physical push-buttons with a user-friendly graphical interface on a 0.96" OLED display.

### **Key Features**

*   **Hybrid IR System:**
    *   **Smart Mode (Slot 1):** Utilizes the `IRremoteESP8266` library's native protocol knowledge to generate perfect, checksum-correct signals for a **Panasonic Air Conditioner**. This method is 100% reliable.
    *   **Learn Mode (Slots 2-4):** Employs a raw signal capture method to learn, store, and replay signals from any standard remote control (e.g., TVs, stereos, fans).
*   **Persistent Memory:** Learned codes are saved to the ESP32's non-volatile flash memory using the `Preferences` library, ensuring they are retained even after power loss.
*   **Standalone Operation:** No computer or Serial Monitor is needed for operation after the initial setup. All functions are accessible via the on-device UI.
*   **On-Device Code Management:** Users can learn new codes into empty slots and delete previously saved codes directly from the device menu.

## **Hardware Requirements**

| Qty | Component | Specification |
|:---:|:---|:---|
| 1 | Microcontroller | ESP32-WROOM-32 Development Board |
| 1 | Display | 0.96" SSD1306 I2C OLED (128x64) |
| 1 | IR Receiver | VS1838B or equivalent (38kHz) |
| 1 | IR Transmitter | Standard 940nm 5mm IR LED |
| 3 | Push-button | 6x6mm Tactile Momentary Switch |
| - | Wires / Breadboard| Standard prototyping supplies |

## **Circuit and Wiring**

The components are connected to the ESP32 as follows. For a detailed visual layout, please refer to the `docs/circuit_diagram.png` file.

| Component Pin | Connects to ESP32 Pin |
|:---|:---|
| **IR LED** (Anode, long leg) | `GPIO 13` |
| **IR Receiver** (Data) | `GPIO 14` |
| **OLED Display** (SDA) | `GPIO 21` |
| **OLED Display** (SCL) | `GPIO 22` |
| **Button UP** | `GPIO 26` |
| **Button DOWN** | `GPIO 25` |
| **Button SELECT** | `GPIO 27` |
| **3.3V** | All VCC pins |
| **GND** | All GND pins & one leg of each button |

**Note:** Buttons are wired between their GPIO pin and `GND`. The ESP32's internal pull-up resistors are used, so no external resistors are required.

## **Software Setup and Installation**

1.  **Install Arduino IDE:** Ensure you have a recent version of the Arduino IDE installed.
2.  **Install ESP32 Board Support:** If you haven't already, add ESP32 board support via the Boards Manager.
3.  **Install Libraries:** Open the Arduino IDE and navigate to **Sketch -> Include Library -> Manage Libraries...**. Search for and install the following libraries:
    *   `IRremoteESP8266` (by crankyoldgit)
    *   `U8g2` (by olikraus)
4.  **Load the Sketch:** Open the `ESP32_Hybrid_IR_Remote.ino` file in the Arduino IDE.
5.  **Erase Flash (First Time Only):** For the very first upload, it is crucial to erase any old data from the ESP32's flash. Go to **Tools -> Erase All Flash Before Sketch Upload** and select **"Enabled"**.
6.  **Upload:** Select your ESP32 board and port, and click the Upload button.
7.  **Disable Erase Flash:** After the first successful upload, return to **Tools -> Erase All Flash Before Sketch Upload** and set it to **"Disabled"** for all future uploads. This will ensure your saved codes are not erased every time you upload new firmware.

## **How to Use the Remote**

The device is controlled with three buttons: `UP`, `DOWN`, and `SELECT`.

### **Main Menu**

*   The main menu lists all available slots.
*   Use `UP`/`DOWN` to navigate the list.
*   Press `SELECT` on a highlighted slot to choose an action.

### **Controlling the "Smart AC" (Slot 1)**

1.  Navigate to `Slot 1 [Smart AC]` and press `SELECT`.
2.  A new menu appears with AC-specific commands (`Power ON`, `Power OFF`).
3.  Navigate to the desired command and press `SELECT` to transmit the signal.
4.  Select "Back" to return to the main menu.

### **Using the "Learn" Slots (Slots 2-4)**

*   **To Learn a New Code:**
    1.  Navigate to an `[EMPTY]` slot and press `SELECT`.
    2.  The device enters "Learn Mode". The screen will prompt you to point a remote at the IR receiver.
    3.  Press the desired button on your external remote.
    4.  The ESP32 will capture the signal and display a "SUCCESS!" message. The slot will now be marked as `[SAVED]`.

*   **To Use a Saved Code:**
    1.  Navigate to a `[SAVED]` slot and press `SELECT`.
    2.  A new menu appears with options to `Transmit Signal` or `Delete Signal`.
    3.  Select `Transmit Signal` to send the IR code.
    4.  Select `Delete Signal` to permanently erase the code from that slot.

## **License**

This project is licensed under the MIT License. See the `LICENSE` file for details.
