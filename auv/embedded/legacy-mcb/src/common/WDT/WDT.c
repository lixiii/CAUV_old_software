//-----------------------------------------------------------------------------
//
//      Project:        CAUV MCU - Motor PIC
//      Filename:       WDT.c
//      Author:         Simon Calcutt
//      Date:           01/07/2010
//      Version:        v1
//
//-----------------------------------------------------------------------------
//  This is the code which controls the WDT so the device will reset if the
//  program crashes
//-----------------------------------------------------------------------------
//  Version notes:
//      v1 - This is the first version
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Include files
//-----------------------------------------------------------------------------
#include <p33fxxxx.h>
#include <reset.h>
#include "WDT.h"

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

void inline resetWDTCounter(void) {
//#asm
    ClrWdt();
//#endasm
}

