/**
 * @author  phi@freimann.eu + gemini
 * @date    2026_11_22
 * @license GPL (www.gnu.org/licenses/gpl-3.0.de.html)
 * Encapsulates DCF77 reception and DS3231 RTC (via RTClib) using three states:
 * STATE_UNREADY, STATE_RTC_FRESH, STATE_RTC_EXPIRED.
 */

#ifndef MY_DCF_RTC_H
#define MY_DCF_RTC_H

#include <Arduino.h>
#include <TimeLib.h>
#include <Wire.h>
#include <RTClib.h>

#include <DCF77.h>
#include <My_Debug.h> // for DEBUG_METHOD define

/**
 * DCF_MAX_VALID_SECS defines how many seconds after a valid DCF sync
 * the clock stays in STATE_RTC_FRESH before switching to STATE_RTC_EXPIRED
 * (keeping the time displayed while enabling DCF blink indication).
 */
#ifdef DEBUG_METHOD
	#define DCF_MAX_VALID_SECS (2UL * 60UL * 60UL)           // 2 hours
#else
	#define DCF_MAX_VALID_SECS (14UL * 24UL * 60UL * 60UL) // 2 weeks
#endif

/**
 * Interval in milliseconds to sync the RTC time into Arduino TimeLib.
 * Important because getHour() and getMinute() read from Arduino TimeLib.
 */
#define SAVE_RTC_TO_ARDUINO_TIME_EVERY_X_MS 60000UL // one minute

#define MIN_YEAR 2026

class MyDCFRTC {
public:

	enum State {
		STATE_UNREADY,     // No valid time: shift registers off, DCF blinks via transistor
		STATE_RTC_FRESH,   // Recently synced via DCF: time on, TIME_VISIBLE_PIN HIGH (no blinking)
		STATE_RTC_EXPIRED  // Last DCF sync older than DCF_MAX_VALID_SECS: time on + DCF blinking
	};

	// Constructor with pin connected to the DCF77 signal
	MyDCFRTC(int dcfPin);

	void update();

	// True when a time should be displayed (STATE_RTC_FRESH or STATE_RTC_EXPIRED)
	bool hasValidTime();

	// True only in STATE_RTC_FRESH ("ES IST" steadily on, no DCF blinking)
	bool isRtcFresh();

	// Returns 1..12
	int getHour();

	// Returns 0..59
	int getMinute();


private:
	DCF77 *       _dcf       ;  // Pointer to DCF77 object
	RTC_DS3231    _rtc       ;  // DS3231 RTC object
	bool          _rtcPresent;  // True if DS3231 initialized successfully: The chip is present and reacts to rtc.begin()
	State         _state     ;  // Current state (see State enum)

	/**
	 * Unix timestamp until which the last received DCF time is considered fresh.
	 * 0 means no DCF signal has been received yet.
	 */
	uint64_t      _dcfTimeValidUntil;

	/**
	 * millis() timestamp at which the next RTC-to-Arduino time sync is due.
	 * Every "some" milliseconds the RTC time is stored in the Arduino.
	 * The following variable says, when it has been saved the last time
	 * [in arduino.mills()].
	 * see: SAVE_RTC_TO_ARDUINO_TIME_EVERY_X_MS
	 * 0 means: arduino time is not valid!
	 */
	unsigned long _arduinoTimeValidUntil;


	/**
	 * Return current Unix time from the RTC (if initialized and valid),
	 * otherwise fall back to Arduino TimeLib now().
	 */
	uint64_t currentUnixTime();


	/**
	 * Update _state based on the current Unix timestamp.
	 */
	void setState(uint64_t nowUnix);


	/**
	 * Handle a newly received valid DCF timestamp:
	 * - Update Arduino TimeLib time
	 * - Update DS3231 RTC time
	 * - Refresh _dcfTimeValidUntil and _arduinoTimeValidUntil
	 * - Recalculate state
	 */
	void handleCorrectDCFSignalAndSetState(time_t correctDCFTime);

};

#endif
