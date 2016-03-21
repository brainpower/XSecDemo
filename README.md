XSecDemo
========


A small little Application which is meant to be used to showcase XML-Signature and XML-Encryption.


Build
-----

Configure the cmake project and make:

    $ mkdir build && cd build
    $ cmake ..
    $ make


Dependencies
------------

* Qt5
* KTextEditor / KatePart
* libxml2
* libxslt
* xmlsec

### Windows

When building for Windows, you'll usually need to compile KTextEditor and it's dependencies yourself.
Those are quite numerous, so this unordered and non-exhaustive list should give you a starting point:

* karchive
* kauth
* kbookmarks
* kcodecs
* kcompletion
* kconfig
* kconfigwidgets
* kcoreaddons
* kcrash
* kdbusaddons
* kglobalaccel
* kguiaddons
* ki18n
* kiconthemes
* kio
* kitemviews
* kjobwidgets
* kparts
* kservice
* ktexteditor
* ktextwidgets
* kwidgetsaddons
* kwindowsystem
* kxmlgui
* oxygen-icons5
* solid
* sonnet


