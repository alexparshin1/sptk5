/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
║  email                alexeyp@gmail.com                                      ║
╚══════════════════════════════════════════════════════════════════════════════╝
┌──────────────────────────────────────────────────────────────────────────────┐
│   This library is free software; you can redistribute it and/or modify it    │
│   under the terms of the GNU Library General Public License as published by  │
│   the Free Software Foundation; either version 2 of the License, or (at your │
│   option) any later version.                                                 │
│                                                                              │
│   This library is distributed in the hope that it will be useful, but        │
│   WITHOUT ANY WARRANTY; without even the implied warranty of                 │
│   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library   │
│   General Public License for more details.                                   │
│                                                                              │
│   You should have received a copy of the GNU Library General Public License  │
│   along with this library; if not, write to the Free Software Foundation,    │
│   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include <sptk5/sptk.h>

#include <sptk5/gui/CFrame.h>

using namespace std;
using namespace sptk;

void CFrames::clear()
{
    for (auto itor: m_frames)
    {
        CFrame* frame = itor.second;
        delete frame;
    }
    m_frames.clear();
    m_fltkFrames.clear();
}

const Strings                   CFrames::frameTypeNames("up frame|thin up frame|thin down frame|down frame", "|");
const std::array<Fl_Boxtype, 4> CFrames::frameTypes = {
    FL_UP_FRAME, FL_THIN_UP_FRAME, FL_THIN_DOWN_FRAME, FL_DOWN_FRAME};

void CFrames::load(Tar& tar, const xdoc::SNode& framesNode)
{
    clear();
    for (const auto& frameNode: framesNode->nodes())
    {
        if (frameNode->getName() != "frame")
        {
            continue;
        }
        String fileName = (String) frameNode->attributes().get("image");
        if (fileName.empty())
        {
            continue;
        }
        String frameTypeStr = (String) frameNode->attributes().get("type");
        String frameName = (String) frameNode->attributes().get("name", frameTypeStr.c_str());
        if (frameTypeStr.empty())
        {
            frameTypeStr = frameName;
        }
        unsigned           frameTypeInt = (unsigned) frameTypeNames.indexOf(frameTypeStr);
        unsigned           frameWidth = frameNode->attributes().get("width", "1").toInt();
        unsigned           cornerZone = frameNode->attributes().get("corner", "1").toInt();
        Fl_Boxtype         frameType = FL_NO_BOX;
        CFrame::CFrameKind kind = CFrame::CFrameKind::USER_EXTENDED;
        if (frameTypeInt < 4)
        {
            frameType = frameTypes[frameTypeInt];
            kind = CFrame::CFrameKind::FLTK_STANDARD;
        }
        CPngImage::CPatternDrawMode drawMode = CPngImage::CPatternDrawMode::PDM_STRETCH;
        if ((String) frameNode->attributes().get("mode") == "tile")
        {
            drawMode = CPngImage::CPatternDrawMode::PDM_TILE;
        }
        const Buffer& imageData = tar.file(fileName.c_str());
        registerFrame(frameName, new CFrame(imageData, frameWidth, cornerZone, drawMode, kind), frameType);
    }
}

void CFrames::registerFrame(std::string frameName, CFrame* frame, Fl_Boxtype frameType)
{
    CFrame* oldFrame = find(frameName);
    delete oldFrame;
    if (frameType != FL_NO_BOX)
    {
        m_fltkFrames[frameType] = frame;
    }
    m_frames[frameName] = frame;
}

CFrame* CFrames::find(Fl_Boxtype frameType) const
{
    try
    {
        if (m_fltkFrames.empty())
        {
            return nullptr;
        }
        auto itor = m_fltkFrames.find(frameType);
        if (itor == m_fltkFrames.end())
        {
            return nullptr;
        }
        return itor->second;
    }
    catch (const Exception&)
    {
        return nullptr;
    }
}

CFrame* CFrames::find(std::string frameName) const
{
    auto itor = m_frames.find(frameName);
    if (itor == m_frames.end())
    {
        return nullptr;
    }
    return itor->second;
}
