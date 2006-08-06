/*
 *  frame.cpp
 * 
 *  Copyright (c) 2006 by Miklos Nemeth <desco@frugalware.org>
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, 
 *  USA.
 */

#include "wx/wx.h"

#include "frame.h"

#include "wx/image.h"
#include "wx/xrc/xmlres.h"
#include "wx/treectrl.h"
#include "wx/listctrl.h"
#include <alpm.h>


BEGIN_EVENT_TABLE(Frame,wxFrame)
    EVT_MENU(XRCID("mni_exit"),Frame::OnExit)
    EVT_CLOSE(Frame::OnClose)
END_EVENT_TABLE()


Frame::Frame(wxWindow* parent)
{
    wxXmlResource::Get()->LoadFrame(this, parent, wxT("frame"));
    DoGetControls();
    DoSetProps();
    DoReadGroups();
}

void Frame::DoGetControls(void)
{
    lc_packs = XRCCTRL(*this,"lc_packs",wxListCtrl);
    lb_groups = XRCCTRL(*this,"lb_groups",wxListBox);
}

void Frame::DoSetProps(void)
{
    SetSize(wxSize(800,600));
    Centre(wxBOTH);

    lc_packs->InsertColumn(0,_("Name"));
    lc_packs->InsertColumn(1,_("Installed"));
    lc_packs->InsertColumn(2,_("Available"));
}

void Frame::OnClose(wxCloseEvent& WXUNUSED(event))
{

    if (wxMessageBox(_("Do you realy want to quit?"),_("Question"),wxYES_NO|wxICON_QUESTION) == wxYES)
        Destroy();
}

void Frame::OnExit(wxCommandEvent& WXUNUSED(event))
{

    if (wxMessageBox(_("Do you realy want to quit?"),_("Question"),wxYES_NO|wxICON_QUESTION) == wxYES)
        Destroy();
}

void Frame::DoReadGroups(void)
{
    wxArrayString array;
    wxString str;
    
    PM_DB *db_local;
    PM_LIST *lp;
    
    
    array.Add(_("All Packages"));
    
    if(alpm_initialize("/") == -1)
	{
		wxMessageBox(_("failed to initilize alpm library"),_("Error"),wxICON_ERROR);
		return;
	}
	
	if(alpm_set_option(PM_OPT_DBPATH, (long)PM_DBPATH) == -1)
	{
		wxMessageBox(_("failed to set option DBPATH"),_("Error"),wxICON_ERROR);
		alpm_release();
		return;
	}
	
	db_local = alpm_db_register("local");
	if(db_local == NULL)
	{
		wxMessageBox(_("could not register 'local' database"),_("Error"),wxICON_ERROR);
		return;
	}
	
	
	for(lp = alpm_db_getgrpcache(db_local); lp; lp = alpm_list_next(lp))
	{
	   PM_GRP *grp = (PM_GRP *)alpm_list_getdata(lp);
	   str = (char *)alpm_grp_getinfo(grp,PM_GRP_NAME);
	   array.Add(str);
	}
	
	alpm_release();
	
    lb_groups->InsertItems(array,0);
}
