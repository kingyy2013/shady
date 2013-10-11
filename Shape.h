#ifndef __SHAPE_H__
#define __SHAPE_H__

#include <list>
#include <iostream>
#include <fstream>
#include "Base.h"
#include "ControlPoint.h"

class Shape;
typedef Shape* Shape_p;
typedef std::list<Shape_p> ShapeList;

class Shape:public Draggable{

    Point _t0;

	unsigned int _flags;
    void (*_shapeChangedHandler)(void*);
    void * _param;

    UIController_p _pController;

protected:
    virtual void onClick(const Point&, Click_e){}
    inline UIController_p getController() const{return _pController;}

public:

    Shape():Draggable(Renderable::SHAPE, &_t0){
        _shapeChangedHandler=0;
        _flags = 0;
        _pController = new UIController(this);
    }

    void renderAll() const {
        _pController->render();
        renderNamed(true);
    }

	//send generic command to the shape
    enum Command_e {};
    virtual void exec(Command_e){}

	virtual void shapeChanged(int type = 0){
		if (_shapeChangedHandler)
			_shapeChangedHandler(_param);
		else 
			onShapeChanged(type);
    }

    virtual void onShapeChanged(int type = 0){}

    void setShapeChangedHandler(void (*handler)(void*), void* param){_shapeChangedHandler = handler; _param = param;}

    Point gT(){
        if (!parent())
			return _t0;
        return _t0 + ((Shape_p)parent())->gT();
    }

    void sendClick(const Point& p, Click_e click){
        Point p0(p);
		p0 = p0 - _t0;
        onClick(p0, click);
	}

    virtual void onActivate(){}
    virtual void onDeactivate(){}

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

#endif
