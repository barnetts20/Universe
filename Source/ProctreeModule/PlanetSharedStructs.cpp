#include "PlanetSharedStructs.h"

const FCubeTransform FCubeTransform::FaceTransforms[6] = {
    {//X+
        FIntVector(2, 1, 0), // FACE: X+, Y+, Z+
        FIntVector(-1, 1, 1), //WORLD: Z-, Y+, X+
        { EdgeOrientation::RIGHT, EdgeOrientation::RIGHT, EdgeOrientation::LEFT, EdgeOrientation::RIGHT },
        { //Face Transition Table   
            { (uint8)EFaceDirection::Z_POS, {2,3,0,1} }, //LEFT
            { (uint8)EFaceDirection::Z_NEG, {0,1,2,3} }, //RIGHT
            { (uint8)EFaceDirection::Y_NEG, {3,1,2,0} }, //DOWN    
            { (uint8)EFaceDirection::Y_POS, {2,0,3,1} }  //UP
        },
        true //FLIP WINDING
    },

    {//X-
        FIntVector( 2, 1, 0), // FACE: X+, Y+, Z+
        FIntVector( 1,-1,-1), //WORLD: Z+, Y-, X-
        { EdgeOrientation::LEFT, EdgeOrientation::LEFT, EdgeOrientation::LEFT, EdgeOrientation::RIGHT },
        { //Face Transition Table
            { (uint8)EFaceDirection::Z_NEG, {1,0,3,2} },//LEFT
            { (uint8)EFaceDirection::Z_POS, {3,2,1,0} },//RIGHT
            { (uint8)EFaceDirection::Y_POS, {3,1,2,0} },//DOWN
            { (uint8)EFaceDirection::Y_NEG, {2,0,3,1} } //UP
        },
        false //DO NOT FLIP WINDING
    },

    {//Y+
        FIntVector( 0, 2, 1), // FACE: X+, Y+, Z+
        FIntVector(-1,-1, 1), //WORLD: X-, Z-, Y+ 
        { EdgeOrientation::UP, EdgeOrientation::DOWN, EdgeOrientation::DOWN, EdgeOrientation::DOWN },
        { //Face Transition Table
            { (uint8)EFaceDirection::X_POS, {1,3,0,2} },//LEFT
            { (uint8)EFaceDirection::X_NEG, {3,1,2,0} },//RIGHT
            { (uint8)EFaceDirection::Z_POS, {3,2,1,0} },//DOWN
            { (uint8)EFaceDirection::Z_NEG, {2,3,0,1} } //UP
        },
        false //DO NOT FLIP WINDING
    },

    {//Y-    
        FIntVector( 0, 2, 1), // FACE: X+, Y+, Z+
        FIntVector( 1, 1,-1), //WORLD: X+, Z+, Y- 
        { EdgeOrientation::UP, EdgeOrientation::DOWN, EdgeOrientation::UP, EdgeOrientation::UP },
        { //Face Transition Table 
            { (uint8)EFaceDirection::X_NEG, {1,3,0,2} },//LEFT
            { (uint8)EFaceDirection::X_POS, {3,1,2,0} },//RIGHT
            { (uint8)EFaceDirection::Z_NEG, {0,1,2,3} },//DOWN
            { (uint8)EFaceDirection::Z_POS, {1,0,3,2} } //UP
        },
        true //FLIP WINDING        
    },

    {//Z+
        FIntVector( 0, 1, 2), // FACE: X+, Y+, Z+
        FIntVector( 1, 1, 1), //WORLD: X+, Y+, Z+  
        { EdgeOrientation::LEFT, EdgeOrientation::RIGHT, EdgeOrientation::UP, EdgeOrientation::DOWN },
        { //Face Transition Table
            { (uint8)EFaceDirection::X_NEG, {3,2,1,0} },//LEFT
            { (uint8)EFaceDirection::X_POS, {2,3,0,1} },//RIGHT
            { (uint8)EFaceDirection::Y_NEG, {1,0,3,2} },//DOWN
            { (uint8)EFaceDirection::Y_POS, {3,2,1,0} } //UP
        },
        true //FLIP WINDING        
    },

    {//Z-
        FIntVector( 0, 1, 2), // FACE: X+, Y+, Z+
        FIntVector( 1, 1,-1), //WORLD: X-, Y-, Z- 
        { EdgeOrientation::RIGHT, EdgeOrientation::LEFT, EdgeOrientation::DOWN, EdgeOrientation::UP },
        { //Face Transition Table 
            { (uint8)EFaceDirection::X_NEG, {1,0,3,2} },//LEFT
            { (uint8)EFaceDirection::X_POS, {0,1,2,3} },//RIGHT
            { (uint8)EFaceDirection::Y_NEG, {0,1,2,3} },//DOWN
            { (uint8)EFaceDirection::Y_POS, {2,3,0,1} } //UP
        },
        false //DO NOT FLIP WINDING
    }
};