/*
 *  app.cpp
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

#include "app.h"

#include "wx/image.h"
#include "wx/xrc/xmlres.h"
#include "frame.h"

IMPLEMENT_APP(App)

bool App::OnInit()
{
    wxXmlResource::Get()->InitAllHandlers();
    wxXmlResource::Get()->Load(wxT("rc/wxfpm.xrc"));
    Frame *frame = new Frame();

    frame->Show(true);

    return true;
}

