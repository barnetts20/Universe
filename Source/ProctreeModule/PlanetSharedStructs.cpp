#include "PlanetSharedStructs.h"

const FCubeTransform FCubeTransform::FaceTransforms[6] = {
    {//X+
        FIntVector( 2, 1, 0), // FACE: X+, Y+, Z+
        FIntVector(-1, 1, 1),//WORLD: Z-, Y+, X+
        { EdgeOrientation::RIGHT, EdgeOrientation::RIGHT, EdgeOrientation::LEFT, EdgeOrientation::RIGHT },
        true                 //FLIP WINDING
    },
    {//X-
        FIntVector( 2, 1, 0), // FACE: X+, Y+, Z+
        FIntVector( 1,-1,-1), //WORLD: Z+, Y-, X-
        { EdgeOrientation::LEFT, EdgeOrientation::LEFT, EdgeOrientation::LEFT, EdgeOrientation::RIGHT },
        false                  //DO NOT FLIP WINDING
    },
    {//Y+
        FIntVector( 0, 2, 1), // FACE: X+, Y+, Z+
        FIntVector(-1,-1, 1), //WORLD: X-, Z-, Y+ 
        { EdgeOrientation::UP, EdgeOrientation::DOWN, EdgeOrientation::DOWN, EdgeOrientation::DOWN },
        false                 //DO NOT FLIP WINDING
    },
    {//Y-    
        FIntVector( 0, 2, 1), // FACE: X+, Y+, Z+
        FIntVector( 1, 1,-1), //WORLD: X+, Z+, Y- 
        { EdgeOrientation::UP, EdgeOrientation::DOWN, EdgeOrientation::UP, EdgeOrientation::UP },
        true                  //FLIP WINDING        
    },
    {//Z+
        FIntVector( 0, 1, 2), // FACE: X+, Y+, Z+
        FIntVector( 1, 1, 1), //WORLD: X+, Y+, Z+  
        { EdgeOrientation::LEFT, EdgeOrientation::RIGHT, EdgeOrientation::UP, EdgeOrientation::DOWN },
        true                  //FLIP WINDING        
    },
    {//Z-
        FIntVector( 0, 1, 2), // FACE: X+, Y+, Z+
        FIntVector( 1, 1,-1), //WORLD: X-, Y-, Z- 
        { EdgeOrientation::RIGHT, EdgeOrientation::LEFT, EdgeOrientation::DOWN, EdgeOrientation::UP },
        false                  //DO NOT FLIP WINDING
    }
};

// FaceDirections:  X+ (0), X- (1), Y+ (2), Y- (3), Z+ (4), Z- (5)
// EdgeDirections:  LEFT (0), RIGHT (1), UP (2), DOWN (3)
const FaceTransition FaceTransition::FaceTransitions[6][4] = {
    // X+ (0)
    {   
        {  //LEFT
            (uint8)EFaceDirection::Z_POS,
            {2,3,0,1}
        }, //LEFT

        {  //RIGHT
            (uint8)EFaceDirection::Z_NEG,
            {0,1,2,3}
        }, //RIGHT

        {  //DOWN
            (uint8)EFaceDirection::Y_NEG,
            {3,1,2,0}
        }, //DOWN    

        {  //UP
            (uint8)EFaceDirection::Y_POS,
            {2,0,3,1}
        }, //UP
    },

    // X- (1)
    {
        {//LEFT
           (uint8)EFaceDirection::Z_NEG,
           {1,0,3,2},
        },//LEFT

        {//RIGHT
            (uint8)EFaceDirection::Z_POS,
            {3,2,1,0}
        },//RIGHT

        {//DOWN
            (uint8)EFaceDirection::Y_POS,
            {3,1,2,0}
        },//DOWN

        {//UP
            (uint8)EFaceDirection::Y_NEG,
            {2,0,3,1}
        },//UP
    },

    // Y+ (2)
    {
        {//LEFT
        (uint8)EFaceDirection::X_POS,
            {1,3,0,2}
        },//LEFT

        {//RIGHT
            (uint8)EFaceDirection::X_NEG,
            {3,1,2,0}
        },//RIGHT

        {//DOWN
            (uint8)EFaceDirection::Z_POS,
            {3,2,1,0}
         },//DOWN

        {//UP
            (uint8)EFaceDirection::Z_NEG,
            {2,3,0,1}
        },//UP
    },

    // Y- (3)
    {
        {//LEFT
            (uint8)EFaceDirection::X_NEG,
            { 1,3,0,2 }
        },//LEFT

        {//RIGHT
            (uint8)EFaceDirection::X_POS, 
            {3,1,2,0}
        },//RIGHT

        {//DOWN
            (uint8)EFaceDirection::Z_NEG,
            {0,1,2,3}
        },//DOWN

        {//UP
            (uint8)EFaceDirection::Z_POS,
            {1,0,3,2}
        },//UP
    },

    // FACE Z+ (4)
    {
        {//LEFT
            (uint8)EFaceDirection::X_NEG,
           //0,1,2,3 - ORIGINAL QUADRANTS{3,2,1,0}
            {3,2,1,0}
        },//LEFT

        {//RIGHT
            (uint8)EFaceDirection::X_POS,
           //0,1,2,3 - ORIGINAL QUADRANTS{2,3,0,1}
            {2,3,0,1}
        },//RIGHT

        {//DOWN
            (uint8)EFaceDirection::Y_NEG,
           //0,1,2,3 - ORIGINAL QUADRANTS{1,0,3,2}
            {1,0,3,2}
        },//DOWN
        {//UP
            (uint8)EFaceDirection::Y_POS ,
            //0,1,2,3 - ORIGINAL QUADRANTS{3,2,1,0}
             {3,2,1,0}
        },//UP
    },

    // FACE: Z- (5)
    {
        {//LEFT
            (uint8)EFaceDirection::X_NEG,
            //0,1,2,3 - ORIGINAL QUADRANTS{3,2,1,0}
             {1,0,3,2}
         },//LEFT

         {//RIGHT
             (uint8)EFaceDirection::X_POS,
             //0,1,2,3 - ORIGINAL QUADRANTS{2,3,0,1}
              {0,1,2,3}
          },//RIGHT

          {//DOWN
              (uint8)EFaceDirection::Y_NEG,
              //0,1,2,3 - ORIGINAL QUADRANTS{1,0,3,2}
               {0,1,2,3}
           },//DOWN
           {//UP
               (uint8)EFaceDirection::Y_POS ,
               //0,1,2,3 - ORIGINAL QUADRANTS{3,2,1,0}
                {2,3,0,1}
           },//UP
    }
};