/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CThemes.h - description                                ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Wednesday November 2 2005                              ║
║  copyright            (C) 1999-2018 by Alexey Parshin. All rights reserved.  ║
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

#ifndef __CTHEMES_H__
#define __CTHEMES_H__

#include <FL/Fl.H>
#include <sptk5/gui/CFrame.h>
#include <sptk5/gui/CRect.h>
#include <sptk5/gui/CFont.h>
#include <sptk5/gui/CIcon.h>
#include <sptk5/gui/CTabImage.h>
#include <sptk5/cxml>
#include <sptk5/gui/CThemeImageCollection.h>
#include <sptk5/gui/CThemeColorCollection.h>
#include <sptk5/gui/CThemeScrollBar.h>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

#define MAX_BACKGROUND_IMAGES 7

/**
 * @brief Theme button size - defines the button size and look to select the correct pixmap
 */
enum CThemeButtonType { THM_BUTTON_COMBO, THM_BUTTON_CHECK, THM_BUTTON_RADIO, THM_BUTTON_NORMAL };

/**
 * @brief Widget themes
 *
 * Class CThemes is a static loader for the themes. Only one one instance of the class should exist, and it's created automatically.
 * The class allows to load a set of routines to draw the particular elements of the interface, and the images for these routines.
 */
class CThemes :
    public CThemeScrollBar
{
    friend class CBaseButton;

    /**
     * Theme name
     */
    static std::string m_name;


    /**
     * Original flat box draw function
     */
    static Fl_Box_Draw_F *m_flatBox;

    /**
     * Original thin up box draw function
     */
    static Fl_Box_Draw_F *m_thinUpBox;

    /**
     * Original up box draw function
     */
    static Fl_Box_Draw_F *m_upBox;

    /**
     * Original thin down box draw function
     */
    static Fl_Box_Draw_F *m_thinDownBox;

    /**
     * Original down box draw function
     */
    static Fl_Box_Draw_F *m_downBox;


    /**
     * Original thin up frame draw function
     */
    static Fl_Box_Draw_F *m_thinUpFrame;

    /**
     * Original up frame draw function
     */
    static Fl_Box_Draw_F *m_upFrame;

    /**
     * Original thin down frame draw function
     */
    static Fl_Box_Draw_F *m_thinDownFrame;

    /**
     * Original down frame draw function
     */
    static Fl_Box_Draw_F *m_downFrame;


    /**
     * Available fonts
     */
    static CFontsMap      m_fonts;

    /**
     * Available frames
     */
    static CFrames        m_frames;

    /**
     * Path to the theme' folder
     */
    static std::string    m_themeFolder;


    /**
     * An array of the background images to replace the original FLTK colors
     * FL_DARK3..FL_BACKGROUND..FL_LIGHT3
     */
    static CPngImage     *m_background[MAX_BACKGROUND_IMAGES];

    /**
     * Four different types of button pixmaps - for normal button, normal pressed button, default button, and default pressed button accordingly.
     */
    static CThemeImageCollection m_normalButtons;

    /**
     * Two different types of combo box button pixmaps - for normal button, and normal pressed button. Two extra array members are reserved.
     */
    static CThemeImageCollection m_comboButtons;

    /**
     * Two different types of check box button pixmaps - for check box unchecked, and check box checked. Two extra array members are reserved.
     */
    static CThemeImageCollection m_checkButtons;

    /**
     * Two different types of radio button pixmaps - for radio button unchecked, and radio button checked. Two extra array members are reserved.
     */
    static CThemeImageCollection m_radioButtons;

    /**
     * @brief Images for painting a progress bar
     *
     * One image is for the frame, one for the progress bar itself. Images should have the
     * compatible size. If you put the progress bar image on top its frame - they should show
     * a picture of 100% progress
     */
    static CThemeImageCollection m_progressBar[2];

    /**
     * @brief Colors for painting theme elements
     *
     * Every color of {bg,fg,base,text} defined here has states (THM_COLOR_NORMAL,THM_COLOR_PRELIGHT,etc).
     */
    static CThemeColorCollection m_colors;

    /**
     * @brief Three images for painting tabs
     *
     * One image is for the inactive tab, one for the active tab, and one for the frame on the bottom of tabs
     */
    static CTabImages      m_tabImages;

    /**
     * Flag indicating if buttons should be drawn as flat bars if mouse isn't over the button
     */
    static bool           m_flatButtons;

    /**
     * Flag indicating that theme is GTK2 theme
     */
    static bool           m_gtkTheme;

    /**
     * Theme registry. Contains several importang theme settings.
     */
    static xml::Document       *m_registry;

    /**
     * Restores the original FLTK theme
     */
    static void reset();

    /**
     * Returns the correct image collection for the given button size
     * @param sz CThemeButtonType, the general type of the button. @see CThemeButtonType for possible button types.
     */
    static CThemeImageCollection& sizeToButtonImages(CThemeButtonType sz);

    /**
     * Loads and sets the pixmap from the file for the given button size and type
     * @param sz CThemeButtonType, theme button size
     * @param bt CThemeImageState, theme button type
     * @param fileName std::string, PNG file name
     * @param extenalFile bool, true if the file is not a part of SPTK theme
     */
    static void replaceButton(CThemeButtonType sz,CThemeImageState bt,std::string fileName,bool externalFile=false);

    /**
     * Loads and sets the pixmap from the file for the given button size and type
     * @param sz CThemeButtonType, theme button size
     * @param bt CThemeImageState, theme button type
     * @param fileNameNormal std::string, PNG file name for normal button image
     * @param fileNameHighlighted std::string, PNG file name for highlighted button image
     * @param extenalFile bool, true if the file is not a part of SPTK theme
     */
    static void replaceButton(CThemeButtonType sz,CThemeImageState btndx,std::string fileNameNormal,std::string fileNameHighlighted,bool externalFile);

    /**
     * Loads and sets the pixmap from the file for the given pixmap array member
     * @param ndx int, image index
     * @param fileName std::string, pixmap file name
     */
    static void replaceImage(CPngImage** images, int ndx, const std::string& fileName);

    /**
     * Paints the rectangle with the background image.
     * @param xx int, rectangle x-coordinate
     * @param yy int, rectangle y-coordinate
     * @param ww int, rectangle width
     * @param hh int, rectangle height
     * @param image CPngImage *, image to use
     */
    static void paintBackground(int xx,int yy,int ww,int hh,CPngImage *image);

    /**
     * @brief Draws theme box with optional frame
     * @param x int, box x-coordinate
     * @param y int, box y-coordinate
     * @param w int, box width
     * @param h int, box height
     * @param color Fl_Color, box color
     * @param frameType Fl_Boxtype, box frame type
     */
    static void drawThemeBox(int x,int y,int w,int h,Fl_Color color,Fl_Boxtype frameType=FL_NO_BOX);

    /**
     * @brief Draws theme box with optional frame
     * @param x int, box x-coordinate
     * @param y int, box y-coordinate
     * @param w int, box width
     * @param h int, box height
     * @param frameType Fl_Boxtype, box frame type
     */
    static void drawThemeFrame(int x,int y,int w,int h,Fl_Boxtype frameType);

    /**
     * Determines the background image (if any) for the given color in range
     * FL_DARK3..FL_LIGHT3
     */
    static CPngImage *imageForColor(Fl_Color clr);

    /**
     * Flag the shows if theme buttons should be desaturated for the inactive
     * state of the button
     */
    static bool m_desaturateInactiveButtons;

    /**
     * Defines a radius of the focus frame' corner
     */
    static int  m_buttonFocusRadius;

    /**
     * Draws a check or radio button image
     *
     * @param buttonImages CThemeImageCollection&, a collection of button images
     * @param x int, x-coordinate of the button
     * @param y int, y-coordinate of the button
     * @param checked bool, is button checked?
     * @param highlited bool, is button highlited?
     * @returns false, if theme has no pixmap for the button, or true otherwise
     */
    static bool drawCheckOrRadioButton(CThemeImageCollection& buttonImages,int x,int y,bool checked,bool highlited);

protected:
    /**
     * Icons for use with buttons (four different sizes available, @see CIconSize)
     */
    static CIconMap m_icons[4];

    /**
     * @brief Loads GTK theme button file name from 'image' node, and stores it into the map
     * @param imageNode xml::Node*, an image node from GTK theme description
     * @param buttonFileNames std::map<CThemeImageState,std::string>&, file names for buttons
     */
    static void loadGtkButton(xml::Node* imageNode,std::map<CThemeImageState,std::string>& buttonFileNames);

    /**
     * @brief Loads GTK theme button file names from XPath
     * @param xml xml::Document&, GTK theme converted to XML
     * @param XPath std::string, XPath to button images
     * @param fileNames std::map<CThemeImageState,std::string>&, file names for buttons
     * @param orientation std::string, optional orientation (used for scrollbar elements)
     */
    static void loadGtkButtonFileNames(xml::Document& xml,std::string XPath,std::map<CThemeImageState,std::string>& fileNames,std::string orientation="");

    /**
     * @brief Loads GTK theme button images from 'style' node
     * @param xml xml::Document&, GTK theme converted to XML
     * @param styleName std::string, image style name ("button", "checkbutton", etc)
     * @param buttons CThemeImageCollection&, set of images for the button type
     * @param function std::string, function ("BOX", CHECK", "OPTION", etc)
     */
    static void loadGtkButtons(xml::Document& xml, const String& styleName, CThemeImageCollection& buttons,
                               const String& function);

    /**
     * @brief Loads current Gnome GTK theme
     *
     * The GTK theme name is read from .gconf directory.
     * @param gtkThemeName std::string, GTK2 theme name
     */
    static void loadGtkTheme(const String& gtkThemeName);

public:

    /**
     * Default constructor
     */
    CThemes();

    /**
     * Destructor
     */
    ~CThemes();

    /**
     * Loads and sets the theme. The themeName is the directory with the theme images.
     */
    static void set(std::string themeName);

    /**
     * @brief Returns a button icon by symbolic name
     * @param iconName string, symbolic name for an icons
     * @param iconSize CIconSize, the size of the icon
     * @returns pointer to the icon, or NULL if not found
     */
    static CIcon* getIcon(const std::string& iconName, CIconSize iconSize);

    /**
     * @brief Returns a button icon image by symbolic name
     * @param iconName string, symbolic name for an icons
     * @param iconSize CIconSize, the size of the icon
     * @returns pointer to the image, or NULL if not found
     */
    static CPngImage* getIconImage(const std::string& iconName, CIconSize iconSize);

    /**
     * @brief Registers a new button icon
     * @param icon CIcon*, pointer to the icon
     * @param iconSize CIconSize, the size of the icon
     */
    static void registerIcon(CIcon* icon,CIconSize iconSize);

    /**
     * The replacement rountine to draw  FL_FLAT_BOX
     */
    static void drawFlatBox(int x,int y,int w,int h,Fl_Color clr);

    /**
     * The replacement rountine to draw  FL_THIN_UP_BOX
     */
    static void drawThinUpBox(int x,int y,int w,int h,Fl_Color clr);

    /**
     * The replacement rountine to draw  FL_UP_BOX
     */
    static void drawUpBox(int x,int y,int w,int h,Fl_Color clr);

    /**
     * The replacement rountine to draw  FL_THIN_DOWN_BOX
     */
    static void drawThinDownBox(int x,int y,int w,int h,Fl_Color clr);

    /**
     * The replacement rountine to draw  FL_DOWN_BOX
     */
    static void drawDownBox(int x,int y,int w,int h,Fl_Color clr);

    /**
     * The replacement rountine to draw  FL_THIN_UP_FRAME
     */
    static void drawThinUpFrame(int x,int y,int w,int h,Fl_Color clr);

    /**
     * The replacement rountine to draw  FL_UP_FRAME
     */
    static void drawUpFrame(int x,int y,int w,int h,Fl_Color clr);

    /**
     * The replacement rountine to draw  FL_THIN_DOWN_FRAME
     */
    static void drawThinDownFrame(int x,int y,int w,int h,Fl_Color clr);

    /**
     * The replacement rountine to draw  FL_DOWN_FRAME
     */
    static void drawDownFrame(int x,int y,int w,int h,Fl_Color clr);

    /**
     * The rountine to draw theme button
     * @param sz CThemeButtonType,
     * @param x int, button x-coordinate
     * @param y int, button y-coordinate
     * @param w int&, button width, may be altered by button to meet the button size requirements
     * @param h int&, button height, may be altered by button to meet the button size requirements
     * @param highlited bool, true to draw the highlighted button
     * @param pressed bool, true to draw pressed button
     * @param defaultButton bool, true to draw default button
     */
    static bool drawButton(CThemeButtonType sz,int x,int y,int& w,int& h,bool highlited,bool pressed,bool defaultButton);

    /**
     * Computes and returns the correct size of the button in the parameters.
     * @param sz CThemeButtonType, the general type of the button. @see CThemeButtonType for possible button types.
     * @param w int&, button width, may be altered by button to meet the button size requirements
     * @param h int&, button height, may be altered by button to meet the button size requirements
     * @returns false, if theme has no pixmap for the button, or true otherwise
     */
    static bool sizeButton(CThemeButtonType sz,int& w,int& h);

    /**
     * Computes and returns the minimum size of the progress bar
     * @param w int&, progress bar width, may be altered by progress bar to meet the progress bar size requirements
     * @param h int&, progress bar height, may be altered by progress bar to meet the progress bar size requirements
     * @returns false, if theme has no pixmap for the progress bar, or true otherwise
     */
    static bool sizeProgressBar(int& w,int& h);

    /**
     * Returns the button focuse corner radius
     */
    static int  buttonFocusRadius() {
        return m_buttonFocusRadius;
    }

    /**
     * Draws a check button image
     * @returns false, if theme has no pixmap for the button, or true otherwise
     * @param x int, x-coordinate of the button
     * @param y int, y-coordinate of the button
     * @param checked bool, is button checked?
     * @param highlited bool, is button highlited?
     */
    static bool drawCheckButton(int x,int y,bool checked,bool highlited);

    /**
     * Draws a radio button image
     * @returns false, if theme has no pixmap for the button, or true otherwise
     * @param x int, x-coordinate of the button
     * @param y int, y-coordinate of the button
     * @param checked bool, is button checked?
     * @param highlited bool, is button highlited?
     */
    static bool drawRadioButton(int x,int y,bool checked,bool highlited);

    /**
     * Draws a progress bar image
     * The height of the image is defined by the 'progress0.png' image of the theme.
     * The width of the image is adjusted as required, but it can't be less than
     * returned by sizeProgressBar() method.
     * @returns false, if theme has no pixmaps for the progress bar, or true otherwise
     * @param x int, x-coordinate of the progress bar
     * @param y int, y-coordinate of the progress bar
     * @param w int, width of the progress bar
     * @param percent float, percent value of the progress bar
     */
    static bool drawProgressBar(int x,int y,int w,float percent);

    /**
     * @brief Draws a tab.
     * @returns false, if theme has no pixmaps for the tab, or true otherwise
     * @param x int, x-coordinate of the scroll bar
     * @param y int, y-coordinate of the scroll bar
     * @param w int, width of the scroll bar
     * @param h int, width of the scroll bar
     * @param active bool, true if stab is active
     */
    static bool drawTab(int x,int y,int w,int h,bool active);

    /**
     * @brief Draws a tab frame
     *
     * Draws a bottom frame for the tabs
     * @returns false, if theme has no pixmaps for the tab frame, or true otherwise
     * @param x int, x-coordinate of the scroll bar
     * @param y int, y-coordinate of the scroll bar
     * @param w int, width of the scroll bar
     * @param h int, width of the scroll bar
     */
    static bool drawTabFrame(int x,int y,int w,int h);

    /**
     * Returns the list of the available themes
     */
    static Strings availableThemes();

    /**
     * @brief Themes root directories
     *
     * On different system it may be a different set of directories.
     * The first available correct combination of theme directory and theme name
     * defines the theme directory
     */
    static const Strings& searchDirectories();

    /**
     * @brief All the available screen fonts
     */
    static CFontsVector   screenFonts;

    /**
     * @brief Returns theme name
     */
    static std::string name() { return m_name; }

    /**
     * @brief Returns path to the theme folder
     */
    static std::string themeFolder() { return m_themeFolder; }

    /**
     * @brief Theme tar archive. Contains all the theme data
     */
    static Tar           m_tar;
};

/**
 * @}
 */
}
#endif
