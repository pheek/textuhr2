/**
 * @author  phi@freimann.eu
 * @date    V0 2016-01-29 (simpleShifter)
 * @date    V1 2026-10-25 (simpleSouthShifter)
 * 
 * @license GPL (www.gnu.org/licenses/gpl-3.0.de.html)
 *
 * Simple shift-register implementation for 74HC595 or similar.
 * ------------------------------------------------------------
 * Feature: Only (logical) Bits on "south" of the shift registers are counted.
 * 
 * eg. three registers (below: logical bits "l-bit" and physical bits "p-bit")
 *
 *p-bit      23                       15                      7
 *           QA                       QA                     QA
 *        --------------------    --------------------    --------------------
 *        QB QC QD QE QF QG QH    QB QC QD QE QF QG QH    QB QC QD QE QF QG QH
 *l-bit:   0  1  2  3  4  5  6     7  8  9 10 11 12 13    14 15 16 17 18 19 20
 *p-bit:  22 21 20 19 18 17 16    14 13 12 11 10  9  8     6  5  4  3  2  1  0 
 *
 * This class does not contain a remove or clear bit function.
 * It only contains a  * clearData() to clear all data,
 * an onBit() to set a specific bit and  at last a writeData()
 * function, which writes all the data.
 * That's why you have to clear all Data first 'clearData()'
 * and set only the bits in your shift-registers that should be
 * "on" 'onBit()'. When all bits are set, use
 * writeData() to write all stored bits to the SOUTH Pins (1..7) of
 * a 74HC595-Shift-Regrister.
 */
#ifndef SIMPLE_SOUTH_SHIFTER_H
#define SIMPLE_SOUTH_SHIFTER_H

#include <Arduino.h>
#include <My_Debug.h>

class SimpleSouthShifter {

public:
	/**
	 * numRegs  Number of used shift registers. (use eg. 3 if you need 24 Pins)
	 * pinData  is the data-Pin (you set 0 or 1 to this before
	 *          calling the clock).
	 * pinSRCLK registers the data for the next latch.
	 *          nothing happens to the pins befor the latch (RCLK)
	 * pinRCLK  is the latch. If this is set to high, all data
	 *          will be sent to the outputs.
	 */
	SimpleSouthShifter(int numRegs, int pinData, int pinSRCLK, int pinRCLK);


	/**
	 * Clear all bits.
	 * Attention: This does not clear write out to the pins:
	 * After calling clearData you shoud write some new data (onBit())
	 * and at last shift them out using writeData().
	 *
	 * example:
	 * clearData();
	 * onBit( 7);
	 * onBit(11);
	 * onBit(12);
	 * writeData(); // turns on PIN 7, 11 and 12
	 */
	void clearData();


	/**
	 * Enables a bit.
	 * Call writeData() to effectively shift the data out.
	 */
	void onBit(int bitNr);


	/**
	 * Write all stored bits to the shift registers.
	 */
	void writeData();

	/**
	 * Clear all Bits
	 */
	void writeAllZeros();


private:
	// Number of shift registers used.
	int _numRegs;

	// A dynamic use of the number of shift registers
	// uses a dynamic array size:
	bool* _logicalData  = NULL;
	bool* _physicalData = NULL;
	int _numLogicalPins ; // 7 * _numRegs (QA on "north" is not used.)
	int _numPhysicalPins; // 8 * _numRegs

	unsigned int _pinData;
	unsigned int _pinSRCLK;
	unsigned int _pinRCLK;

}; // end class SimpleSouthShifter

#endif
