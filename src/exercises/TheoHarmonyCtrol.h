//--------------------------------------------------------------------------------------
//    LenMus Phonascus: The teacher of music
//    Copyright (c) 2002-2009 LenMus project
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

#ifndef __LM_THEOHARMONYCTROL_H__        //to avoid nested includes
#define __LM_THEOHARMONYCTROL_H__

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "TheoHarmonyCtrol.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "HarmonyConstrains.h"
#include "../score/Score.h"
#include "ExerciseCtrol.h"



class lmTheoHarmonyCtrol : public lmFullEditorExercise   
{
   DECLARE_DYNAMIC_CLASS(lmTheoHarmonyCtrol)

public:

    // constructor and destructor    
    lmTheoHarmonyCtrol(wxWindow* parent, wxWindowID id,
               lmHarmonyConstrains* pConstrains, wxSize nDisplaySize, 
               const wxPoint& pos = wxDefaultPosition, 
               const wxSize& size = wxDefaultSize, int style = 0);

    ~lmTheoHarmonyCtrol();

    //event handlers
    void OnEndOfPlay(lmEndOfPlayEvent& WXUNUSED(event));

    //implementation of virtual methods from base class
    void SetNewProblem();    
    wxDialog* GetSettingsDlg();
    void InitializeStrings();
    void OnSettingsChanged();


private:


        // member variables

    lmHarmonyConstrains*    m_pConstrains;      //constraints for the exercise
    lmEKeySignatures        m_nKey;             //key to use for scores

    DECLARE_EVENT_TABLE()
};



#endif  // __LM_THEOHARMONYCTROL_H__