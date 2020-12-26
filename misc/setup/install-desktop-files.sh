#!/bin/sh
set -e
if ! test -e "$SETUP_INSTALLPATH"/kingpinq3.desktop.in; then
  exit 0
fi
xdg_desktop_menu=`which xdg-desktop-menu 2>/dev/null`
if test "x$xdg_desktop_menu" = x; then
  xdg_desktop_menu=./xdg-desktop-menu
fi
sed -e "s#^Exec=.*#Exec=$SETUP_INSTALLPATH/kingpinq3#" \
    -e "s#^Icon=.*#Icon=$SETUP_INSTALLPATH/kpq3.png#" \
    < $SETUP_INSTALLPATH/kingpinq3.desktop.in \
    > $SETUP_INSTALLPATH/kingpinq3.desktop
$xdg_desktop_menu install --novendor $SETUP_INSTALLPATH/kingpinq3.desktop
