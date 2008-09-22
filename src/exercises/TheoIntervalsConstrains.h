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

#ifndef __LM_THEOINTERVALSCONSTRAINS_H__        //to avoid nested includes
#define __LM_THEOINTERVALSCONSTRAINS_H__

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "TheoIntervalsConstrains.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "../score/Score.h"
#include "Constrains.h"


enum EProblemTheoIntervals
{
    ePT_DeduceInterval = 0,         //WARNING: the enum values are used as indexes
    ePT_BuildInterval,              // in DlgCfgTheoIntervals. Do not alter
    ePT_Both                        // neither order nor values. Must start in 0
};


class lmTheoIntervalsConstrains: public lmExerciseOptions
{
public:
    lmTheoIntervalsConstrains(wxString sSection);
    ~lmTheoIntervalsConstrains() {}

    bool IsValidClef(lmEClefType nClef) { return m_oClefs.IsValid(nClef); }
    void SetClef(lmEClefType nClef, bool fValid) { m_oClefs.SetValid(nClef, fValid); }

    EProblemTheoIntervals GetProblemType() { return m_nProblemType; }
    void SetProblemType(EProblemTheoIntervals nType) { m_nProblemType = nType; }

    bool GetAccidentals() { return m_fAccidentals; }
    void SetAccidentals(bool fValue) { m_fAccidentals = fValue; }

    bool GetDoubleAccidentals() { return m_fDoubleAccidentals; }
    void SetDoubleAccidentals(bool fValue) { m_fDoubleAccidentals = fValue; }

    lmClefConstrain* GetClefConstrains() { return &m_oClefs; }

    void SetSection(wxString sSection) {
                m_sSection = sSection;
                LoadSettings();
            }

    void SaveSettings();


private:
    void LoadSettings();

    lmClefConstrain         m_oClefs;
    EProblemTheoIntervals   m_nProblemType;
    bool                    m_fAccidentals;             //allow accidentals
    bool                    m_fDoubleAccidentals;       //allow double accidentals

};


#endif  // __LM_THEOINTERVALSCONSTRAINS_H__
