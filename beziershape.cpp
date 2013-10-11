#include "meshshape.h"
#include "curve.h"
#include "Patch.h"

void MeshShape::onUpdate(){

    EdgeList edges = _control->edges();
    FOR_ALL_CONST_ITEMS(EdgeList, edges){
        if ((*it)->curve)
            (*it)->curve->update();
    }

    FaceList faces = _control->faces();
    FOR_ALL_CONST_ITEMS(FaceList, faces){
        if ((*it)->surface)
            (*it)->surface->update();
    }

    Selectable_p sel = Selectable::getTheSelected();

    /*if (getController()->doesExist((ControlPoint_p)sel)){
       //this is a control point
        ControlPoint_p cp = (ControlPoint_p)sel;

        Corner_p corn = cp->pV ? cp->pV->C() : ((ControlPoint_p)cp->parent())->pV->C();
        Corner_p corn0 = corn;
        while(corn){

            corn->E()->curve->outdate();
            corn->prev()->E()->curve->outdate();

            corn->E()->curve->update();
            corn->prev()->E()->curve->update();
            corn = corn->vNext();
            if (corn == corn0)
                break
        }
    }*/
}

void MeshShape::fixCurve(Edge_p e){

    if (!e->curve)
        return;


}

Bezier* initCurve(Edge_p e){

    if (e->curve)
        return e->curve;

    Vertex_p v0 = e->C0()->V();
    Vertex_p v1 = e->C0()->next()->V();
    Point p0 = *v0->pP;
    Point p1 = *v1->pP;
    Vec2 tan = p1 - p0;

    MeshShape* shape = (MeshShape*) e->mesh()->caller();

    Point_p p0_t = new Point(p0 + tan*0.25);
    Point_p p1_t = new Point(p1 - tan*0.25);

    Bezier* c = new Bezier(100);
    e->curve = c;
    c->insert(v0->pP);
    c->insert(p0_t);
    c->insert(p1_t);
    c->insert(v1->pP);

    ControlPoint_p cp0 = (ControlPoint_p)v0->pData;
    if (!cp0)
        cp0 = shape->addControl(v0);

    ControlPoint_p cp0_t = shape->addControl(p0_t);
    cp0->adopt(cp0_t);


    ControlPoint_p cp1 = (ControlPoint_p)v1->pData;;
    if (!cp1)
        cp1 = shape->addControl(v1);

    ControlPoint_p cp1_t = shape->addControl(p1_t);
    cp1->adopt(cp1_t);

    c->pRef = (void*) e;
    return c;
}

void onInsertEdge(Edge_p e){
    initCurve(e);
}

void onAddFace(Face_p pF)
{
    pF->surface = (Patch*)new Patch4(pF);
}

void MeshShape::onSplitEdge(Corner_p c,double t)
{

    c->F()->Face::update(true);
    if (c->other())
        c->other()->F()->Face::update(true);


    bool isForward = (c->prev()->V()->pP == c->prev()->E()->curve->pCV(0));

    Edge_p e0 = (isForward)? c->prev()->E() : c->E();

    Bezier* curve0 = e0->curve;

    Point newCP[7];
    curve0->calculateDivideCV(t, newCP);

    ControlPoint_p cptan    = getController()->findControl(e0->curve->pCV(isForward?2:1));
    ControlPoint_p cp       = getController()->findControl(c->V()->pP);

    if (!cp || !cptan) // throw exception
        return;

    cptan->unparent();
    cp->adopt(cptan);

    //parenting fixed
    //now adjust control p's

    curve0->set(c->V()->pP, isForward?3:0);

    curve0->pCV(isForward?1:2)->set(newCP[isForward?1:6]);
    curve0->pCV(isForward?2:1)->set(newCP[isForward?2:5]);
    curve0->pCV(isForward?3:0)->set(newCP[isForward?3:4]);

    Bezier* curve1 = c->E()->curve;

    curve1->pCV(isForward?1:2)->set(newCP[isForward?4:3]);
    curve1->pCV(isForward?2:1)->set(newCP[isForward?5:2]);

    c->V()->pN->set(0,0,1); //fix

    ControlNormal_p cpnorm = getController()->findControl(c->V()->pN);
    if (cpnorm){
        cpnorm->pP()->set(*c->V()->pP);
    }

}
