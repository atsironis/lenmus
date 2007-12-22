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

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "StatusBar.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

// for all others, include the necessary headers
#ifndef WX_PRECOMP
    #include "wx/statusbr.h"
#endif

#if !wxUSE_STATUSBAR
    #error "You need wxWidgets compiled with wxUSE_STATUSBAR to 1 in setup.h!"
#endif // wxUSE_STATUSBAR


#include "StatusBar.h"


// ----------------------------------------------------------------------------
// lmStatusBar implementation
// ----------------------------------------------------------------------------

lmStatusBar::lmStatusBar(wxFrame* pFrame, lmEStatusBarLayout nType, wxWindowID id)
           : wxStatusBar(pFrame, wxID_ANY, wxST_SIZEGRIP)
{
    m_pFrame = pFrame;
    m_nNumFields = 2;
    int ch = GetCharWidth();
    const int widths[] = {-1, 20*ch};
    SetFieldsCount(m_nNumFields);
    SetStatusWidths(m_nNumFields, widths);
}

lmStatusBar::~lmStatusBar()
{
}

void lmStatusBar::SetMsgText(const wxString& sText)
{
    SetStatusText(sText, 0);
}

