ev3linuxfb
==========

Qt integration plugin for the Lego Mindstorms EV3. Coded for Qt 5.1.1 and based on qlinuxfb plugin.

 - First you must have compiled Qt for the EV3. You must have built without "-no-gui" configuration option. See this article in my blog for some explanations:

http://broija.blogspot.fr/2013/11/compile-qt-5-qtbase-for-lego-mindstorms.html

 - Download source code using git:

    git clone https://github.com/broija/ev3linuxfb

 - Copy ev3linuxfb directory to your qt source package directory. For example:

    cp -r ev3linuxfb qt-everywhere-opensource-src-5.1.1/qtbase/src/plugins/platforms/

 - Go to this directory and run the qmake you built for the EV3. For example:

    cd qt-everywhere-opensource-src-5.1.1/qtbase/src/plugins/platforms/
    qmake.ev3

 - Build:
  
    make

 - Copy the freshly built 'libev3linuxfb.so' plugin to your EV3 SD card, in /media/card/lib/platforms. The file is located in this directory:

    qt-everywhere-opensource-src-5.1.1/qtbase/plugins/platforms/

 - Copy all the fonts to your EV3 SD card, in /media/card/lib/fonts. They are located in the Qt installation directory:

    Qt5.1.1/lib/fonts

 - Set the environment for QPA (Qt Platform Abstraction) by entering these commands on your EV3: 

    export QT_QPA_FONTDIR=/media/card/lib/fonts && export QT_QPA_PLATFORM_PLUGIN_PATH=/media/card/lib/platforms

 - Run your application and tell Qt that you want it to use the ev3linuxfb platform plugin to display things on screen:

    ./myapp -platform ev3linuxfb
