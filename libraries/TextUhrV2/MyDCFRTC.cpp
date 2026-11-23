/**
 * @author  phi@freimann.eu
 * @date    2026_11_23
 * @license GPL (www.gnu.org/licenses/gpl-3.0.de.html)
 * See MyDCFRTC.h for details.
 */

#include "MyDCFRTC.h"

// Constructor (typically DCF signal on pin 2)
MyDCFRTC::MyDCFRTC(int dcfPin) {
	// Map pin number to interrupt number (typically interrupt 0 on pin 2)
	_dcf                   = new DCF77(dcfPin, digitalPinToInterrupt(dcfPin));
	_rtcPresent                 = false;
	_state                 = STATE_UNREADY;
	_dcfTimeValidUntil     = 0;
	_arduinoTimeValidUntil = 0;

	// Start I2C communication (A4 = SDA, A5 = SCL on Arduino Nano)
	Wire.begin();
	if (_rtc.begin()) {
		_rtcPresent = true;
	} else {
		#ifdef DEBUG_METHOD
		Serial.println("WARNING: DS3231 RTC not found!");
		#endif
	}

	_dcf->Start();
}


uint64_t MyDCFRTC::currentUnixTime() {
	if (_rtcPresent && _dcfTimeValidUntil > 0) {
		DateTime rtcNow = _rtc.now();
		if (rtcNow.year() >= MIN_YEAR) {
			return (uint64_t)rtcNow.unixtime();
		}
	}
	return (uint64_t)now();
}


void MyDCFRTC::setState(uint64_t nowUnix) {
	#ifdef DEBUG_METHOD
		State prevState = _state;
	#endif

	// in dubio pro reo:
	_state = STATE_RTC_FRESH;
	// check states and save to "_state"
	if (0 == _dcfTimeValidUntil) {
		// Never received a DCF signal yet
		_state = STATE_UNREADY;
	} else if(nowUnix > _dcfTimeValidUntil) {
		// DCF validity window has elapsed
		_state = STATE_RTC_EXPIRED;
	}

	#ifdef DEBUG_METHOD
	if (_state != prevState) {
		Serial.print("State -> ");
		if (_state == STATE_UNREADY)        Serial.println("STATE_UNREADY (no time, DCF blinking)");
		else if (_state == STATE_RTC_FRESH) Serial.println("STATE_RTC_FRESH (time on, ES IST steady)");
		else                                Serial.println("STATE_RTC_EXPIRED (time on + DCF blinking)");
	}
	#endif
}


void MyDCFRTC::handleCorrectDCFSignalAndSetState(time_t correctDCFTime) {
	uint64_t dcfUnix = (uint64_t)correctDCFTime;
	setTime(correctDCFTime); // Sync Arduino TimeLib clock with DCF time

	if (_rtcPresent) {
		_rtc.adjust(DateTime((uint32_t)dcfUnix));
	}

	_dcfTimeValidUntil     = dcfUnix + DCF_MAX_VALID_SECS;
	_arduinoTimeValidUntil = millis() + SAVE_RTC_TO_ARDUINO_TIME_EVERY_X_MS;

	#ifdef DEBUG_METHOD
	Serial.println("Valid DCF signal received -> RTC updated, state STATE_RTC_FRESH.");
	#endif

	setState(dcfUnix);
}


void MyDCFRTC::update() {
	time_t dcfTime = _dcf->getTime();

	// 1. Handle newly received valid DCF77 time
	if (0 != dcfTime) {
		handleCorrectDCFSignalAndSetState(dcfTime);
		return;
	}

	// 2. Periodically pull current time from DS3231 RTC into Arduino TimeLib
	if (_rtcPresent && (_dcfTimeValidUntil > 0) && ((long)(millis() - _arduinoTimeValidUntil) >= 0)) {
		_arduinoTimeValidUntil = millis() + SAVE_RTC_TO_ARDUINO_TIME_EVERY_X_MS;
		DateTime rtcNow = _rtc.now();
		if (rtcNow.year() >= MIN_YEAR) {
			setTime((time_t)rtcNow.unixtime());
		}
	}

	// 3. Re-evaluate state using current Unix time
	setState(currentUnixTime());
}

bool MyDCFRTC::hasValidTime() {
	return (_state == STATE_RTC_FRESH) || (_state == STATE_RTC_EXPIRED);
}


bool MyDCFRTC::isRtcFresh() {
	return (_state == STATE_RTC_FRESH);
}


int MyDCFRTC::getHour() {
	int h = hour() % 12;
	return (0 == h) ? 12 : h;
}


int MyDCFRTC::getMinute() {
	return minute();
}
