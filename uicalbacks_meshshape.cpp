#include "mainwindow.h"
#include "glwidget.h"
#include "canvas.h"
#include "MeshShape/meshshape.h"

void MainWindow::selectExtrudeEdge()
{
    MeshShape::setOPMODE(MeshShape::EXTRUDE_EDGE);
    unselectDrag();
}

void MainWindow::selectExtrudeFace()
{
    MeshShape::setOPMODE(MeshShape::EXTRUDE_FACE);
    unselectDrag();
}

void MainWindow::selectDeleteFace()
{
    MeshShape::setOPMODE(MeshShape::DELETE_FACE);
    unselectDrag();
}

void MainWindow::selectInsertSegment()
{
    MeshShape::setOPMODE(MeshShape::INSERT_SEGMENT);
    unselectDrag();
}

void MainWindow::new2NGon()
{
    MeshShape* pMS = MeshShape::insertNGon(Point(0,0), 2, 0.1);
    Canvas::get()->insert(pMS);
    glWidget->updateGL();
}

void MainWindow::newGrid()
{
    MeshShape* pMS = MeshShape::insertGrid(Point(0,0), 0.1, 2, 2);
    Canvas::get()->insert(pMS);
    glWidget->updateGL();
}

void MainWindow::newSpine()
{
    //MeshShape* pM = MeshShape::newMeshShape(Point(0,0),MeshShape::SPINE);
    //Canvas::get()->insert(pM);
}
