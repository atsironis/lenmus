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

#ifndef __LM_HELPCONTROLLER_H__
#define __LM_HELPCONTROLLER_H__

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "HelpController.cpp"
#endif

#include "wx/defs.h"
#include "wx/html/helpctrl.h"

class lmHelpController : public wxHtmlHelpController
{
public:
    lmHelpController(int style = wxHF_DEFAULT_STYLE) : wxHtmlHelpController(style) {}
    void OnQuit();

};

#endif // __LM_HELPCONTROLLER_H__
