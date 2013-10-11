#include <qgl.h>
#include "Canvas.h"
#include "SampleShape.h"
#include "spineshape.h"
#include "meshshape.h"
#include "ControlPoint.h"
#include "curve.h"
#include "Patch.h"

Canvas* Canvas::_canvas = new Canvas();
int Selectable::_COUNT = 0;
Selectable_p Selectable::_theSelected = 0;
Selectable_p Selectable::_lastSelected = 0;
SelectableMap Selectable::_selectables;
ControlPoint_p ControlPoint::_pTheActive = 0;

Canvas::EditMode_e Canvas::MODE = Canvas::POINT_NORMAL_SHAPE_M;

//selection stuff
bool isInRenderMode(){
    GLint mode;
    glGetIntegerv(GL_RENDER_MODE, &mode);
    return ( mode == GL_RENDER);
}

bool MeshShape::IsSelectMode(SELECTION_e eMode){
    return GetSelectMode() == eMode && !Canvas::get()->isDragMode;
}

void Selectable::renderNamed(bool ispush) const{
    glLoadName(name());
    render();
}

Selectable_p select(GLint hits, GLuint *buff){
   //only one selectable object in the stack for now
   Selectable_p pSel = Selectable::get(buff[3]);
   return pSel;
}

// now all renders here

void Canvas::render() const{

    if (MODE == SHADED_M)
    {
        _lights[0]->renderNamed();
    }
    FOR_ALL_CONST_ITEMS(ShapeList, _shapes){ //need to add layers
        Shape_p s = *it;
        if (!s->isChild())
            render(*it);
    }
}

void Light::render() const {
    glColor3f(1.0, 1.0, 0);
    glPointSize(8);
    glBegin(GL_POINTS);
    glVertex3f(P().x, P().y, 0);
    glEnd();

}

void Canvas::render(Shape_p pShape) const {

    glPushMatrix();

    Point p = pShape->P();
    glTranslatef(p.x, p.y, 0);

    pShape->renderAll();

    if (pShape->isParent()){
        DraggableList childs = pShape->getChilds();
        FOR_ALL_CONST_ITEMS(DraggableList, childs){
            render((Shape_p)*it);
        }
    }

    glPopMatrix();
}

void SampleShape::render() const{

    glColor3f(1.0, 0, 0);
    glBegin(GL_POLYGON);
    for(int i = 0; i < 4; i++)
        glVertex3f(_p[i].x, _p[i].y, 0);
    glEnd();
}

void UIController::render() const {

    if (!Canvas::get()->isDragMode)
        return;

    glColor3f(1.0, 1.0, 1.0);

    if (Canvas::MODE == Canvas::NORMAL_M || Canvas::MODE == Canvas::POINT_NORMAL_M || Canvas::MODE == Canvas::POINT_NORMAL_SHAPE_M )
        FOR_ALL_CONST_ITEMS(CNList, _controlNs)
        {
            (*it)->renderNamed();
        }

    if (Canvas::MODE == Canvas::POINT_M || Canvas::MODE == Canvas::POINT_NORMAL_M || Canvas::MODE == Canvas::POINT_NORMAL_SHAPE_M )
        FOR_ALL_CONST_ITEMS(CPList, _controlPs)
        {
            (*it)->renderNamed();
        }

    //glEnd();
}

void ControlPoint::render() const {

    if (isChild() && !isActive()) return;

    if (isTheSelected()){
        glColor3f(1.0, 0, 0);
    }else{
        glColor3f(1.0, 1.0, 1.0);
    }

    if (isMarked)
         glColor3f(0, 1.0, 0);

    glPointSize(5.0);
    glBegin(GL_POINTS);
    glVertex3f(P().x, P().y, 0);
    glEnd();

    //not selectable

    if (isInRenderMode() && isActive() && isParent()){
        //render lines

         glColor3f(1.0, 1.0, 1.0);
         glLineWidth(1.0);

         DraggableList childs = getChilds();
         glBegin(GL_LINES);
         Point p0 = P();
         FOR_ALL_CONST_ITEMS(DraggableList, childs){
             Point p1 = (*it)->P();
             glVertex3f(p0.x, p0.y, 0);
             glVertex3f(p1.x, p1.y, 0);
         }
         glEnd();
    }
}


void ControlNormal::render() const {

    if ( (Canvas::MODE == Canvas::POINT_NORMAL_M || Canvas::MODE ==Canvas::POINT_NORMAL_SHAPE_M) && !isActive() )
        return;

    glColor3f(1.0, 0, 0);
    glPointSize(7.0);
    glBegin(GL_POINTS);
    glVertex3f(P().x, P().y, 0);
    glEnd();
}

void SpineShape::render() const{

    glBegin(GL_LINES);
        FOR_ALL_CONST_ITEMS(SVertexList, _verts){
            if (!(*it)->pP)
                continue;

            Point p0 = (*it)->P();
            for(SVertexList::iterator itv = (*it)->links.begin(); itv != (*it)->links.end(); itv++)
            {
                Point p1 = (*itv)->P();
                glVertex3f(p0.x, p0.y, 0);
                glVertex3f(p1.x, p1.y, 0);
            }
        }
    glEnd();
}

void MeshShape::render() const {

    if ( Canvas::MODE!=Canvas::SHADED_M && (isInRenderMode() || IsSelectMode(EDGE) )){
        EdgeList edges = _control->edges();
        FOR_ALL_CONST_ITEMS(EdgeList, edges){
            render(*it);
        }
    }

    //too messy, fix it!
    if (isInRenderMode() || (IsSelectMode(FACE) || Canvas::get()->canDragShape() ) ){
        FaceList faces = _control->faces();
        FOR_ALL_CONST_ITEMS(FaceList, faces){
            render(*it);
        }
    }
}

void MeshShape::render(Edge_p pEdge) const{

    if (pEdge->curve){
        pEdge->curve->renderNamed();
        return;
    }
    //non selectable line representation
    glColor3f(1.0,1.0, 0);
    Point p0 = P0(pEdge);
    Point p1 = P1(pEdge);

    glBegin(GL_LINES);
    glVertex3f(p0.x, p0.y, 0);
    glVertex3f(p1.x, p1.y, 0);
    glEnd();

}

void MeshShape::render(Face_p pFace) const{
    if (pFace->surface){
        //this is messy, needs better solution
        if (Canvas::get()->canDragShape()){
            pFace->surface->renderUnnamed();
        }else{
            pFace->surface->renderNamed();
        }
        return;
    }
}

void Curve::render() const {

    if (isTheSelected()){
        glColor3f(1.0, 0, 0);
        glLineWidth(2.0);
    }else{
        glColor3f(1.0, 1.0, 1.0);
        glLineWidth(1.0);
    }

    glBegin(_isClosed? GL_LINE_LOOP : GL_LINE_STRIP);
    FOR_ALL_I(_size){
        Point p = this->P(i);
        glVertex3f(p.x, p.y, 0);
    }
    glEnd();
}


void Patch4::render() const{

    for(int j=0; j < Ni; j++){
        for(int i = 0; i< Ni; i++){

            Point p[4];
            p[0] = P(i, j);
            p[1] = P(i+1, j);
            p[2] = P(i+1, j+1);
            p[3] = P(i, j+1);

            Vec3 n[4];
            n[0] = _ns[ind(i, j)];
            n[1] = _ns[ind(i+1, j)];
            n[2] = _ns[ind(i+1, j+1)];
            n[3] = _ns[ind(i, j+1)];

            if (isTheSelected())
            {
                glBegin(GL_POLYGON);
                for(int k=0; k<4; k++){
                    glColor3f(1.0, 0, 0);
                    glVertex3f(p[k].x, p[k].y, 0);
                }
                glEnd();
                continue;
            }

            if (Canvas::MODE == Canvas::NORMAL_M || Canvas::get()->isNormalsOn){
                //drawNormals
                glColor3f(1,1,1);
                glPointSize(4.0);

                for(int j=0; j < NN; j++){
                    Vec3 p0 = _ps[j];
                    Vec3 p1 = p0 + _ns[j]*NORMAL_RAD;
                    glBegin(GL_LINES);
                    glVertex3f(p0.x, p0.y, p0.z);
                    glVertex3f(p1.x, p1.y, p1.z);
                    glEnd();
                    glBegin(GL_POINTS);
                    glVertex3f(p0.x, p0.y, p0.z);
                    glEnd();
                }                
            }

            if (Canvas::get()->isWireframeOn){

                glColor3f(1.0,1.0,1.0);
                glLineWidth(0.5);
                glBegin(GL_LINE_LOOP);
                for(int k=0; k<4; k++)
                    glVertex3f(p[k].x, p[k].y, 0);
                glEnd();

            }

            if (Canvas::MODE == Canvas::SHADED_M || Canvas::get()->isShadingOn ){
                glEnable(GL_LIGHTING);
                Point light0_p = Canvas::get()->lightPos(0);
                GLfloat light0_pf[] = { light0_p.x, light0_p.y, -4.0, 0.0 };
                glLightfv(GL_LIGHT0, GL_POSITION, light0_pf);
            }else
                glDisable(GL_LIGHTING);

            glBegin(GL_POLYGON);
            for(int k=0; k<4; k++){                
                if (Canvas::MODE == Canvas::SHADED_M || Canvas::get()->isShadingOn)
                    glNormal3f(n[k].x, n[k].y, n[k].z );
                else
                    glColor3f((n[k].x+1)/2, (n[k].y+1)/2, n[k].z );

                glVertex3f(p[k].x, p[k].y, 0);
            }
            glEnd();

            glDisable(GL_LIGHTING);

        }
    }
}
