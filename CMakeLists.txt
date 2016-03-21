project(XSecDemo)

cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0020 NEW)

if(MSYS)
	set(CMAKE_INSTALL_PREFIX /c/Users/brainpower/Seafile/ba/code/xsecdemo/installed/)
    set(CMAKE_PREFIX_PATH /c/Users/brainpower/Seafile/ba/code/xsecdemo/installed/)
endif()



find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_DIR} ${PROJECT_SOURCE_DIR}/cmake/)

find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(KF5TextEditor NO_MODULE)
#find_package(KF5IconThemes NO_MODULE)

find_package(LibXml2 REQUIRED)
find_package(LibXslt REQUIRED)
find_package(XMLSec REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(WIN32_EXECUTABLE ON)
option(DEBUG ON "Enable this for debugging purposes")


option(WIN32 "Build for WIN32 environment..." OFF)
option(DEBUG "Build in debug mode..." OFF)


# gettext translations search path
# set(LOCALEDIR "share/locale")

set(QT_MODULES Qt5::Widgets Qt5::Sql Qt5::Core)
add_definitions( -DBP_QT5 )
add_definitions( -std=gnu++14 )
#add_definitions( ${XMLSEC1_DEFINITIONS} ${XMLSEC1-OPENSSL_DEFINITIONS} )
add_definitions( ${LIBXSLT_DEFINITIONS} ${LIBXML2_DEFINITIONS} )

add_definitions( -D__XMLSEC_FUNCTION__=__FUNCTION__ -DXMLSEC_NO_SIZE_T -DXMLSEC_NO_GOST=1 -DXMLSEC_NO_XKMS=1 -DXMLSEC_NO_CRYPTO_DYNAMIC_LOADING=1  -DXMLSEC_OPENSSL_100=1 -DXMLSEC_CRYPTO_OPENSSL=1 -DXMLSEC_CRYPTO=\"openssl\" )

set(CORE_LIBS  ${XMLSEC1-OPENSSL_LIBRARIES} ${XMLSEC1_LIBRARIES}  ${LIBXSLT_LIBRARIES}  ${LIBXML2_LIBRARIES} )

set(LIBS ${LIBS} ${CORE_LIBS} ${Qt5Widgets_LIBRARIES} ${Qt5Core_LIBRARIES} KF5::TextEditor) # KF5::IconThemes)
message(STATUS ${LIBS})


include_directories(${Qt5Widgets_INCLUDE_DIRS} ${Qt5Core_INCLUDE_DIRS} ${XMLSEC1_INCLUDE_DIR} ${XMLSEC1-OPENSSL_INCLUDE_DIR} ${LIBXSLT_INCLUDE_DIR} ${LIBXML2_INCLUDE_DIR}
        C:/Users/brainpower/Seafile/ba/code/xsecdemo/installed/include/KF5/KIconThemes/)


set( SRCS src/main.cpp src/xsdmainwindow.cpp
    #      src/xsdsigndialog.cpp        src/xsdsigndialog.hpp
          src/xsd_reference_dialog.cpp src/xsd_reference_dialog.hpp
          src/xsd_sign_dialog.cpp      src/xsd_sign_dialog.hpp
          src/xsd_verify_dialog.cpp    src/xsd_verify_dialog.hpp
          src/xsd_encrypt_dialog.cpp   src/xsd_encrypt_dialog.hpp
          src/xsd_decrypt_dialog.cpp   src/xsd_decrypt_dialog.hpp
          src/file_select.cpp          src/file_select.hpp
)
set( CORE_SRCS lib/xseccore.cpp )

qt5_add_resources(SRCS data/xsecdemo.qrc)

add_library(xseccore ${CORE_SRCS} )
target_link_libraries(xseccore ${CORE_LIBS})

if(NOT DEBUG)
  add_executable(xsecdemo WIN32 ${SRCS})
else()
  add_executable(xsecdemo ${SRCS})
endif()

target_link_libraries(xsecdemo xseccore ${LIBS} )
