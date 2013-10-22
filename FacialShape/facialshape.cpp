#include "facialshape.h"
#include "Canvas.h"


FacialShape::FacialShape()
{
    SymmetryQuad mSymQuad;
    mSymQuad.AssignNormalColor();
    m_imgName = mSymQuad.GetFileName();
    vector<vector<int> > gridInd = mSymQuad.CreatGridIndices();
    std::vector<std::vector<Point *> > PointsGrid = CvtGrid(mSymQuad.GetVerticesVector(), gridInd, (float)mSymQuad.m_width, (float)mSymQuad.m_height);
    std::vector<std::vector<Normal *> > ColorGrid = CvtGrid(mSymQuad.GetColorVector(), gridInd);
    std::vector<std::vector<Vertex *> > VerticesGrid;

    //insert quad faces only
    _control = new Mesh();
    _control->setInsertEdgeCB(onInsertEdge);
    _control->setAddFaceCB(onAddFace);
    _control->setCaller((void*)this);

    VerticesGrid.resize(PointsGrid.size());
    for(int i=0;i<PointsGrid.size();i++)
    {
        VerticesGrid[i].resize(PointsGrid[i].size());
        for(int j=0;j<PointsGrid[i].size();j++)
        {
            VerticesGrid[i][j] = _control->addVertex(PointsGrid[i][j],ColorGrid[i][j]);
        }
    }

    for(int i=0;i<VerticesGrid.size()-1;i++)
    {
        for(int j=0;j<VerticesGrid[i].size()-1;j++)
            _control->addQuad(VerticesGrid[i][j], VerticesGrid[i][j+1], VerticesGrid[i+1][j+1], VerticesGrid[i+1][j]);
    }

    MeshShape::isSMOOTH = false;
    _control->buildEdges();

    EdgeList edges = _control->edges();
    FOR_ALL_CONST_ITEMS(EdgeList, edges){
        Edge_p pE = (*it);
        Point_p tan0 = getTanP(pE, pE->C0());
        Point_p tan1 = 0;
        Corner_p vprev = pE->C0()->vPrev();
        Corner_p vnext = pE->C0()->vNext();
        VertexData* p0 = pE->C0()->next()->V()->pP;
        VertexData* p1 = 0;
        if (vprev){
            tan1 = getTanP(vprev->prev()->E(), vprev);
            p1 = vprev->prev()->V()->pP;
        }else if(vnext){
            tan1 = 0;
            p1 = pE->C0()->V()->pP;
            //tan1 = getTanP(pE->C0()->prev()->E(), vprev);
        }else{
            tan1 = getTanP(pE->C0()->prev()->E(), pE->C0());
            p1 = pE->C0()->prev()->V()->pP;
        }

        ControlTangent_p ct0 = (ControlTangent_p)findControl(tan0);
        if (tan1){
            ControlTangent_p ct1 = (ControlTangent_p)findControl(tan1);
            ct0->setPair(ct1);
        }
        Vec2 tan_d= (*p0-*p1);
        ct0->setTangent(tan_d,true);
    }

    FOR_ALL_CONST_ITEMS(EdgeList, edges){
        Edge_p pE = (*it);
        Point_p tan0 = getTanP(pE, pE->C0());
        Point_p tan1 = getTanP(pE, pE->C0()->next());
        double length = (*(pE->C0()->V()->pP) - *(pE->C0()->next()->V()->pP)).norm();
        ControlTangent_p ct0 = (ControlTangent_p)findControl(tan0);
        ControlTangent_p ct1 = (ControlTangent_p)findControl(tan1);
        Vec2 tan_d0 = ct0->getTangent();
        ct0->setTangent(tan_d0.normalize()*length/3);
        Vec2 tan_d1 = ct1->getTangent();
        ct1->setTangent(tan_d1.normalize()*length/3);
    }
//    initBG();
    Canvas::get()->setImagePlane(mSymQuad.GetFileName());
    /*
    //Set Tangent
    for(int i=0;i<PointsGrid.size();i++)
    {
        VerticesGrid[i].resize(PointsGrid[i].size());
        for(int j=0;j<PointsGrid[i].size();j++)
        {
            ControlTangent *P_control = (ControlTangent *)findControl(PointsGrid[i][j]);
//            Vec2 tan;
//            Point p1,p2;
//            if(i-1<0)
//                p1 =
            if(P_control!=NULL)
                P_control->setTangent(Vec2(0,0));
        }
    }*/

//    int num_v, num_q;
//    float *vertices;
//    float *colors = NULL;
//    int *indices;
//    mSymQuad.GetVertices(&vertices, num_v);
//    mSymQuad.GetIndices(&indices, num_q);
//    mSymQuad.GetColors(&colors, num_v);

//    //insert quad faces only
//    _control = new Mesh();
//    _control->setInsertEdgeCB(onInsertEdge);
//    _control->setAddFaceCB(onAddFace);
//    _control->setCaller((void*)this);

//    Vertex_p *verts = new Vertex_p[num_v];
//    for(int i=0;i<num_v;i++)
//    {
//        Normal *temp = new Normal(2*colors[3*i]-1,2*colors[3*i+1]-1,2*colors[3*i+2]-1);
//        temp->normalize();
//        verts[i] = _control->addVertex(new Point(2*vertices[2*i]/(float)mSymQuad.m_width-1,-2*vertices[2*i+1]/(float)mSymQuad.m_height+1), temp);
//    }

//    for(int i=0; i<num_q; i++)
//        _control->addQuad(verts[indices[i*4]], verts[indices[i*4+1]], verts[indices[i*4+2]], verts[indices[i*4+3]]);
//    _control->buildEdges();

    update();

}

std::vector<std::vector<Point *> > FacialShape::CvtGrid(const std::vector<cv::Point2d> &vertices, const std::vector<std::vector<int> > &grid_ind, float w, float h)
{
    std::vector<std::vector<Point *> > VGrid;
    VGrid.resize(grid_ind.size());
    for(int i=0;i<grid_ind.size();i++)
    {
        VGrid[i].resize(grid_ind[i].size());
        for(int j=0;j<grid_ind[i].size();j++)
            VGrid[i][j] = new Point(vertices[grid_ind[i][j]].x/w*2-1,-vertices[grid_ind[i][j]].y/h*2+1);
    }

    return VGrid;
}

std::vector<std::vector<Normal *> > FacialShape::CvtGrid(const std::vector<cv::Vec3f> &colors, const std::vector<std::vector<int> > &grid_ind)
{
    std::vector<std::vector<Normal *> > CGrid;
    CGrid.resize(grid_ind.size());
    for(int i=0;i<grid_ind.size();i++)
    {
        CGrid[i].resize(grid_ind[i].size());
        for(int j=0;j<grid_ind[i].size();j++)
        {
            CGrid[i][j] = new Normal(2*colors[grid_ind[i][j]][0]-1,2*colors[grid_ind[i][j]][1]-1,2*colors[grid_ind[i][j]][2]-1);
            CGrid[i][j]->normalize();
        }
    }

    return CGrid;

}
