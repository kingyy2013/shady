#ifndef MESHSHAPE_H
#define MESHSHAPE_H

#include "Shape.h"
#include "CMesh.h"

using namespace dlfl;

inline Point P0(Corner_p pC) {return *pC->V()->pP;}
inline Point P0(Edge_p pE)  {return *pE->C0()->V()->pP;}
inline Point P1(Edge_p pE)  {return *pE->C0()->next()->V()->pP;}

class MeshShape : public Shape
{
    Mesh_p _control;
    //void generateControlPoints();

protected:

    //overridden functions
    void render() const;
    void onClick(const Point &, Click_e);
    void onUpdate();
    //void onOutdate();


    void render(Edge_p) const;
    void render(Face_p) const;
    void execOP(const Point&);

public:

    MeshShape(Mesh_p control = 0);

    //ui item????
    ControlPoint_p      addControl(Point_p, ControlPoint_p pParent = 0);
    ControlPoint_p      addControl(Vertex_p);
    ControlPoint_p      findControl(Point_p pP);
    ControlNormal_p     findControl(Normal_p pN);


    //static stuff
    enum OPERATION_e {NONE, EXTRUDE_EDGE, EXTRUDE_FACE, DELETE_FACE, SPLIT_FACE, INSERT_SEGMENT};
    enum SELECTION_e {NOSELECT, EDGE, FACE, CORNER, EDGE_EDGE};
    enum PRIMITIVE_e {SQUARE, GRID, NGON, SPINE};

    static double EXTRUDE_T;
    static void setOPMODE(OPERATION_e eMode);

    static SELECTION_e GetSelectMode(){
        static const SELECTION_e SELECTMODE[] = {NOSELECT, EDGE, FACE, FACE, CORNER, EDGE};
        return SELECTMODE[(int)_OPMODE];
    }

    inline static bool IsSelectMode(SELECTION_e eMode);

    void insertGrid(const Point& p, double len, int n, int m);
    void insertNGon(const Point& p, int n, double rad);

    static MeshShape* newMeshShape(const Point&p, PRIMITIVE_e prim = SQUARE);

    static bool isSMOOTH;

private:

    static OPERATION_e _OPMODE;

    //mesh operations
    static void             insertSegment(Edge_p, const Point&);
    static void             diagonalDivide(Corner_p);
    static Face_p           extrude(Face_p, double);
    static Edge_p           extrude(Edge_p, double);
    static void             deleteFace(Face_p);

    static void             onSplitEdge(Corner_p, double t);
    static void             fixCurve(Edge_p);


};

void onInsertEdge(Edge_p);
void onAddFace(Face_p);


Bezier* initCurve(Edge_p);

Point_p getTanP(Edge_p, Corner_p);


#endif // MESHSHAPE_H
