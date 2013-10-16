#include "meshshape.h"
#include "curve.h"
#include "Patch.h"

void MeshShape::onUpdate(){

    EdgeList edges = _control->edges();
    FOR_ALL_CONST_ITEMS(EdgeList, edges){
        Edge_p e = (*it);

        if (e->isBorder()){

        }

        if (e->curve)
            e->curve->update();

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

    Vec2 tan0 = p1 - p0;
    Vec2 tan1 = p0 - p1;

    if (MeshShape::isSMOOTH && e->isBorder()){
        tan0 = p1 - (*e->C0()->prev()->V()->pP);
        tan1 = p0 - (*e->C0()->next()->next()->V()->pP);
    }

    MeshShape* shape = (MeshShape*) e->mesh()->caller();

    Point_p p0_t = new Point(p0 + tan0*0.25);
    Point_p p1_t = new Point(p1 + tan1*0.25);

    Bezier* c = new Bezier(100);
    e->curve = c;
    c->insert(v0->pP);
    c->insert(p0_t);
    c->insert(p1_t);
    c->insert(v1->pP);

    ControlPoint_p cp0 = (ControlPoint_p)v0->pData;
    if (!cp0)
        cp0 = shape->addControl(v0);

    ControlTangent_p cp0_tan = (ControlTangent_p)shape->addControl(p0_t, cp0);


    ControlPoint_p cp1 = (ControlPoint_p)v1->pData;;
    if (!cp1)
        cp1 = shape->addControl(v1);

    ControlTangent_p cp1_tan = (ControlTangent_p) shape->addControl(p1_t, cp1);

    if (MeshShape::isSMOOTH && e->isBorder()){

        //cp0_tan->setPair();
        Edge_p e0 = e->C0()->prev()->E();
        if ( e0 && e0->isBorder() && e0->curve){
            ControlTangent_p pair = (ControlTangent_p)shape->findControl(getTanP(e0, e->C0()));
            cp0_tan->setPair(pair);
            tan0 = tan0.normalize();
            e->C0()->V()->pN->set( Vec3(0,0,1)%Vec3(tan0.x, tan0.y, 0));
            shape->findControl(e->C0()->V()->pN)->updatePos();
        }

        Edge_p e1 = e->C0()->next()->E();
        if ( e1 && e1->isBorder() && e1->curve){
            ControlTangent_p pair = (ControlTangent_p)shape->findControl(getTanP(e1, e->C0()->next()));
            cp1_tan->setPair(pair);
            tan1 = -tan1.normalize();
            e->C0()->next()->V()->pN->set( Vec3(0,0,1)%Vec3(tan1.x, tan1.y, 0));
            shape->findControl(e->C0()->next()->V()->pN)->updatePos();
        }
    }

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

void MeshShape::onSplitEdge(Corner_p c, double t)
{
    c = c->isC0()? c : c->vNext();

    c->F()->Face::update(true);
    if (c->other())
        c->other()->F()->Face::update(true);

    Edge_p e0 = c->prev()->E();
    Bezier* curve0 = e0->curve;

    bool isForward = (c->prev()->V()->pP == curve0->pCV(0));


    Point newCP[7];
    curve0->calculateDivideCV(t, newCP);

    ControlPoint_p cptan    = getController()->findControl(e0->curve->pCV(isForward?2:1));
    ControlPoint_p cp       = getController()->findControl(c->V()->pP);

    if (!cp || !cptan){ // throw exception
        return;
    }

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
