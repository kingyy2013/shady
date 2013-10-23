#ifndef __SHAPE_H__
#define __SHAPE_H__

#include <list>
#include <map>
#include <iostream>
#include <fstream>
#include "base.h"

class Shape;
typedef Shape* Shape_p;
typedef std::list<Shape_p> ShapeList;

struct ShapeVertex;

typedef ShapeVertex* ShapeVertex_p;
typedef std::list<ShapeVertex_p> SVList;
typedef std::map<int, ShapeVertex_p> SVMap;

struct ShapeVertex {

    Point           P;
    Normal          N;
    unsigned long   flag;
    void*           pRef;
    bool            isPositionControl, isNormalControl;

    //enum SVType     {POINT, TANGENT};

    void drag(const Vec2& t, bool isNormal = false);

    void adopt(ShapeVertex_p sv);
    void setPair(ShapeVertex_p sv, bool isSetTangent =false , bool isSetNormal = false);
    void unpair();

    inline Point_p       pP()                 {return &P;}
    inline Normal_p      pN()                 {return &N;}
    inline Shape_p       shape()        const {return _pShape;}

    inline ShapeVertex_p parent()       const {return _parent;}
    inline ShapeVertex_p pair()         const {return _pair;}
    inline int           id()           const {return _id;}
    inline bool          hasChilds()    const {return _childs.size()>0;}
    inline SVList        getChilds()    const {return _childs;}


    void update();
    static ShapeVertex_p get(int id);

private:

    friend class Shape;

    int                 _id;
    Shape_p             _pShape;
    bool                _isPositionControl, _isNormalControl;

    ShapeVertex(Shape_p pS);
    ~ShapeVertex();

    ShapeVertex_p _parent, _pair;
    SVList _childs;

    static int _COUNT;
    static SVMap _svmap;
};

class Shape:public Draggable{

    Point _t0;

    unsigned int _flags;
    SVList _vertices;

protected:
    virtual void onClick(const Point&, Click_e){}

public:

    Shape():Draggable(Renderable::SHAPE, &_t0){
        _flags = 0;
    }

    void renderAll() const;

    //Vertex Handling
    ShapeVertex_p addVertex();
    ShapeVertex_p addVertex(const Point& p, ShapeVertex_p parent = 0, bool isPositionControl = true, bool isNormalControl = true);
    void removeVertex(ShapeVertex_p sv);
    void removeVertex(Point_p pP);
    SVList getVertices() const {return _vertices;}
    void update(ShapeVertex_p sv);

	//send generic command to the shape
    enum Command_e {};
    virtual void exec(Command_e){}

    void sendClick(const Point& p, Click_e click);

    Point gT();
    void resetT(){_t0.set();}
    virtual void frezeT(){}

	//flag operations
    bool is(unsigned int bit){return _flags & (1 << bit);}
    void set(unsigned int bit){_flags |= (1 << bit);}
    void unset(unsigned int bit){_flags &= ~(1 << bit);}

	//save&load
    virtual int load(std::ifstream&){return -1;}
    virtual int save(std::ofstream&){return -1;}

};

#define NORMAL_RAD 0.04
#define NORMAL_CONTROL_BIT 30
#define SV_ID_BIT 16

class ShapeControl: public Draggable{

    Point _t0;
    static ShapeControl*   _thecontroller;

    static ShapeVertex_p   _theSelected;
    bool isNormalControl;

protected:

    void onDrag(const Vec2& t);

public:

    ShapeControl():Draggable(Renderable::UI, &_t0){
        isNormalControl = false;
    }

    void startSelect(unsigned int svname);
    void render() const {}
    void renderControls(const Shape_p) const;

    static ShapeControl* get(){return _thecontroller;}

};

#endif
