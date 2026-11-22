/**
 * @author  phi@freimann.eu
 * @date    2016-01-29
 * @license GPL (www.gnu.org/licenses/gpl-3.0.de.html)
 *
 * Toggle compilation of debug info.
 * Just comment/uncomment the one and only #define line in this sketch:
 * //#define DEBUG_METHOD
 * (which is on line number 16)
 */

#ifndef MY_DEBUG_H
#define MY_DEBUG_H

// debug only: if next line is omitted (or commented), no Serial.outs are done.
// otherwise all debug is done (Serial.out)
#define DEBUG_METHOD

#endif
