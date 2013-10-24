#ifndef CONTROLPOINT_H
#define CONTROLPOINT_H

#include "Base.h"

#define NORMAL_RAD 0.04

class ControlPoint;
class ControlNormal;
class ControlTangent;

typedef ControlPoint* ControlPoint_p;
typedef std::list<ControlPoint_p> CPList;

typedef ControlNormal* ControlNormal_p;
typedef std::list<ControlNormal_p> CNList;

typedef ControlTangent* ControlTangent_p;

class ControlPoint: public Draggable{

    static ControlPoint_p _pTheActive;

protected:

    Selectable_p _pControlled;

    void onDown(){
        if (isChild())
            _pTheActive = (ControlPoint_p)parent();
        else
            _pTheActive = this;
    }

    virtual void onDrag(const Point& t){ // move the children
        if (isParent()){
            DraggableList childs = getChilds();
            for(DraggableList::iterator it = childs.begin(); it!= childs.end(); it++)
                (*it)->pP()->set((*it)->P() + t);
        }

        if (_pControlled)
            _pControlled->update();
    }

public:

    dlfl::Vertex_p pV;
    bool isMarked;

    ControlPoint(Point_p pP, Selectable_p pControlled):Draggable(Renderable::UI, pP), _pControlled(pControlled){
        pV = 0;
        isMarked = false;
    }
    void render() const;

    bool isActive() const {return (_pTheActive == this) || (_pTheActive == this->parent());}
};


class ControlNormal:public ControlPoint{

    Normal_p _pN;

protected:

    void onDrag(const Vec2& t){

        Vec2 v = (P() - parent()->P());
        double l = v.norm();
        Vec2 vn = v.normalize();
        if ( l > NORMAL_RAD ){
            pP()->set(parent()->P() + vn*NORMAL_RAD);
            v = vn*NORMAL_RAD;
            l = NORMAL_RAD;
        }

        double h = sqrt(NORMAL_RAD*NORMAL_RAD - l*l);
        if (h < 0)
            h = 0;

        _pN->set(Vec3(v.x, v.y, h).normalize());
        ControlPoint::onDrag(t);
    }

    void onUpdate(){
        pP()->set( parent()->P() + Vec2(_pN->x, _pN->y) );
    }

public:

    ControlNormal(ControlPoint_p pParent, Normal_p pN, Selectable_p pControlled):ControlPoint(new Point(), pControlled){
        pV  =   0;
        _pN =   pN;
        pParent->adopt(this);
        pV = pParent->pV;
        pP()->set(parent()->P() + Vec2(pN->x, pN->y));
    }

    void render() const;

    void updatePos(){
         pP()->set( parent()->P() + Vec2(_pN->x, _pN->y)*NORMAL_RAD );
    }

    inline Normal_p pN()const{return _pN;}

};


class ControlTangent: public ControlPoint{


    ControlTangent_p _pPair;

protected:

   void onDrag(const Vec2& t){ // move the children

        if (_pPair){

            Vec2 tan  = (parent()->P() - P()).normalize();
           _pPair->pP()->set(parent()->P() + tan*(_pPair->P() - parent()->P()).norm());
           ControlNormal_p cn = (ControlNormal_p)parent()->getChilds().front(); //not the best way to get the normal controller
           Vec2 n2d(cn->pN()->x, cn->pN()->y);

           Vec2 tan0 = (parent()->P() - P() + t).normalize();
           Vec2 y_ax0 = Vec3(0,0,1)%Vec3(tan0.x, tan0.y, 0);
           Vec2 y_ax1 = Vec3(0,0,1)%Vec3(tan.x, tan.y, 0);

           Vec2 n2d_1 = tan*(tan0*n2d) + y_ax1*(y_ax0*n2d);
           Vec3 n3d(n2d_1.x, n2d_1.y, cn->pN()->z);
           n3d = n3d.normalize();

           cn->pN()->set(n3d);
           cn->updatePos();
        }

        if (_pControlled)
            _pControlled->update();
    }

public:

   ControlTangent(ControlPoint_p pParent, Point_p pP, Selectable_p pControlled):ControlPoint(pP, pControlled){
       pV = 0;
       pParent->adopt(this);
       isMarked = false;
       _pPair = 0;
   }

   ~ControlTangent(){

       setPair();

   }

   void setPair(ControlTangent_p pair = 0){

       if (pair)
           pair->_pPair = this;
       else if (_pPair)
           _pPair->_pPair = 0; //break

       _pPair = pair;
   }

   void setTangent(const Vec2 &tan, bool bothpairs=false){
       pP()->set(parent()->P()+tan);
       if (bothpairs && _pPair){
           _pPair->pP()->set(parent()->P()-tan);
       }
   }

   Vec2 getTangent() const { return P() - parent()->P();}
};


typedef class UIController: public Renderable{

    CPList _controlPs;
    CNList _controlNs;
    Selectable_p _pControlled;

public:

    UIController(Selectable_p pControlled):Renderable(UI),_pControlled(pControlled){
    }

    void render() const;

    ControlPoint_p addControl(Point_p p){
        ControlPoint_p cp = new ControlPoint(p, _pControlled);
        _controlPs.push_back(cp);
        return cp;
    }

    ControlTangent_p addControlTangent(ControlPoint_p pParent, Point_p pP){
        ControlTangent_p cp = new ControlTangent(pParent, pP, _pControlled);
        _controlPs.push_back(cp);
        return cp;
    }

    ControlPoint_p addControlNormal(ControlPoint_p pParent, Normal_p pN){
        ControlNormal_p cn = new ControlNormal(pParent, pN, _pControlled);
        _controlNs.push_back(cn);
        return cn;
    }

    void removeControl(ControlPoint_p cp){
        _controlPs.remove(cp);
        _controlNs.remove((ControlNormal_p)cp);
        delete cp;
    }

    void removeControl(Point_p pP){
        CPList::iterator it = _controlPs.begin();
        for(;it!=_controlPs.end() && (*it)->pP() != pP; it++);
        if (it!=_controlPs.end()){
            ControlPoint_p cp =*it;

            //remove kids first
            DraggableList childs = cp->getChilds();
            for(DraggableList::iterator it = childs.begin(); it!= childs.end(); it++)
                removeControl((ControlPoint_p)*it);

            _controlPs.erase(it);
            delete cp;
        }
    }

    void removeControl(Normal_p pN){
        CNList::iterator it = _controlNs.begin();
        for(;it!=_controlNs.end() && (*it)->pN() != pN; it++);
        if (it!=_controlNs.end()){
            ControlNormal_p cn = *it;
            _controlNs.erase(it);
            delete cn;
        }
    }

    ControlPoint_p findControl(Point_p pP){
        CPList::const_iterator it = _controlPs.begin();
        for(;it!=_controlPs.end() && (*it)->pP() != pP; it++);
        if (it!=_controlPs.end())
            return *it;
        return 0;
    }

    ControlNormal_p findControl(Normal_p pN){
        CNList::const_iterator it = _controlNs.begin();
        for(;it!=_controlNs.end() && (*it)->pN() != pN; it++);
        if (it!=_controlNs.end())
            return *it;
        return 0;
    }

    bool doesExist(ControlPoint_p cp) const {
        CPList::const_iterator it = _controlPs.begin();
        for(;it!=_controlPs.end() && (*it)!= cp; it++);
        if (it!=_controlPs.end())
            return true;
        return false;
    }

    CPList getControls() const {return _controlPs;}

}* UIController_p;

#endif // CONTROLPOINT_H
