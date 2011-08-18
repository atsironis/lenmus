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

#ifndef __LENMUS_THEO_INTERVALS_CTROL_PARAMS_H__        //to avoid nested includes
#define __LENMUS_THEO_INTERVALS_CTROL_PARAMS_H__

//lenmus
#include "lenmus_exercise_params.h"

#include "lenmus_theo_intervals_constrains.h"
//#include "ObjectParams.h"
//#include "../ldp_parser/AuxString.h"
//#include "ParamsParser.h"
//

namespace lenmus
{

//---------------------------------------------------------------------------------------
// This class pack all parameters to set up a Theo Intervals Identification exercise,
// The settings must be read/setup by the TheoIntervalsCtrol object.
class TheoIntervalsCtrolParams : public ExerciseParams
{
public:
    TheoIntervalsCtrolParams(EBookCtrolOptions* pConstrains);
    ~TheoIntervalsCtrolParams();


protected:
    void do_final_settings();
    void process(ImoParamInfo* pParam);

//    // html object window attributes
//    long    m_nWindowStyle;
//    TheoIntervalsConstrains* m_pConstrains;
//
//    DECLARE_NO_COPY_CLASS(TheoIntervalsCtrolParams)
};


//=======================================================================================
// Implementation
//=======================================================================================
TheoIntervalsCtrolParams::TheoIntervalsCtrolParams(EBookCtrolOptions* pConstrains)
    : ExerciseParams(pConstrains)
{
}


//---------------------------------------------------------------------------------------
TheoIntervalsCtrolParams::~TheoIntervalsCtrolParams()
{
}

//---------------------------------------------------------------------------------------
void TheoIntervalsCtrolParams::process(ImoParamInfo* pParam)
{
    // problem_level     0 | 1 | 2 | 3                                  [2]
    // accidentals       none | simple | double                         [none]
    // problem_type      DeduceInterval | BuildInterval
    // clef*             G | F4 | F3 | C4 | C3 | C2 | C1                [G]
    // control_settings  Value="[key for storing the settings]"
    //                      By coding this param it is forced the inclusion of
    //                      the 'settings' link. Its value will be used
    //                      as the key for saving the user settings.

    TheoIntervalsConstrains* pConstrains
        = dynamic_cast<TheoIntervalsConstrains*>( m_pConstrains );

    string& name = pParam->get_name();
    string& value = pParam->get_value();


    // accidentals        none | simple | double
    if (name == "accidentals")
    {
        if (value == "none")
        {
            pConstrains->SetAccidentals(false);
            pConstrains->SetDoubleAccidentals(false);
        }
        else if (value == "simple")
            pConstrains->SetAccidentals(true);
        else if (value == "double")
            pConstrains->SetDoubleAccidentals(true);
        else
            error_invalid_param(name, value, "none | simple | double");
    }

    //problem_type    DeduceInterval | BuildInterval
    else if ( name == "problem_type")
    {
        if (value == "DeduceInterval")
            pConstrains->SetProblemType( TheoIntervalsConstrains::k_deduce_interval );
        else if (value == "BuildInterval")
            pConstrains->SetProblemType( TheoIntervalsConstrains::k_build_interval );
        else
            error_invalid_param(name, value, "DeduceInterval | BuildInterval");
    }

    //problem_level     0 | 1 | 2 | 3
    else if ( name == "problem_level")
    {
        int nLevel;
        bool fOK = pParam->get_value_as_int(&nLevel);
        if (!fOK || nLevel < 0 || nLevel > 3)
            error_invalid_param(name, value, "0 | 1 | 2 | 3");
        else
            pConstrains->SetProblemLevel( nLevel );
    }

//TODO
//    // clef        G | F4 | F3 | C4 | C3 | C2 | C1
//    else if ( name == "clef")
//    {
//        EClefType nClef = E_Sol;        //default value
//        m_sParamErrors += ParseClef(value, &nClef);
//        pConstrains->SetClef(nClef, true);
//    }

    // Unknown param
    else
        ExerciseParams::process(pParam);

}

//---------------------------------------------------------------------------------------
void TheoIntervalsCtrolParams::do_final_settings()
{
//    // ensure that at least a Clef is selected
//    bool fClefSpecified = false;
//    for (int i=MIN_CLEF; i <= MAX_CLEF; i++) {
//        fClefSpecified = fClefSpecified || m_pConstrains->IsValidClef((EClefType)i);
//        if (fClefSpecified) break;
//    }
//    if (!fClefSpecified) {
//        m_pConstrains->SetClef(E_Sol, true);
//    }
}

//void TheoIntervalsCtrolParams::CreateHtmlCell(wxHtmlWinParser *pHtmlParser)
//{
//
//    // create the window
//    wxWindow* wnd;
//    if (m_pConstrains->GetProblemType() == TheoIntervalsConstrains::k_build_interval)
//        wnd = new BuildIntervalCtrol((wxWindow*)g_pMainFrame->GetHtmlWindow(), -1,
//                                       m_pConstrains, wxPoint(0,0),
//                                       wxSize(m_nWidth, m_nHeight), m_nWindowStyle);
//    else
//        wnd = new IdfyIntervalCtrol((wxWindow*)g_pMainFrame->GetHtmlWindow(), -1,
//                                       m_pConstrains, wxPoint(0,0),
//                                       wxSize(m_nWidth, m_nHeight), m_nWindowStyle);
//    wnd->Show(true);
//    pHtmlParser->GetContainer()->InsertCell(new wxHtmlWidgetCell(wnd, m_nPercent));
//}


}   // namespace lenmus

#endif  // __LENMUS_THEO_INTERVALS_CTROL_PARAMS_H__
