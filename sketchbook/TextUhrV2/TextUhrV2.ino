/**
 * @author  phi@freimann.eu + gemini
 * @date    2026_11_22
 * @version 2.2
 * @license GPL (www.gnu.org/licenses/gpl-3.0.de.html)
 */

// DEBUG ONLY: Provides define "DEBUG_METHOD"
#include "My_Debug.h"

// get time or info, that the time is not ready
#include "MyDCFRTC.h"

// Show the time on the output-shift registers
#include "SimpleSouthShifter.h"

// input of the DCF-signal
#define DCF_INPUT_PIN 2

// TIME_VISIBLE_PIN:
// HIGH -> "ES IST" is permanently on (time is ok)
// LOW  -> "ES IST" blinks with the DCF-raw signal.
#define TIME_VISIBLE_PIN 3

// pointer to myDCFRTC-Library-object
MyDCFRTC *myDCFRTC;

// Nur debug: Sodass die Zeit nicht zu oft, aber auch nicht
// zu selten in der arduino-Konsole erscheint.
// Dies wird wieder entfernt, sobald der Code stabil läuft.
#ifdef DEBUG_METHOD
unsigned long DEBUG_LAST_PRINT_TIME = 0;
#endif


// Pins for the shift registers (input):
// Arduino Output Data:
#define SHIFT_DATA_PIN      6   // (weiss  to 1. shift reg pin 14 SER)
// Arduino Output Clock:
#define SHIFT_SRCLK_PIN     7   // (yellow to 1. shift reg pin 11. "clock")
// Arduino Output Latch:
#define SHIFT_RCLK_PIN      8   // (braun  to 1. shift reg pin 12 "latch")

// How many shift registers are required (7 Bit pro Register!):
#define NUM_SHIFT_REGISTERS 3

// initialize the shift registers
SimpleSouthShifter simpleSouthShifter(NUM_SHIFT_REGISTERS, SHIFT_DATA_PIN, SHIFT_SRCLK_PIN, SHIFT_RCLK_PIN);


// ZEIT Anzeige PINS auf den Shift Registern:
#define LED_MINUTE_ZEHN     0
#define LED_MINUTE_VIERTEL  1
#define LED_MINUTE_ZWANZIG  2
#define LED_PUNKT           3

#define LED_MINUTE_FUENF    4
#define LED_VOR             5
#define LED_NACH            6
#define LED_HALB            7

// STUNDEN
#define LED_STUNDE_EINS     8
/* (the following are calculated, so the variables are not used)
#define LED_STUNDE_ZWEI     9
#define LED_STUNDE_DREI    10
#define LED_STUNDE_VIER    11
#define LED_STUNDE_FUENF   12
#define LED_STUNDE_SECHS   13
#define LED_STUNDE_SIEBEN  14
#define LED_STUNDE_ACHT    15
#define LED_STUNDE_NEUN    16
#define LED_STUNDE_ZEHN    17
#define LED_STUNDE_ELF     18
#define LED_STUNDE_ZWOELF  19
*/


// init arduino
void setup() {
	#ifdef DEBUG_METHOD
		Serial.begin(9600);
	#endif

	myDCFRTC = new MyDCFRTC(DCF_INPUT_PIN);
	pinMode(TIME_VISIBLE_PIN, OUTPUT);
}


#ifdef DEBUG_METHOD
/**
 * Print time at arduino serial port only for debug purpose.
 */
void debugPrintTime(int h, int m) {
	Serial.print("debugPrintTime(): ");
	Serial.print(h);
	Serial.print(":");
	if (m < 10) Serial.print("0");
	Serial.println(m);
}
#endif


// arduinos main loop:
void loop() {
	myDCFRTC->update();

	// 1. "ES IST" (IT IS) only lights permanently, when RTC has a fresh signal from the DCF
	if (myDCFRTC->isRtcFresh()) {
		digitalWrite(TIME_VISIBLE_PIN, HIGH);
	} else {
		digitalWrite(TIME_VISIBLE_PIN, LOW);
	}

	// 2. Show time (if RTC has a valid time, even if it is not fresh)
	if (myDCFRTC->hasValidTime()) {
		showTimeIfReady();

		#ifdef DEBUG_METHOD
		if (millis() - DEBUG_LAST_PRINT_TIME >= 60000UL || DEBUG_LAST_PRINT_TIME == 0) {
			debugPrintTime(myDCFRTC->getHour(), myDCFRTC->getMinute());
			DEBUG_LAST_PRINT_TIME = millis();
		}
		#endif

		delay(100); // cool down arduino a bit.
	} else { // no valid Time (even no unfresh)
		simpleSouthShifter.writeAllZeros();

		#ifdef DEBUG_METHOD
		if (millis() - DEBUG_LAST_PRINT_TIME >= 60000UL || DEBUG_LAST_PRINT_TIME == 0) {
			Serial.println("UNREADY: warte auf gültiges DCF-Signal...");
			DEBUG_LAST_PRINT_TIME = millis();
		}
		#endif
	}
}


/**
 * Evaluate pin number for actual hour.
 */
int hourPin(int hour) {
	return hour - 1 + LED_STUNDE_EINS; // all other hours are +1 on their pin numbers each
}

void ledON(int pin) {
	simpleSouthShifter.onBit(pin);
}


void showTimeIfReady() {
	simpleSouthShifter.clearData();
	if (myDCFRTC->hasValidTime()) {
		showHoursAndMinutes();
	}
	simpleSouthShifter.writeData();
}

/**
 * Register hours and minute-Pins to the shift registers.
 * The "writeData()" is done within the method "showTimeIfReady()".
 */
void showHoursAndMinutes() {
	int min  = myDCFRTC->getMinute();
	int hour = myDCFRTC->getHour();
	schalteStundenLEDs(hour, min);
	schalteMinutenLEDs(min);
}


/**
 * Die folgenden Methoden sind deutsch, denn die Uhrzeit wird ja
 * auch in deutsch angezeigt.
 */

void schalteStundenLEDs(int stunde, int min) {
	if (min > 23) {
		stunde = stunde + 1;
	}
	if (stunde > 12) {
		stunde = 1;
	}
	ledON(hourPin(stunde));
}


/**
 * Hier werden die LEDs auf den Shift-Registern angesprochen.
 * input:
 *   int min: Die aktuelle Minute der Systemzeit, die via DCF77 geholt wurde.
 * Da nur im ca. 5 Min takt geschaltet wird, reicht es , wenn der Parameter min "imZeitfenster" der
 * aktuellen 5 Minuten liegt.
 */
void schalteMinutenLEDs(int min) {
	if      (imZeitfenster(min, + 5)) { ledON(LED_MINUTE_FUENF  ); ledON(LED_NACH);                  }
	else if (imZeitfenster(min, +10)) { ledON(LED_MINUTE_ZEHN   ); ledON(LED_NACH);                  }
	else if (imZeitfenster(min, +15)) { ledON(LED_MINUTE_VIERTEL); ledON(LED_NACH);                  }
	else if (imZeitfenster(min, +20)) { ledON(LED_MINUTE_ZWANZIG); ledON(LED_NACH);                  }
	else if (imZeitfenster(min, +25)) { ledON(LED_MINUTE_FUENF  ); ledON(LED_VOR ); ledON(LED_HALB); }
	else if (imZeitfenster(min, +30)) {                                             ledON(LED_HALB); }
	else if (imZeitfenster(min, +35)) { ledON(LED_MINUTE_FUENF  ); ledON(LED_NACH); ledON(LED_HALB); }
	else if (imZeitfenster(min, +40)) { ledON(LED_MINUTE_ZWANZIG); ledON(LED_VOR );                  }
	else if (imZeitfenster(min, +45)) { ledON(LED_MINUTE_VIERTEL); ledON(LED_VOR );                  }
	else if (imZeitfenster(min, +50)) { ledON(LED_MINUTE_ZEHN   ); ledON(LED_VOR );                  }
	else if (imZeitfenster(min, +55)) { ledON(LED_MINUTE_FUENF  ); ledON(LED_VOR );                  }
	else    /* >=59 && <= 3 */     { ledON(LED_PUNKT         );                                   }
}

/**
 * 5 Minuten - Zeitfenster
 * in : min                    : Aktuelle Uhrzeit von 0..59
 *    : fuenfMinutenIntervall  : 5, 10, 15, 20, etc. bis 55
 * Bei 3 Minuten nach xy sage ich noch nicht es ist 'fünf nach'.
 * Bei 4 Minuten eigentlich auch noch nicht, aber bei 59 Sekunden
 * kann ich mich auf den Standpunkt stellen, dass die Zeit nun genug
 * nahe an den nächsten fünf Minuten ist. Daher wechsle ich
 * immer exakt bei 4, 9, 14, 19, ... Minuten auf den nächsten 'Fünfer'.
 *
 * output: true, genau dann wenn:
 *
 *    min im Zeitfenster der "fuenfMinuten Intervall (Genauigkeit)" liegt.
 *
 * Beispiel: Zeit Nahe an 45 -> true bei min=44, 45, 46, 47, und 48
 *                              false bei allen anderen Zahlen.
 */
boolean imZeitfenster(int min, int fuenfMinutenIntervall) {
	return min >= fuenfMinutenIntervall - 1 &&
	       min <= fuenfMinutenIntervall + 3;
}
