#include "meshshape.h"
#include "curve.h"

double MeshShape::EXTRUDE_T = 0.25;
MeshShape::OPERATION_e MeshShape::_OPMODE = MeshShape::EXTRUDE_EDGE;
bool MeshShape::isSMOOTH = true;

MeshShape::MeshShape(Mesh_p control)
{
    _control = control;
    if (!_control){ 
        //insert a quad face only
        _control = new Mesh();
        _control->setInsertEdgeCB(onInsertEdge);
        _control->setAddFaceCB(onAddFace);
        _control->setCaller((void*)this);
        //generateControlPoints();
    }
}

MeshShape* MeshShape::newMeshShape(const Point&p, PRIMITIVE_e ePrim)
{

    MeshShape* pMesh = new MeshShape();

    switch(ePrim)
    {
        case SQUARE:
        {
            Vertex_p verts[4];
            Point ps[4];
            ps[0].set(-0.1, 0.1);
            ps[1].set(0.1, 0.1);
            ps[2].set(0.1, -0.1);
            ps[3].set(-0.1, -0.1);
            for(int i=0; i<4; i++) verts[i] = pMesh->_control->addVertex(new Point(ps[i]), new Normal(0,0,1));
            pMesh->_control->addQuad(verts[0], verts[1], verts[2], verts[3]);
            pMesh->_control->buildEdges();
        }
        break;

        case GRID:
        {
            pMesh->insertGrid(p, 0.1, 2, 2);
        }
        break;

        case NGON:
        {
            pMesh->insertNGon(p, 2, 0.1);
        }
        break;
    }

    pMesh->update();
    return pMesh;
}

/*
void MeshShape::generateControlPoints(){

    list<Vertex_p> verts = _control->verts();
        for(list<Vertex_p>::iterator it = verts.begin(); it!= verts.end(); it++)
            this->getController()->addControl((*it)->pP);
}
*/

void MeshShape::onClick(const Point & p, Click_e eClick){

    if (eClick == UP){
        execOP(p);
    }
}

//all operations on meshshape needs to be made static to allow operation on all layers
void MeshShape::execOP(const Point &p){

    Selectable_p obj = Selectable::getLastSelected();

    if (!obj || obj->type() != Renderable::SHAPE)
        return;

    Edge_p e = (Edge_p)obj->pRef;
    Face_p f = (Face_p)obj->pRef;

    MeshShape* mesh = 0;

    if (_OPMODE == EXTRUDE_EDGE || _OPMODE == INSERT_SEGMENT){
         mesh = ((MeshShape*)e->mesh()->caller());
    }else if (_OPMODE == EXTRUDE_FACE || _OPMODE == DELETE_FACE){
         mesh = ((MeshShape*)f->mesh()->caller());
    }

    if ( !e && !f)
        return;

    switch(_OPMODE){

        case MeshShape::NONE:
        break;

        case MeshShape::EXTRUDE_EDGE:
            if (e)
                extrude(e, EXTRUDE_T);
        break;

        case MeshShape::INSERT_SEGMENT:
            if (e)
                insertSegment(e, p);
        break;

        case MeshShape::EXTRUDE_FACE:
            if (f)
                extrude(f, EXTRUDE_T);
        break;

        case MeshShape::DELETE_FACE:
            if (f)
                deleteFace(f);
        break;

    }

    if (mesh)
        mesh->update();

}

void MeshShape::insertSegment(Edge_p e, const Point & p){

    if (!e || !e->isBorder())
        return;

    Mesh_p pMesh = e->mesh();

    double t = 0.5;
    //((Curve*)e->curve)->computeDistance(p, t);

    Curve* curve = e->curve;
    Corner* c0 = pMesh->splitEdge(e, pMesh->addVertex(new Point(), new Normal(0,0,1) ) );
    onSplitEdge(c0, t);

    Corner* c1 = c0->vNext();
    Face_p endf = (c1)?c1->F():0;

    while(c0 && c0->F()!=endf){

        Corner* c01 = pMesh->splitEdge(c0->next()->next()->E(),  pMesh->addVertex(new Point(), new Normal() ) ,c0->F());
        onSplitEdge(c01, (1-t));
        Corner* c0n = c01->vNext();
        pMesh->insertEdge(c0, c01);
        c0 = c0n;
    }

    if (c0 && c0->F() == endf)
        (pMesh->insertEdge(c0, c0->next()->next()->next()));
    else while(c1){
        Corner* c11 = pMesh->splitEdge(c1->next()->next()->E(),  pMesh->addVertex(new Point(), new Normal() ), c1->F());
        onSplitEdge(c11, 1-t);
        Corner* c1n = c11->vNext();
         pMesh->insertEdge(c1, c11);
        //onInsertEdge(CEdge::insert(c1, c11));
        c1 = c1n;
    }

}

void MeshShape::diagonalDivide(Corner_p c){
    if (!c)
        return;
    Mesh_p pMesh = c->V()->mesh();
    Edge_p e = pMesh->insertEdge(c, c->next()->next(), true);
    //c = e->split();
    /*e->C0()->F()->CFace::update(true);
    e->C1()->F()->CFace::update(true);*/
    //ControlPoint::create(this, c->V()->pP);
}

Face_p MeshShape::extrude(Face_p f0, double t){

    Mesh_p pMesh = f0->mesh();

    f0->update();
    Face_p f1 = pMesh->addFace(f0->size());

    Point pmid;//init as centroid of P

    for(int i=0; i<f0->size(); i++){
        Point p = P(f0->C(i)) * (1-t) + pmid * t;
        f1->set(pMesh->addVertex(new Point(p), new Normal(0,0,1)), i);
    }

    Edge_p e3 = 0;
    Face_p f_side_0 = 0;
    for(int i=0; i<f1->size(); i++){
        Face_p f_side =  pMesh->addFace(4);

        //set verts
        f_side->set(f0->C(i)->V(), 0);
        f_side->set(f0->C(i+1)->V(), 1);
        f_side->set(f1->C(i+1)->V(), 2);
        f_side->set(f1->C(i)->V(), 3);
        f_side->Face::update();

        f0->C(i)->E()->set(f_side->C(0), f0->C(i)->isC0()?0:1);

        Edge_p e1 = pMesh->addEdge(f_side->C(1),0);
        Edge_p e2 = pMesh->addEdge(f1->C(i), f_side->C(2));

        if (e3)
            e3->set(f_side->C(3),1);
        else
            f_side_0  = f_side;

        e3 = e1;
    }

    e3->set(f_side_0->C(3),1);
    f1->update();

    pMesh->remove(f0);
    //*f0 = *f_side_0; //replace 1st side face with the old face
    return f1;
}

Edge_p MeshShape::extrude(Edge_p e0, double t){

    if (!e0 || !e0->isBorder())
        return 0;

    Mesh_p pMesh = e0->mesh();

    Face_p f = pMesh->addFace(4);

    //set verts
    Vec2 n = ( Vec3(0,0,1) % Vec3(P1(e0) - P0(e0)) ).normalize()*t;
    Vertex_p v0 = pMesh->addVertex( new Point(P0(e0)+n) , new Normal(0,0,1));
    Vertex_p v1 = pMesh->addVertex( new Point(P1(e0)+n) , new Normal(0,0,1));

    f->set(e0->C0()->next()->V(), 0);
    f->set(e0->C0()->V(), 1);
    f->set(v0, 2);
    f->set(v1, 3);

    //insert edges
    e0->set(f->C(0));
    pMesh->addEdge(f->C(1), 0); //e1
    Edge_p e2 = pMesh->addEdge(f->C(2), 0);
    pMesh->addEdge(f->C(3), 0); //e3

    f->Face::update();

    return e2;
}

void MeshShape::deleteFace(Face_p f){

    if (!f)
        return;

    for(int i=0; i<f->size(); i++){
        if (f->C(i)->E()->isBorder() && f->C(i-1)->E()->isBorder()){
            getController()->removeControl(f->C(i)->V()->pP);
            getController()->removeControl(f->C(i)->V()->pN);
        }
        if (f->C(i)->E()->isBorder()){
            getController()->removeControl(f->C(i)->E()->curve->pCV(1));
            getController()->removeControl(f->C(i)->E()->curve->pCV(2));
            delete f->C(i)->E()->curve;
        }
    }

    f->mesh()->remove(f);
}

void MeshShape::insertGrid(const Point& p, double len, int n, int m){

    m = m?m:n;
    Vertex_p* vs = new Vertex_p[(n+1)*(m+1)];
    for(int j=0; j<m+1; j++)
        for(int i=0; i<n+1; i++)
            vs[i+j*(n+1)] = _control->addVertex(new Point(p+Point(len*i,0)+ Point(0, len*j)), new Normal() );

    for(int j=0; j<m; j++)
        for(int i=0; i<n; i++)
            _control->addQuad(vs[i+j*(n+1)], vs[i+1+j*(n+1)], vs[i+1+(j+1)*(n+1)], vs[i+(j+1)*(n+1)] );
    delete vs;
    _control->buildEdges();

}

void MeshShape::insertNGon(const Point& p, int n, double rad){

    int nn = 2*n;

    Vertex_p vmid = _control->addVertex(new Point(p), new Normal(1,0,0));
    Vertex_p* vs = new Vertex_p[nn];

    for(int i=0; i<nn; i++){
        double ang = -i*PI/n;
        vs[i] = _control->addVertex( new Point(p + Point(1,0)*rad*cos(ang) + Point(0,1)*rad*sin(ang)), new Normal() );
    }

    for(int i=0; i<n; i++)
        _control->addQuad(vmid, vs[i*2], vs[i*2+1], vs[(i*2+2)%nn]);

    delete vs;

    _control->buildEdges();
}

ControlPoint_p MeshShape::addControl(Point_p pP, ControlPoint_p pParent){
    if (pParent)
           return getController()->addControlTangent(pParent, pP);

    return getController()->addControl(pP);
}

ControlPoint_p MeshShape::findControl(Point_p pP){
    return getController()->findControl(pP);
}

ControlNormal_p MeshShape::findControl(Normal_p pN){
    return getController()->findControl(pN);
}

ControlPoint_p MeshShape::addControl(Vertex_p pV){
    ControlPoint_p cp = getController()->addControl(pV->pP);
    pV->pData = (void*) cp;
    cp->pV = pV;
    getController()->addControlNormal(cp, pV->pN);
    return cp;
}

void MeshShape::setOPMODE(OPERATION_e eMode){
    _OPMODE = eMode;
}

Point_p getTanP(Edge_p pE, Corner_p pC)
{
    if (!pE->curve)
        return 0;

    bool isForward = (pE->curve->pCV(0) == pE->C0()->V()->pP);

    if (pE->C0() == pC){
        return pE->curve->pCV( isForward?1:2);
    }else if (pE->C0()->next() == pC){
        return pE->curve->pCV( isForward?2:1);
    }

    return 0;
}
