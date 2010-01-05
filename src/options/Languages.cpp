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

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "Languages.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/dir.h"
#include <wx/intl.h>
#include <wx/stattext.h>

#include <wx/hashmap.h>
WX_DECLARE_STRING_HASH_MAP(wxString, LangHash);

#include "Languages.h"
#include "../globals/Paths.h"
extern lmPaths* g_pPaths;

bool TranslationExists(wxArrayString& pathList, wxString code)
{
    wxArrayString results;
    wxString sPattern = code;

    if (sPattern == _T("")) return false;

    //DBG: To have a place to put a break
    if (sPattern == _T("es") ) {
        int kk;
        kk=23;
    }

    wxString sFileName;
    bool fFound;
    for(unsigned int i=0; i < pathList.GetCount(); i++) {
        wxString path = pathList[i];

        wxDir oDir(path);
        if (!oDir.IsOpened()) continue;    //directory does not exists!
        fFound = oDir.GetFirst(&sFileName, sPattern, wxDIR_FILES | wxDIR_DIRS);
        while(fFound) {
            //wxLogMessage(wxString::Format(_T("[Languages::TranslationExists] code=%s, sFileName=%s"),
            //    code, sFileName));
            results.Add(sFileName);
            fFound = oDir.GetNext(&sFileName);
        }
    }
    return (results.GetCount() > 0);
}

wxString GetSystemLanguageCode()
{
    /*
    Tries to guess current machine system language and returns it
    */
    wxArrayString langCodes;
    wxArrayString langNames;

    GetLanguages(langCodes, langNames);
    int sysLang = wxLocale::GetSystemLanguage();
    const wxLanguageInfo *info = wxLocale::GetLanguageInfo(sysLang);

    if (info) {
        wxString fullCode = info->CanonicalName;
        if (fullCode.length() < 2)
            return _T("en");

        wxString code = fullCode.Left(2);
        unsigned int i;

        for(i=0; i<langCodes.GetCount(); i++) {
            if (langCodes[i] == fullCode)
                return fullCode;

            if (langCodes[i] == code)
                return code;
        }
    }

    return _T("en");
}

void GetLanguages(wxArrayString &langCodes, wxArrayString &langNames)
{
    //Get the list of languages for existing LenMus translations.
    //Return a list of language codes (like "es", "fr", or "pt_BR") 
    //and corresponding language names in original language, not in English,
    //like "Español" or "Português".

    //AWARE using string translation ( macro _() ) is not possible as locale could
    //not yet be set, as this code is used at first run.

    // supported languages table
    typedef struct lmLangDataStruct {
        wxString sLangCode;
        wxString sLangName;
    } lmLangData;

    //Ordered by ISO code
    static const lmLangData tLanguages[] = { 
        { _T("el"),     wxString("Ελληνικά", wxConvUTF8) }, //Greek
        { _T("en"),     _T("English") },                    //English 
        { _T("es"),     wxString("Español", wxConvUTF8) },  //Spanish 
        { _T("eu"),     _T("Euskara") },                    //Basque 
        { _T("fr"),     wxString("Français", wxConvUTF8) }, //French
        { _T("gl_ES"),  _T("Galego") },                     //Galician
        { _T("it"),     _T("Italiano") },                   //Italian 
        { _T("nl"),     _T("Nederlands") },                 //Dutch 
        { _T("tr"),     wxString("Türkçe", wxConvUTF8) },   //Turkish 
    };

    for(int j=0; j < sizeof(tLanguages)/sizeof(lmLangData); j++)
    {
        langNames.Add(tLanguages[j].sLangName);
        langCodes.Add(tLanguages[j].sLangCode);
    }
}

