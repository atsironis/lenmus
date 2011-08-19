//---------------------------------------------------------------------------------------
//    LenMus Phonascus: The teacher of music
//    Copyright (c) 2002-2011 LenMus project
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
//---------------------------------------------------------------------------------------

#ifndef __LENMUS_APP_H__        //to avoid nested includes
#define __LENMUS_APP_H__

//lenmus
#include "lenmus_injectors.h"
#include "lenmus_standard_header.h"

//wxWidgets
#include <wx/wxprec.h>
#include <wx/app.h>
#include <wx/frame.h>
#include <wx/snglinst.h>

//in wxWigets all global functions (i.e. wxGetApp) live in the global namespace.
namespace lenmus
{
    class TheApp;
}
using lenmus::TheApp;
DECLARE_APP(lenmus::TheApp)


namespace lenmus
{

//forward declarations
class MainFrame;
class SplashFrame;

DECLARE_EVENT_TYPE(lmEVT_CHANGE_LANGUAGE, -1)
const int ID_CHANGE_LANGUAGE = ::wxNewId();


//---------------------------------------------------------------------------------------
// Define the application
class TheApp : public wxApp
{
protected:
    MainFrame* m_frame;
//    bool            m_fUseGui;
    wxSingleInstanceChecker*    m_pInstanceChecker;
    wxLocale*           m_pLocale;          //locale we'll be using (user config)
    SplashFrame*        m_pSplash;
    ApplicationScope    m_appScope;
    long                m_nSplashStartTime;
    long                m_nSplashVisibleMilliseconds;
//    wxBitmap        m_background;       //background bitmap (user config)

public:
    TheApp();

    // event handlers
    bool OnInit();
    int OnExit();
    void on_change_language(wxCommandEvent& WXUNUSED(event));

//    // Accessors
//    wxBitmap& GetBackgroundBitmap() const { return (wxBitmap&) m_background; }
//    wxString GetLanguageCanonicalName() { return m_pLocale->GetCanonicalName(); }
//    wxString GetLocaleName() { return m_pLocale->GetLocale(); }
//    wxString GetLocaleSysName() { return m_pLocale->GetSysName(); }
//    const wxString GetCurrentUser();
//
//	//overrides
//	virtual int FilterEvent(wxEvent& event);
//    virtual void OnFatalException();
//    virtual void OnInitCmdLine(wxCmdLineParser& parser);
//    virtual bool OnCmdLineParsed(wxCmdLineParser& parser);
//
//    ApplicationScope& app_scope() { return m_appScope; }
//    LibraryScope& library_scope() { return m_appScope.library_scope(); }

private:
    bool do_application_setup();
    void do_application_cleanup();
    void create_paths_object();
    void load_user_preferences();
    void create_needed_folders_if_dont_exist();
    wxString determine_exe_path();

    void get_main_window_placement(wxRect* frameRect, bool* fMaximized);
    void get_default_placement(wxRect* defRect);
    wxString get_installer_language();
    wxString choose_language(wxWindow *parent);
    void set_up_locale(wxString lang);
    SplashFrame* create_GUI(int milliseconds, bool fFirstTime);
//    void SendForensicLog(wxString& sLogFile, bool fHandlingCrash);
//    void ParseCommandLine();
    void set_up_current_language();
    void create_main_frame();
    void wait_and_destroy_splash();
    void show_welcome_window();
//    void RecoverScoreIfPreviousCrash();
//    void CheckForUpdates();
    void configure_midi();
//    void CreateDocumentManager();
//    void CreateDocumentTemplates();
    void initialize_xrc_resources();
//    void DefineTraceMasks();
//    void OpenDataBase();
//
//
//    DECLARE_EVENT_TABLE()

};


}   // namespace lenmus

#endif    // __LENMUS_APP_H__

