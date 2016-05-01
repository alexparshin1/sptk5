/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          clayoutmanager.cpp  -  description
                             -------------------
    begin                : Sun May 18 2003
    copyright            : (C) 1999-2016 by Alexey Parshin. All rights reserved.
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at
   your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library
   General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

   Please report all bugs and problems to "alexeyp@gmail.com"
 ***************************************************************************/

#include <sptk5/sptk.h>

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <sptk5/cgui>
#include <sptk5/cxml>

using namespace std;
using namespace sptk;

createControlCallbackMap CLayoutManager::controlCreator;
map<string,Fl_Boxtype> CLayoutManager::m_boxTypeNames;

namespace sptk {
class CLayoutManagerInitializer {
public:
    CLayoutManagerInitializer();
};
}

const CLayoutManagerInitializer staticData;

CLayoutManagerInitializer::CLayoutManagerInitializer() {
    CLayoutManager::registerControl("button",CButton::creator);
    CLayoutManager::registerControl("check_buttons",CCheckButtons::creator);
    CLayoutManager::registerControl("radio_buttons",CRadioButtons::creator);
    CLayoutManager::registerControl("combo",CComboBox::creator);
    CLayoutManager::registerControl("string_input",CInput::creator);
    CLayoutManager::registerControl("text_input",CMemoInput::creator);
    CLayoutManager::registerControl("text",CBox::creator);
    CLayoutManager::registerControl("html",CHtmlBox::creator);
    CLayoutManager::registerControl("date_interval",CDateIntervalInput::creator);
    CLayoutManager::registerControl("phone_input",CPhoneNumberInput::creator);
    CLayoutManager::registerControl("integer_input",CIntegerInput::creator);
    CLayoutManager::registerControl("float_input",CFloatInput::creator);
    CLayoutManager::registerControl("password_input",CPasswordInput::creator);
    CLayoutManager::registerControl("date_input",CDateInput::creator);
    CLayoutManager::registerControl("time_input",CTimeInput::creator);
    CLayoutManager::registerControl("menu_bar",CMenuBar::creator);
    CLayoutManager::registerControl("list_view",CDBListView::creator);
    CLayoutManager::registerControl("font_combo",CFontComboBox::creator);
    CLayoutManager::registerControl("group",CGroup::creator);
    CLayoutManager::registerControl("scroll",CScroll::creator);
    CLayoutManager::registerControl("tree",CTreeView::creator);
    CLayoutManager::registerControl("progress",CProgressBar::creator);
    CLayoutManager::registerControl("tabs",CTabs::creator);
    CLayoutManager::registerControl("toolbar",CToolBar::creator);
    CLayoutManager::registerControl("splitter",CSplitter::creator);

    // Initializing box type names
    CLayoutManager::m_boxTypeNames["no"] = FL_NO_BOX;
    CLayoutManager::m_boxTypeNames["flat"] = FL_FLAT_BOX;
    CLayoutManager::m_boxTypeNames["thin up box"] = FL_THIN_UP_BOX;
    CLayoutManager::m_boxTypeNames["up box"] = FL_UP_BOX;
    CLayoutManager::m_boxTypeNames["thin down box"] = FL_THIN_DOWN_BOX;
    CLayoutManager::m_boxTypeNames["down box"] = FL_DOWN_BOX;
    CLayoutManager::m_boxTypeNames["thin up frame"] = FL_THIN_UP_FRAME;
    CLayoutManager::m_boxTypeNames["up frame"] = FL_UP_FRAME;
    CLayoutManager::m_boxTypeNames["thin down frame"] = FL_THIN_DOWN_FRAME;
    CLayoutManager::m_boxTypeNames["down frame"] = FL_DOWN_FRAME;
}

CLayoutManager::CLayoutManager(Fl_Group* group,int layoutSize,CLayoutAlign ca)
        : CLayoutClient(group,layoutSize,ca) {
    m_frame = 0L;
    m_layoutSpacing = 3;
    m_layoutGrowMode = LGM_AUTO_GROW;
    m_group = group;
    m_noXml = false;
    m_frameDrawBackground = false;
}

void CLayoutManager::clear() {
    m_group->clear();
}

void CLayoutManager::relayout() {
    if (!m_group)
        return;
    unsigned cnt = m_group->children();
    for (unsigned i = 0; i < cnt; i++) {
        CLayoutClient *ca = 0L;
        Fl_Widget *widget = m_group->child(i);
        if (!widget->visible())
            continue;
        try {
            ca = dynamic_cast<CLayoutClient *>(widget);
        } catch (...) {
            continue;
        }
        if (ca->layoutAlign() == SP_ALIGN_NONE)
            continue;
        ca->m_lastPreferredW = 0;
        ca->m_lastPreferredH = 0;
    }
    m_group->resize(m_group->x(),m_group->y(),m_group->w(),m_group->h());
}

int autoLayoutCounter;

bool CLayoutManager::autoLayout(int x,int y,int& w,int& h,bool resizeWidgets) const {
    autoLayoutCounter++;
    if (!m_group)
        return true;
/*
    static int layoutLevel = -1;
    layoutLevel++;
    string layoutIndent(layoutLevel*2,' ');
    const CTreeItem* item = dynamic_cast<const CTreeItem*>(this);
    if (layoutLevel < 5)
    {
        cout << layoutIndent << "Entering autolayout " << className() << " (" << w << "," << h << ") ";
        if (item)
            cout << item->label();
        cout << endl;
    }
*/
    int frame_dw,frame_dh,frame_dx,frame_dy;

    if (m_frame) {
        int dc = m_frame->frameWidth() + m_layoutSpacing;
        frame_dw = frame_dh = dc * 2;
        frame_dx = frame_dy = dc;
    } else {
        frame_dw = Fl::box_dw(m_group->box()) + m_layoutSpacing * 2;
        frame_dh = Fl::box_dh(m_group->box()) + m_layoutSpacing * 2;
        frame_dx = Fl::box_dx(m_group->box()) + m_layoutSpacing;
        frame_dy = Fl::box_dy(m_group->box()) + m_layoutSpacing;
    }

    for (;;) {
        int xx = x + frame_dx;
        int yy = y + frame_dy;
        int ww = w - frame_dw;
        int hh = h - frame_dh;
        int preferred_w, preferred_h;
        Fl_Widget *clientWidget = 0L;
        unsigned cnt = m_group->children();
        bool extended = false;
        CLayoutClient *ca;
        unsigned layoutWidgets = 0;
        for (unsigned i = 0; i < cnt; i++) {
            Fl_Widget *widget = m_group->child(i);
            if (!widget->visible())
                continue;
            try {
                ca = dynamic_cast<CLayoutClient *>(widget);
            } catch (...) {
                continue;
            }
            if (ca->layoutAlign() == SP_ALIGN_NONE)
                continue;

            preferred_w = ww;
            preferred_h = hh;
            if (preferred_w < 0)
                preferred_w = 20;
            if (preferred_h < 0)
                preferred_h = 20;

            layoutWidgets++;

            int preferred_x = xx, preferred_y = yy;

            switch (ca->layoutAlign()) {
            case SP_ALIGN_NONE:
                break;

            case SP_ALIGN_RIGHT:
                preferred_w = ca->layoutSize();
                ca->preferredSize(preferred_w,preferred_h);
                preferred_x = xx + ww - preferred_w;
                preferred_y += (hh - preferred_h) / 2;
                break;

            case SP_ALIGN_LEFT:
                preferred_w = ca->layoutSize();
                ca->preferredSize(preferred_w,preferred_h);
                preferred_y += (hh - preferred_h) / 2;
                break;

            case SP_ALIGN_BOTTOM:
                preferred_h = ca->layoutSize();
                if (preferred_w == ca->m_lastPreferredW && ca->m_lastPreferredH >= ca->layoutSize())
                    preferred_h = ca->m_lastPreferredH;
                else
                    ca->computeSize(preferred_w,preferred_h);
                preferred_y = yy + hh - preferred_h;
                break;

            case SP_ALIGN_TOP:
                preferred_h = ca->layoutSize();
                if (preferred_w == ca->m_lastPreferredW && ca->m_lastPreferredH >= ca->layoutSize())
                    preferred_h = ca->m_lastPreferredH;
                else
                    ca->computeSize(preferred_w,preferred_h);
                break;

            case SP_ALIGN_CLIENT:
                clientWidget = m_group->child(i);
                break;
            }

            switch (ca->layoutAlign()) {
            case SP_ALIGN_NONE:
                break;

            case SP_ALIGN_LEFT:
                xx += preferred_w + m_layoutSpacing;
            case SP_ALIGN_RIGHT:
                ww -= preferred_w + m_layoutSpacing;
                if (preferred_h > hh) {
                    if (!(m_layoutGrowMode & LGM_VERTICAL_GROW))
                        preferred_h = hh;
                    else {
                        extended = true;
                        if (hh < 0) {
                            h += preferred_h;
                            hh += preferred_h;
                        } else {
                            h += preferred_h - hh;
                            hh = preferred_h;
                        }
                    }
                }
                break;

            case SP_ALIGN_TOP:
                yy += preferred_h + m_layoutSpacing;
            case SP_ALIGN_BOTTOM:
                hh -= preferred_h + m_layoutSpacing;
                if (preferred_w > ww) {
                    if (!(m_layoutGrowMode & LGM_HORIZONTAL_GROW))
                        preferred_w = ww;
                    else {
                        extended = true;
                        if (ww < 0) {
                            w += preferred_w;
                            ww += preferred_w;
                        } else {
                            w += preferred_w - ww;
                            ww = preferred_w;
                        }
                    }
                }
                break;

            case SP_ALIGN_CLIENT:
                clientWidget = m_group->child(i);
                break;
            }

            if (resizeWidgets && ca->layoutAlign() != SP_ALIGN_CLIENT) {
                widget->resize(preferred_x,preferred_y,preferred_w,preferred_h);
/*
                cout << layoutIndent << " resizing " << ca->className()
                        << " " << preferred_x << ":" << preferred_y
                        << " " << preferred_w << "x" << preferred_h << endl;
            } else {
                cout << layoutIndent << " " << ca->className()
                        << " " << preferred_x << ":" << preferred_y
                        << " " << preferred_w << "x" << preferred_h << endl;
*/
            }
        }

        if (clientWidget) { // One widget has the client alignment
            CLayoutClient *ca = dynamic_cast<CLayoutClient *>(clientWidget);

            layoutWidgets++;

            preferred_w = ww; // - m_layoutSpacing;
            preferred_h = hh; // - m_layoutSpacing;
            if (preferred_w < 0)
                preferred_w = 20;
            if (preferred_h < 0)
                preferred_h = 20;

            if (preferred_w != ca->m_lastPreferredW || preferred_h != ca->m_lastPreferredH)
                ca->computeSize(preferred_w,preferred_h);

            if (resizeWidgets && !dynamic_cast<CScroll *>(m_group)) {
                if (preferred_w > ww && !(m_layoutGrowMode & LGM_HORIZONTAL_GROW))
                    preferred_w = ww;
                if (preferred_h > hh && !(m_layoutGrowMode & LGM_VERTICAL_GROW))
                    preferred_h = hh;
            }

            ww -= preferred_w; // + m_layoutSpacing;
            hh -= preferred_h; // + m_layoutSpacing;

            if (resizeWidgets)
                clientWidget->resize(xx,yy,preferred_w,preferred_h);
        }

        // Final adjustment
        if (m_layoutGrowMode & LGM_HORIZONTAL_GROW) {
            if (ww < 0)
                w -= ww;
        } else
            ww = 0;

        if (m_layoutGrowMode & LGM_VERTICAL_GROW) {
            if (hh < 0)
                h -= hh;
        } else
            hh = 0;
        if ((ww >= 0 && hh >= 0 && !extended) || layoutWidgets == 1)
            break;
    }
/*
    //if (layoutLevel < 5)
    {
        layoutLevel--;
        cout << layoutIndent << "Leaving autolayout  " << className() << " (" << w << "," << h << ") ";
        if (item)
            cout << item->label();
        cout << endl;
    }
*/
    return false;
}

void CLayoutManager::paintBackground() {
    if (m_frame)
        m_frame->drawResized(m_group->x(),m_group->y(),m_group->w(),m_group->h(),m_frameDrawBackground);
}

void CLayoutManager::loadLayout(const CXmlNode* groupNode,CLayoutXMLmode xmlMode) THROWS_EXCEPTIONS {
    if (m_noXml)
        return;

    string className = "group";
    CLayoutClient* layoutClient = dynamic_cast<CLayoutClient*>(m_group);
    if (layoutClient) {
        className = layoutClient->name() + "(" + layoutClient->className() + ")";
        layoutClient->load(groupNode,xmlMode);
    }

    if (xmlMode & (int) LXM_LAYOUT) {
        clear();
        m_group->begin();
        CXmlNode::const_iterator itor = groupNode->begin();
        for (; itor != groupNode->end(); itor++) {
            CXmlNode* widgetNode = *itor;
            if (!widgetNode->isElement())
                continue;
            string widgetType = widgetNode->name();
            //cout << "Creating " << widgetType << ": ";
            createControlCallbackMap::iterator cctor = controlCreator.find(widgetType);
            if (cctor == controlCreator.end()) {
                //cout << " not supported" << endl;
                continue; // the widget type isn't supported
            }
            createControlCallback& creator = cctor->second;
            CLayoutClient* layoutClient = creator(widgetNode);
            Fl_Widget* widget = layoutClient->m_widget;
            //cout << "created for parent " << hex << widget->parent() << endl;
            if (widgetNode->size()) {
                try {
                    CLayoutManager* layoutManager = dynamic_cast<CLayoutManager*>(widget);
                    if (layoutManager)
                        layoutManager->loadLayout(widgetNode,xmlMode);
                } catch (...) {}
            }
            else {
                try {
                    CControl* cwidget = dynamic_cast<CControl*>(widget);
                    if (cwidget)
                        cwidget->load(widgetNode,xmlMode);
                } catch (...) {}
            }
            if (widget->parent() != m_group)
                m_group->add
                (widget);
        }
        m_group->end();
    } else {
        map<string,CXmlNode *> xmlControls;
        map<string,CXmlNode *> xmlGroups;
        CXmlNode::const_iterator itor = groupNode->begin();
        for (; itor != groupNode->end(); itor++) {
            CXmlNode* node = *itor;
            string label = node->getAttribute("label");
            if (label.empty())
                label = "noName:" + node->getAttribute("nn_index");
            if (node->name() == "group")
                xmlGroups[label] = node;
            else if (node->name() == "control")
                xmlControls[label] = node;
        }

        for (int i = 0; i < m_group->children(); i++) {
            Fl_Widget *widget = m_group->child(i);
            try {
                CLayoutManager *group = dynamic_cast<CLayoutManager *>(widget);
                if (group) {
                    string glabel = widget->label();
                    if (glabel.empty())
                        glabel = "noName:" + int2string(i);
                    map<string,CXmlNode *>::iterator itor = xmlGroups.find(glabel);
                    if (itor != xmlGroups.end()) {
                        CXmlNode* node = itor->second;
                        group->loadLayout(node,xmlMode);
                    }
                    continue;
                }
            } catch (...) {}
            try {
                CControl *control = dynamic_cast<CControl *>(widget);
                if (control) {
                    string clabel = control->label();
                    if (clabel.empty())
                        clabel = "noName:" + int2string(i);
                    map<string,CXmlNode *>::iterator itor = xmlControls.find(clabel);
                    if (itor != xmlControls.end()) {
                        CXmlNode* node = itor->second;
                        control->load(node,xmlMode);
                    }
                    continue;
                }
            } catch (...) {}
        }
    }
    //cout << className << " has " << m_group->children() << " children, parent " << hex << m_group->parent() << endl << endl;
}

void CLayoutManager::saveLayout(CXmlNode* groupNode,CLayoutXMLmode xmlMode) const {
    groupNode->clear();
    if (m_noXml)
        return;
    CLayoutClient* layoutClient = dynamic_cast<CLayoutClient*>(m_group);
    if (layoutClient)
        layoutClient->save(groupNode,xmlMode);
    unsigned childCount = m_group->children();
    CScroll* scroll = dynamic_cast<CScroll*>(m_group);
    if (scroll)
        childCount -= 2; // Skipping scrollbars
    groupNode->name("group");
    for (unsigned i = 0; i < childCount; i++) {
        Fl_Widget *widget = m_group->child(i);
        try {
            CLayoutManager* layoutManager = dynamic_cast<CLayoutManager*>(widget);
            CLayoutClient* layoutClient = dynamic_cast<CLayoutClient*>(widget);
            if (layoutClient) {
                CXmlNode* node = new CXmlElement(groupNode,layoutClient->className().c_str());
                if (layoutManager) {
                    layoutManager->saveLayout(node,xmlMode);
                } else {
                    CControl* control = dynamic_cast<CControl*>(widget);
                    if (xmlMode != LXM_DATA || control)
                        layoutClient->save(node,xmlMode);
                }
                if (node->empty() && !node->hasAttributes())
                    groupNode->remove(node);
                else {
                    if (widget->label() == 0 || widget->label()[0] == 0)
                        node->setAttribute("nn_index",i);
                }
                continue;
            }
        } catch (...) {}
    }
}

void CLayoutManager::nameIndex(CWidgetNamesMap& index,bool recursive,bool clean) const {
    if (clean)
        index.clear();
    for (unsigned i = 0; i < (unsigned) m_group->children(); i++) {
        Fl_Widget *widget = m_group->child(i);
        try {
            CLayoutClient *client = dynamic_cast<CLayoutClient *>(widget);
            if (client && !client->name().empty())
                index[client->name()] = widget;
        } catch (...) {}

        if (recursive)
            try {
                CLayoutManager *group = dynamic_cast<CLayoutManager *>(widget);
                if (group)
                    group->nameIndex(index,true,false);
            } catch (...) {}
    }
}

unsigned CLayoutManager::frameWidth() const {
    if (m_frame)
        return m_frame->frameWidth();
    if (m_group)
        return Fl::box_dx(m_group->box());
    return 0;
}
