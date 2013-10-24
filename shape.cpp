#include "shape.h"

//ShapeVertex///////////////////////////////////////////////////////////////////////////
int ShapeVertex::_COUNT = 0;
SVMap ShapeVertex::_svmap;

ShapeVertex::ShapeVertex(Shape_p pS){
    _pShape = pS;
    N.set(0,0,1);
    flag = 0x00;
    _id = _COUNT;
    _COUNT++;
    _parent = _pair = 0;
    pRef = 0;
    _svmap[_id] = this;
    isPositionControl = isNormalControl = true;
}

void ShapeVertex::adopt(ShapeVertex_p sv){
    if (sv->_parent)
        sv->_parent->_childs.remove(sv);
    sv->_parent = this;
    _childs.push_back(sv);
}

void ShapeVertex::setPair(ShapeVertex_p sv,bool isSetTangent,  bool isSetNormal){
    if (!sv || sv->_parent != _parent)
        return;

    if (sv->_pair)
        sv->_pair->_pair = 0; //unpair
    sv->_pair = this;
    _pair = sv;

    Vec2 tan = (P - _pair->P);
    if (isSetNormal){
        _parent->N = Vec3(0,0,1)%Vec3(tan).normalize();
    }
    if (isSetTangent){
        P.set(_parent->P + tan*0.5);
        _pair->P.set(_parent->P - tan*0.5);
    }
}

void ShapeVertex::unpair(){
    if (_pair)
        _pair->_pair =0;
    _pair =0;
}

ShapeVertex_p ShapeVertex::get(int id){
    SVMap::const_iterator it = _svmap.find(id);
    if (it == _svmap.end())
        return 0;
    return it->second;
}

ShapeVertex::~ShapeVertex(){
    _svmap.erase(_id);
    if (_childs.size()){
        FOR_ALL_ITEMS(SVList, _childs)
           (*it)->_parent = 0;
    }
    if (_parent)
        _parent->_childs.remove(this);
    if (_pair)
        _pair->_pair =0;
}

void ShapeVertex::drag(const Vec2 &t, bool isNormal){

    if (isNormal){

        Vec2 v = (Vec2(N.x*NORMAL_RAD, N.y*NORMAL_RAD) + t);
        double l = v.norm();
        if ( l > NORMAL_RAD ){
            v = v.normalize()*NORMAL_RAD;
            l = NORMAL_RAD;
        }
        double h = sqrt(NORMAL_RAD*NORMAL_RAD - l*l);
        if (h < 0)
            h = 0;
        N.set(Vec3(v.x, v.y, h).normalize());
        _pShape->update(this);
        return;
    }

    P = P + t;

    if (_pair){

        Vec2 tan  = (_parent->P - P).normalize();
       _pair->P.set(_parent->P + tan*(_pair->P - _parent->P).norm());

       //now rotate the normal
       Vec2 n2d(_parent->N);
       Vec2 tan0 = (_parent->P - P + t).normalize();
       Vec2 y_ax0 = Vec3(0,0,1)%Vec3(tan0.x, tan0.y, 0);
       Vec2 y_ax1 = Vec3(0,0,1)%Vec3(tan.x, tan.y, 0);

       Vec2 n2d_1 = tan*(tan0*n2d) + y_ax1*(y_ax0*n2d);
       Vec3 n3d(n2d_1.x, n2d_1.y, _parent->N.z);
       _parent->N = n3d.normalize();
    }

    if (hasChilds()){
        FOR_ALL_CONST_ITEMS(SVList, _childs)
            (*it)->P = (*it)->P + t;
    }

    _pShape->update(this);
}



//Shape////////////////////////////////////////////////////////////////////////////////////

ShapeVertex_p Shape::addVertex()
{
    ShapeVertex_p sv = new ShapeVertex(this);
    _vertices.push_back(sv);
    return sv;
}

ShapeVertex_p Shape::addVertex(const Point& p, ShapeVertex_p parent, bool isPositionControl, bool isNormalControl){
    ShapeVertex_p sv = addVertex();
    sv->P = p;
    sv->_parent = parent;
    if (parent){
        parent->adopt(sv);
    }
    sv->isPositionControl  = isPositionControl;
    sv->isNormalControl    = isNormalControl;
    return sv;
}

void Shape::removeVertex(ShapeVertex_p sv){
    _vertices.remove(sv);
    delete sv;
}

void Shape::removeVertex(Point_p pP){
    SVList::iterator it = _vertices.begin();
    for(;it!=_vertices.end() && (*it)->pP() != pP; it++);
    if (it!=_vertices.end()){
        ShapeVertex_p sv =*it;
        _vertices.erase(it);
        delete sv;
    }
}

void Shape::update(ShapeVertex_p sv){
    Renderable::update();
}

Point Shape::gT(){
    if (!parent())
        return _t0;
    return _t0 + ((Shape_p)parent())->gT();
}

void Shape::sendClick(const Point& p, Click_e click){
    Point p0(p);
    p0 = p0 - _t0;
    onClick(p0, click);
}

//ShapeControl///////////////////////////////////////////////////////////////////////////////////
ShapeControl* ShapeControl::_thecontroller = new ShapeControl();
ShapeVertex_p ShapeControl::_theSelected;

void ShapeControl::startSelect(unsigned int svname){
    int svid = svname & ((1<<SV_ID_BIT)-1);
    isNormalControl = svname & (1 << NORMAL_CONTROL_BIT);
    _theSelected = ShapeVertex::get(svid);
}

void ShapeControl::onDrag(const Vec2& t){
    if (_theSelected){
        _theSelected->drag(t, isNormalControl);
    }
}



