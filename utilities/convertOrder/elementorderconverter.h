#ifndef ELEMENTORDERCONVERTER_H
#define ELEMENTORDERCONVERTER_H

class sbfMesh;

class ElementOrderConverter
{
public:
    ElementOrderConverter();

    sbfMesh *convert ( const sbfMesh *originalMesh,
                       int targetOrder,
                       bool verbouse = false,
                       float mergeTolerance = 0.001 );

};

#endif // ELEMENTORDERCONVERTER_H
