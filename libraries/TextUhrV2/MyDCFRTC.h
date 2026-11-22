/**
 * @author  phi@gress.ly + gemini
 * @date    2026_11_22
 * @license GPL (www.gnu.org/licenses/gpl-3.0.de.html)
 * Kapselt DCF77-Empfang und DS3231-RTC (via RTClib) mit drei Zuständen:
 * STATE_UNREADY, STATE_RTC_FRESH, STATE_RTC_EXPIRED.
 */

#ifndef MY_DCF_RTC_H
#define MY_DCF_RTC_H

#include <Arduino.h>
#include <TimeLib.h>
#include <Wire.h>
#include <RTClib.h>

#include <DCF77.h>
#include <My_Debug.h>

/**
 * RTC_MAX_VALID_SECS gibt an, nach wie vielen Sekunden die
 * Uhr in den Original-Zustand (searching) zurückfallen soll.
 * Dies soll eigentlich alle 2 Wochen geschehen, es sei denn,
 * es wurde in der Zwischenzeit ein neues DCF Signal empfangen,
 * dann wird "_lastSyncUnix" neu beschrieben und wir haben wieder "ewig"
 * Zeit, bis die nächsten zwei Wochen vergangen sind.
 * Dies kommt eigentlich erst zum Tragen, wenn wirklich während zwei Wochen kein DCF Signal empfangen wurde.
 */
#ifdef DEBUG_METHOD
	#define RTC_MAX_VALID_SECS (2UL * 60UL * 60UL)           // 2 Stunden
#else
	#define RTC_MAX_VALID_SECS (14UL * 24UL * 60UL * 60UL) // 2 Wochen
#endif

class MyDCFRTC {
public:



	enum State {
		STATE_UNREADY,     // Keine gültige Zeit: Shift-Register aus, DCF blinkt via Transistor
		STATE_RTC_FRESH,   // RTC aktuell gestellt: Zeit an, TIME_VISIBLE_PIN HIGH (kein Blinken)
		STATE_RTC_EXPIRED  // Letzter DCF-Sync älter als RTC_MAX_VALID_SECS: Zeit an + Blinken
	};

	// constructor with PIN to listen to DCF77 signal
	MyDCFRTC(int dcfPin);
	
	void update();

	// True, sobald eine Zeit angezeigt werden soll (STATE_RTC_FRESH oder STATE_RTC_EXPIRED)
	bool hasValidTime();

	// True nur bei STATE_RTC_FRESH ("ES IST" dauerhaft an, kein DCF-Blinken)
	bool isRtcFresh();

	State getState();

	// returns 1..12
	int getHour();
	// returns 0..59
	int getMinute();



private:
	DCF77 *       _dcf;
	RTC_DS3231    _rtc;
	bool          _rtcOk;
	State         _state;
	uint64_t      _lastSyncUnix; // Zeitpunkt der letzten DCF-Synchronisation (0 = noch nie)
	unsigned long _lastRtcReadMs;

	uint64_t currentUnixTime();
	void evaluateState(uint64_t nowUnix);
};

#endif
