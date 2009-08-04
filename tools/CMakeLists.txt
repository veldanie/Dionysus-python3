find_package                (Qt4 REQUIRED)
set                         (QT_USE_QTOPENGL TRUE)
set                         (QT_USE_QTXML TRUE)
include                     (${QT_USE_FILE})

#find_library                (gle_LIBRARY                NAMES gle)
#find_library                (QGLViewer_LIBRARY          NAMES QGLViewer)
#find_path                   (QGLViewer_INCLUDE_DIR      QGLViewer/qglviewer.h)
#include_directories         (${QGLViewer_INCLUDE_DIR})

add_executable              (extract-diagram                extract-diagram.cpp)
target_link_libraries       (extract-diagram                ${libraries} ${Boost_SERIALIZATION_LIBRARY})

add_subdirectory            (diagram-viewer)
add_subdirectory            (matching)
