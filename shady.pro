HEADERS       = glwidget.h \
                mainwindow.h \
    vec/Vector.h \
    vec/Utility.h \
    vec/Quaternion.h \
    vec/Matrix.h \
    Base.h \
    Shape.h \
    Canvas.h \
    SampleShape.h \
    ControlPoint.h \
    meshshape.h \
    spineshape.h \
    CMesh.h \
    curve.h \
    Patch.h

SOURCES       = glwidget.cpp \
                main.cpp \
                mainwindow.cpp \
    vec/Vector.cpp \
    vec/Utility.cpp \
    vec/Quaternion.cpp \
    vec/Matrix.cpp \
    Render.cpp \
    meshshape.cpp \
    spine.cpp \
    CMesh.cpp \
    curve.cpp \
    beziershape.cpp \
    Patch.cpp

QT           += opengl widgets
