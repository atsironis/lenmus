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

#ifndef __LM_TEXT_H__        //to avoid nested includes
#define __LM_TEXT_H__

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "Text.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "Score.h"


class lmBasicText
{
public:
    lmBasicText(wxString sText, wxString sLanguage,
                   lmLocation* pPos, lmFontInfo oFontData);

    ~lmBasicText() {}

    void SetText(wxString sText) { m_sText = sText; }
    void SetLanguage(wxString sLanguage) { m_sLanguage = sLanguage; }


protected:
    wxString    m_sText;
    wxString    m_sLanguage;

    // position
    lmLocation  m_tPos;

    // font
    wxString    m_sFontName;
    int         m_nFontSize;
    bool        m_fBold;
    bool        m_fItalic;

};

class lmShapeText;
class lmBox;


class lmScoreText :  public lmAuxObj
{
public:
    lmScoreText(wxString sTitle, lmEAlignment nAlign, lmLocation tPos, lmFontInfo tFont,
                bool fTitle=false, wxColour colorC = *wxBLACK);

    ~lmScoreText() {}

    //implementation of virtual methods defined in abstract base class
    lmLUnits LayoutObject(lmBox* pBox, lmPaper* pPaper, lmUPoint uPos, wxColour colorC);
	wxFont* GetSuitableFont(lmPaper* pPaper);
	lmUPoint ComputeBestLocation(lmUPoint& uOrg, lmPaper* pPaper);

    //implementation of virtual methods from base class
    lmEAuxObjType GetAuxObjType() { return eAXOT_Text; }


    //    debugging
    wxString Dump();
    wxString SourceLDP(int nIndent);
    wxString SourceXML(int nIndent);

    //specific method of this object
    void SetText(wxString text) { m_sText = text; }
	wxString GetText() {return m_sText; }
    lmEAlignment GetAlignment() { return m_nAlignment; }
    lmLocation GetLocation() { return m_tPos; }
	lmShapeText* CreateShape(lmPaper* pPaper, lmUPoint uPos);

private:
    wxString        m_sText;

    lmEAlignment    m_nAlignment;
	wxColour		m_color;

    wxString        m_sFontName;
    int             m_nFontSize;
    bool            m_fBold;
    bool            m_fItalic;
    bool            m_fIsTitle;     //to identify titles

};

//global functions defined in this module
extern lmLUnits PointsToLUnits(int nPoints);
extern int LUnitsToPoints(lmLUnits uUnits);

#endif    // __LM_TEXT_H__

