/*
 *  frame.h
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

#ifndef _FRAME_H_
#define _FRAME_H_

#include "wx/frame.h"

class Frame : public wxFrame
{

public:
    Frame( wxWindow* parent=(wxWindow *)NULL);

    void OnExit(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);

    void DoSetProps(void);
    void DoGetControls(void);
    void DoReadGroups(void);

private:
    wxListCtrl* lc_packs;
    wxListBox* lb_groups;
    
    DECLARE_EVENT_TABLE()
};

#endif  // _FRAME_H_
