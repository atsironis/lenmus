//--------------------------------------------------------------------------------------
//    LenMus Phonascus: The teacher of music
//    Copyright (c) 2002-2007 Cecilio Salmeron
//
//    This program is free software; you can redistribute it and/or modify it under the 
//    terms of the GNU General Public License as published by the Free Software Foundation;
//    either version 2 of the License, or (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful, but WITHOUT ANY 
//    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A 
//    PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License along with this 
//    program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, 
//    Fifth Floor, Boston, MA  02110-1301, USA.
//
//    For any comment, suggestion or feature request, please contact the manager of 
//    the project at cecilios@users.sourceforge.net
//
//-------------------------------------------------------------------------------------

#ifndef __LM_BOXSLICEVSTAFF_H__        //to avoid nested includes
#define __LM_BOXSLICEVSTAFF_H__

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "BoxSliceVStaff.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vector"

#include "../score/defs.h"
#include "GMObject.h"

class lmVStaff;
class lmBoxSliceInstr;
class lmBoxSliceVStaff;

//
// Class lmBoxSliceVStaff represents a part of the VStaff of an instrument. 
//

class lmBoxSliceVStaff : public lmBox
{
public:
    lmBoxSliceVStaff(lmBoxSliceInstr* pParent, lmVStaff* pVStaff);
    ~lmBoxSliceVStaff();

    void AddShape(lmShape* pShape);

    void Render(lmPaper* pPaper, lmUPoint uPos, wxColour color=*wxBLACK);

	//positioning and bounds
    void SetFinalX(lmLUnits xPos);

    //implementation of virtual methods from base class
    wxString Dump();


private:
    void RenderMeasure(int nMeasure, lmPaper* pPaper, int nNumPage);

    lmBoxSliceInstr*    m_pSliceInstr;      //paren instrumet slice
    lmVStaff*           m_pVStaff;          //VStaff to which this slice belongs

	std::vector<lmShape*>	m_Shapes;		//list of contained shapes

};



#endif  // __LM_BOXSLICEVSTAFF_H__

