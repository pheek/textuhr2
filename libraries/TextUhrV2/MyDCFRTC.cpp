/**
 * @author  phi@gress.ly
 * @date    2026_10_19
 * @license GPL (www.gnu.org/licenses/gpl-3.0.de.html)
 * Siehe MyDCF.h für Details.
 */

#include "MyDCFRTC.h"

MyDCFRTC::MyDCFRTC(int dcfPin) {
	// zu Pin 2 gehört z. B. interrupt 0
	_dcf           = new DCF77(dcfPin, digitalPinToInterrupt(dcfPin));
	_rtcOk         = false;
	_state         = STATE_UNREADY;
	_lastSyncUnix  = 0;
	_lastRtcReadMs = 0;

	// starte serielle Kommunikation Wire A4 und A5 auf dem Arduino
	Wire.begin();
	if (_rtc.begin()) {
		_rtcOk = true;
		/*		DateTime rtcNow = _rtc.now();
		// Beim Betrieb ohne Batterie ist lostPower() nach dem Boot meist true.
		// Falls dennoch eine plausible Zeit anliegt, übernehmen wir sie als EXPIRED,
		// damit im Zweifel nachsynchronisiert wird.
		if (!_rtc.lostPower() && rtcNow.year() >= 2025) {
			uint64_t u = (uint64_t)rtcNow.unixtime();
			setTime((time_t)u);
			_lastSyncUnix = u;
			_state = STATE_RTC_EXPIRED;
			} */
	} else {
		#ifdef DEBUG_METHOD
		Serial.println("WARNUNG: DS3231 nicht gefunden!");
		#endif
	}

	_dcf->Start();
}

uint64_t MyDCFRTC::currentUnixTime() {
	if (_rtcOk && _lastSyncUnix > 0) {
		DateTime rtcNow = _rtc.now();
		if (rtcNow.year() >= 2025) {
			return (uint64_t)rtcNow.unixtime();
		}
	}
	return (uint64_t)now();
}

void MyDCFRTC::evaluateState(uint64_t nowUnix) {
	State prevState = _state;

	if (_lastSyncUnix == 0 || nowUnix < _lastSyncUnix) {
		_state = STATE_UNREADY;
	} else if ((nowUnix - _lastSyncUnix) <= RTC_MAX_VALID_SECS) {
		_state = STATE_RTC_FRESH;
	} else {
		_state = STATE_RTC_EXPIRED;
	}

	#ifdef DEBUG_METHOD
	if (_state != prevState) {
		Serial.print("State -> ");
		if (_state == STATE_UNREADY)          Serial.println("STATE_UNREADY (keine Zeit, Blinken)");
		else if (_state == STATE_RTC_FRESH)   Serial.println("STATE_RTC_FRESH (Zeit an, ES IST stabil)");
		else                                  Serial.println("STATE_RTC_EXPIRED (Zeit an + Blinken)");
	}
	#endif
}

void MyDCFRTC::update() {
	time_t dcfTime = _dcf->getTime();

	// 1. Neues gültiges DCF-Signal -> RTC sofort stellen & Sync-Zeitpunkt merken
	if (0 != dcfTime) {
		uint64_t dcfUnix = (uint64_t)dcfTime;
		setTime(dcfTime);

		if (_rtcOk) {
			_rtc.adjust(DateTime((uint32_t)dcfUnix));
		}

		_lastSyncUnix  = dcfUnix;
		_lastRtcReadMs = millis();

		#ifdef DEBUG_METHOD
		Serial.println("Neues DCF-Signal -> RTC gestellt, Zustand STATE_RTC_FRESH.");
		#endif

		evaluateState(dcfUnix);
		return;
	}

	// 2. Regelmäßig (alle 2 s) Zeit aus DS3231 in TimeLib nachziehen
	if (_rtcOk && _lastSyncUnix > 0 && (millis() - _lastRtcReadMs >= 2000UL || _lastRtcReadMs == 0)) {
		_lastRtcReadMs = millis();
		DateTime rtcNow = _rtc.now();
		if (rtcNow.year() >= 2025) {
			setTime((time_t)rtcNow.unixtime());
		}
	}

	// 3. Zustand prüfen
	evaluateState(currentUnixTime());
}

bool MyDCFRTC::hasValidTime() {
	return (_state == STATE_RTC_FRESH) || (_state == STATE_RTC_EXPIRED);
}

bool MyDCFRTC::isRtcFresh() {
	return (_state == STATE_RTC_FRESH);
}

MyDCFRTC::State MyDCFRTC::getState() {
	return _state;
}

int MyDCFRTC::getHour() {
	int h = hour() % 12;
	return (0 == h) ? 12 : h;
}

int MyDCFRTC::getMinute() {
	return minute();
}
