/**
 * @author  phi@freimann.eu
 * @date    2026-11-22
 * @version 2.2
 * @doku    see simpleSouthShifter.h
 * @license GPL (www.gnu.org/licenses/gpl-3.0.de.html)
 * --------------------
 */
#include "SimpleSouthShifter.h"
#include <Arduino.h>


/**
 * numRegs  Number of physical shift registers.
 * pinData  Data: is the data-Pin (you set 0 or 1 to this before
 *          calling the clock).
 * pinSRCLK Clock: registers the data for the next latch.
 *          nothing happens to the pins befor the latch (RCLK)
 * pinRCLK  Latch: If this is set to high, all data
 *          will be sent to the outputs.
 */
SimpleSouthShifter::SimpleSouthShifter(int numRegs, int pinData, int pinSRCLK, int pinRCLK) {
	_numRegs         = numRegs     ;
	_numLogicalPins  = 7 * _numRegs;
	_numPhysicalPins = 8 * _numRegs;
	_pinData         = pinData     ;
	_pinSRCLK        = pinSRCLK    ;
	_pinRCLK         = pinRCLK     ;
	pinMode(_pinData,  OUTPUT);
	pinMode(_pinSRCLK, OUTPUT);
	pinMode(_pinRCLK,  OUTPUT);
	// alloc memory for the bits (bool). "null" if not enough memory.
	_logicalData  = (bool *) malloc(_numLogicalPins  * (sizeof (bool)));
	_physicalData = (bool *) malloc(_numPhysicalPins * (sizeof (bool)));
	clearData();
}


/**
 * Clear all bits.
 * Attention: This does not clear write out to the pins:
 * After calling clearData you shoud write some new data (ssh_onBit())
 * and at last shift them out using ssh_writeData().
 */
void SimpleSouthShifter::clearData() {
	if(NULL == _logicalData) { return; }
	for(int i = _numLogicalPins - 1; i >= 0; i = i - 1) {
		_logicalData[i] = false;
	}
}


/**
 * Enables a logical bit in a _logicalData - Array.
 * Call writeData() to effectively shift the data out to the physical Pins
 * on the shift registers..
 */
void SimpleSouthShifter::onBit(int bitNr) {
	if(NULL == _logicalData) { return; }
	if(bitNr >= 0 && bitNr < _numLogicalPins) {
		_logicalData[bitNr] = true;
	}
}


/**
 * Schreibe alle Daten in die Shift-Register.
 * Sobald alle Daten geschrieben sind, weise die Register an,
 * die Daten auch anzuzeigen (SHIFT_RCLK_PIN).
 */
void SimpleSouthShifter::writeData() {
	// assert, if malloc failed
	if(NULL == _logicalData)  { return; }
	if(NULL == _physicalData) { return; }

	// omit each bit on the "north" (Each QA-bit)
	int logicalIndex = _numLogicalPins - 1;
	// i = physical index
	for(int i = 0; i < _numPhysicalPins; i++) {
		// every 8th physical bit (7, 15, 23) is omitted
		if(7 == i % 8) {
			_physicalData[i] = false;
		} else {
			_physicalData[i] = _logicalData[logicalIndex--];
		}
	}


	digitalWrite(_pinRCLK, LOW);
	int val;

	// clock delay: Should be at least 4-5 micro-seconds.
	//              Arduino is slower anyway, so these delays are not necessary.
	//delay(1); // not necessary: Arduino is too slow
	for(int i = 0; i < _numPhysicalPins; i = i + 1) {
		digitalWrite(_pinSRCLK, LOW);
		//delay(1);
		val = _physicalData[i];
		digitalWrite(_pinData, val ? HIGH : LOW);
		//delay(1);
		digitalWrite(_pinSRCLK, HIGH);
	}
	//delay(1);
	digitalWrite(_pinRCLK, HIGH); // latch
}


/**
 * Helper method to clear all data and write them LOW to all outputs.
 */
void SimpleSouthShifter::writeAllZeros() {
	clearData();
	writeData();
}
