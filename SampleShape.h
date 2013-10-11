#ifndef SAMPLESHAPE_H
#define SAMPLESHAPE_H

#include "Shape.h"

class SampleShape: public Shape{

    Point _p[4];

public:

    SampleShape():Shape(){
        _p[0].set(-0.1, 0.1);
        _p[1].set(0.1, 0.1);
        _p[2].set(0.1, -0.1);
        _p[3].set(-0.1, -0.1);

        for(int i=0; i<4; i++)
            getController()->addControl(&_p[i]);
    }

    void render() const;
};

#endif // SAMOLESHAPE_H
