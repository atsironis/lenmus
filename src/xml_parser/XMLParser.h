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

#ifndef __XMLPARSER_H__        //to avoid nested includes
#define __XMLPARSER_H__

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "XmlParser.cpp"
#endif

#include "wx/txtstrm.h"
#include "wx/wfstream.h"
#include "wx/xml/xml.h"          // to use wxXmlDocument


class lmXmlParser
{
public:
    lmXmlParser();
    ~lmXmlParser();

    // auxiliary XML methods
    wxXmlNode* GetFirstChild(wxXmlNode* pNode);
    wxXmlNode* GetNextSibling(wxXmlNode* pNode);
    wxString GetAttribute(wxXmlNode* pNode, wxString sName, wxString sDefault = _T(""));
    bool GetYesNoAttribute(wxXmlNode* pNode, wxString sName, bool fDefault=true);
    wxString GetText(wxXmlNode* pElement);

    void DumpXMLTree(wxXmlNode *pRoot);
    void ParseError(const wxChar* szFormat, ...);
    void TagError(const wxString sElement, const wxString sTagName, wxXmlNode* pElement = NULL);

private:

    int        m_nErrors;        // number of parsing errors


};

#endif    // __XMLPARSER_H__
