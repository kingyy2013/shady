#ifndef __AUXILIARY_H__	
#define __AUXILIARY_H__
#include <vector>
#include <map>
#include <list>
#include "Vec/Matrix.h"


#define TYPE_BIT 24

#define FOR_ALL_ITEMS(ItemList, items) for(ItemList::iterator it = items.begin(); it != items.end(); it++)
#define FOR_ALL_CONST_ITEMS(ItemList, items) for(ItemList::const_iterator it = items.begin(); it != items.end(); it++)
#define FOR_ALL_I(size) for(int i =0; i < size; i++ )
#define CLAMP(n, min, max) ( (n) < (min) ? (min) : ( ( (n) > (max) ) ? (max) : (n) ) )

class Renderable;
class Selectable;
class Draggable;

typedef void* Void_p;
typedef Renderable* Renderable_p;
typedef Selectable* Selectable_p;
typedef Draggable* Draggable_p;
typedef std::map<int, Selectable_p> SelectableMap;
typedef std::list<Draggable_p> DraggableList;

typedef Vec2    Point;
typedef Vec2*   Point_p;

typedef Vec3    Normal;
typedef Vec3*   Normal_p;

typedef std::list<Point> PointList;
typedef std::list<Point_p> Point_pList;

class Renderable{

protected:

    virtual void onOutdate(){}
    virtual void onUpdate(){}
    virtual void render() const = 0;

public:
    enum Type_e {NONE, UI, SHAPE};

    Renderable(Type_e type):_type(type){_upToDate = false;}
    Type_e type() const {return _type;}

    void renderUpToDate(){
        update();
        render();
    }

    void outdate(){
        _upToDate = false;
        onOutdate();
    }

    void update(){
        //if (_upToDate) return;
        onUpdate();
        _upToDate = true;
    }

    Void_p pRef; //genereic pointer to refering object

private:
    Type_e _type;
    bool _upToDate;
};


class Selectable:public Renderable{

    int _name;
    bool _isDraggable;

protected:

    virtual void onUp(){}
    virtual void onDown(){}

public:

    enum Click_e{UP, DOWN, R_UP, R_DOWN};

    Selectable(Type_e type):Renderable(type){
        _name = (type << TYPE_BIT) | _COUNT;
        _selectables[_name] = this;
        _isDraggable = false;
        _COUNT++;
    }

    ~Selectable(){
        _selectables.erase(_name);
    }

    void renderNamed(bool ispush = false) const;
    void renderUnnamed() const{
        render();
    }

    int name() const{return _name;}
    inline bool isTheSelected() const {return this == _theSelected;}
    inline bool isDraggable() const{return _isDraggable;}
    inline void makeDraggable(){_isDraggable = true;}

    //statics
    static Selectable_p get(int iname){
        SelectableMap::const_iterator it =_selectables.find(iname);
        if (it == _selectables.end())
            return 0;
        return it->second;
    }

    static Selectable_p getTheSelected(){return _theSelected;}
    static Selectable_p getLastSelected(){return _lastSelected;}

    static void startSelect(Selectable_p pObj){
        _theSelected = pObj;
        if (_theSelected)
            _theSelected->onDown();
    }

    static void stopSelect(){
        _lastSelected = _theSelected;
        if (!_theSelected)
            return;
        _theSelected->onUp();
        _theSelected = 0;
    }

    static void reset(){
        _selectables.clear();
        _COUNT = 0;
    }


private:

    static int _COUNT;
    static Selectable_p _theSelected;
    static Selectable_p _lastSelected;
    static SelectableMap _selectables;

};

class Draggable:public Selectable{

    Point_p _pP;

    DraggableList _childs;
    Draggable_p _parent;


protected:

    virtual void onDrag(const Point&){}

public:

    Draggable(Type_e type, Point_p pP = 0):Selectable(type){
        if (pP){
            makeDraggable();
        }
        _pP = pP;
        _parent = 0;
    }

    ~Draggable(){
        unparent();
        for(list<Draggable_p>::iterator it = _childs.begin(); it!= _childs.end(); it++)
            (*it)->_parent = 0;
    }

    inline Point    P() const{return *_pP;}
    inline Point_p  pP() const{return _pP;}

    void drag(const Point& t){
        _pP->set(*_pP + t);
        onDrag(t);
    }

    bool isChild() const {return _parent!=0;}
    bool isParent()const {return _childs.size()>0;}
    Draggable_p parent() const {return _parent;}

    bool adopt(Draggable_p pObj){
        if (pObj->_parent)
            return false;
        _childs.push_back(pObj);
        pObj->_parent = this;
        return true;
    }

    void unparent(){
        if (_parent)
            _parent->_childs.remove(this);
        _parent = 0;
    }

    //statics
    static bool dragTheSelected(const Point& t){

        if (!Selectable::getTheSelected() || !Selectable::getTheSelected()->isDraggable())
            return false;
        ((Draggable_p)Selectable::getTheSelected())->drag(t);
        return true;
    }

    DraggableList getChilds() const {return _childs;}
};

namespace dlfl {

    class Corner;
    class Edge;
    class Vertex;
    class Face;
    class Mesh;

    typedef Corner*     Corner_p;
    typedef Edge*       Edge_p;
    typedef Vertex*     Vertex_p;
    typedef Face*       Face_p;
    typedef Mesh*       Mesh_p;

}

#endif
