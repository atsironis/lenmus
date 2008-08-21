//--------------------------------------------------------------------------------------
//    LenMus Phonascus: The teacher of music
//    Copyright (c) 2002-2008 Cecilio Salmeron
//
//    This program is free software; you can redistribute it and/or modify it under the
//    terms of the GNU General Public License as published by the Free Software Foundation,
//    either version 3 of the License, or (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful, but WITHOUT ANY
//    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
//    PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License along with this
//    program. If not, see <http://www.gnu.org/licenses/>.

//
//    For any comment, suggestion or feature request, please contact the manager of
//    the project at cecilios@users.sourceforge.net
//
//-------------------------------------------------------------------------------------

#ifndef __LM_GENERATORS_H__        //to avoid nested includes
#define __LM_GENERATORS_H__

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "Generators.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "Constrains.h"

class lmRandomGenerator
{
public:
    lmRandomGenerator();
    ~lmRandomGenerator() {}

    //random numbers
    static int RandomNumber(int nMin, int nMax);
    static bool FlipCoin();

    // clefs
    static lmEClefType GenerateClef(lmClefConstrain* pValidClefs);

    // key signature
    static lmEKeySignatures GenerateKey(lmKeyConstrains* pValidKeys);
    static lmEKeySignatures RandomKeySignature();

    //time signature
    static lmETimeSignature GenerateTimeSign(lmTimeSignConstrains* pValidTimeSignatures);
    static lmETimeSignature RandomTimeSignature();

    //notes
    static lmDPitch GenerateRandomDPitch(int nMinLine, int nRange, bool fRests, lmEClefType nClef);
    static wxString GenerateRandomRootNote(lmEClefType nClef, lmEKeySignatures nKey, bool fAllowAccidentals);

};


#endif  // __LM_GENERATORS_H__
