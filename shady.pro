HEADERS       = glwidget.h \
                mainwindow.h \
    vec/Vector.h \
    vec/Utility.h \
    vec/Quaternion.h \
    vec/Matrix.h \
    base.h \
    canvas.h \
    sampleshape.h \
    controlpoint.h \
    curve.h \
    shape.h \
    meshshape/patch.h \
    meshshape/spineshape.h \
    meshshape/meshshape.h \
    meshshape/meshdata.h \
    meshshape/cMesh.h

SOURCES       = glwidget.cpp \
                main.cpp \
                mainwindow.cpp \
    vec/Vector.cpp \
    vec/Utility.cpp \
    vec/Quaternion.cpp \
    vec/Matrix.cpp \
    render.cpp \
    curve.cpp \
    shape.cpp \
    meshshape/patch.cpp \
    meshshape/spine.cpp \
    meshshape/meshshape.cpp \
    meshshape/cmesh.cpp \
    meshshape/beziershape.cpp \
    meshshape/meshoperations.cpp \
    meshshape/meshprimitives.cpp \
    uicalbacks_meshshape.cpp

QT           += opengl widgets
