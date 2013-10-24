INCLUDEPATH += -I D:\opencv246\build\include \
-I D:\youyou_program\ASM\stasm4.0.0\stasm \
-I D:\glm

LIBS += -LD:\opencv246\mingw\bin \
-lopencv_core246 \
-lopencv_highgui246 \
-lopencv_imgproc246 \
-lopencv_objdetect246 \
-LD:\youyou_program\ASM\stasm4.0.0\build\mingw48_32\bin \
-lstasm_lib_proj

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
    Patch.h \
    FacialShape/symmetryquad.h \
    FacialShape/featurequads.h \
    FacialShape/facialshape.h \
    customdialog.h

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
    Patch.cpp \
    FacialShape/symmetryquad.cpp \
    FacialShape/featurequads.cpp \
    FacialShape/facialshape.cpp \
    customdialog.cpp

QT           += opengl widgets
