SPTK_THEMES = \
    Blue.tar \
    Dark.tar \
    Flat.tar \
    Keramic.tar \
    OSX.tar \
    OSX_Flat.tar \
    Opera.tar \
    E17.tar \
    Default.tar \
    tar2h

tar2h: tar2h.cpp
	$(CXX) -c $< -o $@.o
	$(CXX) $@.o -o $@

Blue.tar : $(srcdir)/Blue/*.png $(srcdir)/Blue/theme.ini $(srcdir)/Blue/dialog_icons/*.png $(srcdir)/Blue/icons.16/*.png
	cd Blue; tar cf ../Blue.tar * --exclude .svn

Default.tar : $(srcdir)/Default/theme.ini $(srcdir)/Default/dialog_icons/*.png $(srcdir)/Default/icons.16/*.png tar2h
	cd Default; tar cf ../Default.tar * --exclude .svn
	./tar2h Default.tar default_icons > ../sptk3/gui/default_icons.h

E17.tar : $(srcdir)/E17/*.png $(srcdir)/E17/theme.ini $(srcdir)/E17/dialog_icons/*.png $(srcdir)/E17/icons.16/*.png
	cd E17; tar cf ../E17.tar * --exclude .svn

Flat.tar : $(srcdir)/Flat/*.png $(srcdir)/Flat/theme.ini $(srcdir)/Flat/dialog_icons/*.png $(srcdir)/Flat/icons.16/*.png
	cd Flat; tar cf ../Flat.tar * --exclude .svn 

Keramic.tar : $(srcdir)/Keramic/*.png $(srcdir)/Keramic/theme.ini $(srcdir)/Keramic/dialog_icons/*.png $(srcdir)/Keramic/icons.16/*.png
	cd Keramic; tar cf ../Keramic.tar * --exclude .svn 

OSX.tar : $(srcdir)/OSX/*.png $(srcdir)/OSX/theme.ini $(srcdir)/OSX/dialog_icons/*.png $(srcdir)/OSX/icons.16/*.png
	cd OSX; tar cf ../OSX.tar * --exclude .svn

OSX_Flat.tar : $(srcdir)/OSX_Flat/*.png $(srcdir)/OSX_Flat/theme.ini $(srcdir)/OSX_Flat/dialog_icons/*.png $(srcdir)/OSX_Flat/icons.16/*.png
	cd OSX_Flat; tar cf ../OSX_Flat.tar * --exclude .svn 

Opera.tar : $(srcdir)/Opera/*.png $(srcdir)/Opera/theme.ini $(srcdir)/Opera/dialog_icons/*.png $(srcdir)/Opera/icons.16/*.png
	cd Opera; tar cf ../Opera.tar * --exclude .svn

Dark.tar : $(srcdir)/Dark/*.png $(srcdir)/Dark/theme.ini $(srcdir)/Dark/dialog_icons/*.png $(srcdir)/Dark/icons.16/*.png
	cd Dark; tar cf ../Dark.tar * --exclude .svn

all: $(SPTK_THEMES)

nobase_pkgdata_DATA = \
   $(srcdir)/Blue.tar \
   $(srcdir)/Dark.tar \
   $(srcdir)/E17.tar \
   $(srcdir)/Flat.tar \
   $(srcdir)/Keramic.tar \
   $(srcdir)/OSX.tar \
   $(srcdir)/OSX_Flat.tar \
   $(srcdir)/Opera.tar
