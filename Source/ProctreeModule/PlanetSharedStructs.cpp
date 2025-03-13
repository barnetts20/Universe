#include "PlanetSharedStructs.h"

// Then, define the static table in a .cpp file:
// Order of directions: LEFT, RIGHT, UP, DOWN
// Order: LEFT, RIGHT, UP, DOWN
//const FaceTransition FQuadIndex::FaceTransitions[6][4] = {
//    // FACE: X+ (0)
//    {
//        {static_cast<uint8>(EFaceDirection::Z_POS), 3, false, false},  // LEFT -> Z+, rot 270°
//        {static_cast<uint8>(EFaceDirection::Z_NEG), 1, false, false},  // RIGHT -> Z-, rot 90°
//        {static_cast<uint8>(EFaceDirection::Y_POS), 0, false, false},  // UP -> Y+, no rot
//        {static_cast<uint8>(EFaceDirection::Y_NEG), 0, false, false}   // DOWN -> Y-, no rotation
//    },
//
//    // FACE: X- (1)
//    {
//        {static_cast<uint8>(EFaceDirection::Z_NEG), 3, false, false},  // LEFT -> Z-, rot 270°
//        {static_cast<uint8>(EFaceDirection::Z_POS), 1, false, false},  // RIGHT -> Z+, rot 90°
//        {static_cast<uint8>(EFaceDirection::Y_POS), 2, true, true},    // UP -> Y+, rot 180°, flip XY
//        {static_cast<uint8>(EFaceDirection::Y_NEG), 0, true, true}     // DOWN -> Y-, no rotation, flip XY
//    },
//
//    // FACE: Y+ (2)
//    {
//        {static_cast<uint8>(EFaceDirection::X_NEG), 0, false, false},  // LEFT -> X-, no rotation
//        {static_cast<uint8>(EFaceDirection::X_POS), 0, false, false},  // RIGHT -> X+, no rotation
//        {static_cast<uint8>(EFaceDirection::Z_POS), 0, false, false},  // UP -> Z+, no rotation
//        {static_cast<uint8>(EFaceDirection::Z_NEG), 2, false, true}    // DOWN -> Z-, 180° rotation, flip Y
//    },
//
//    // FACE: Y- (3)
//    {
//        {static_cast<uint8>(EFaceDirection::X_POS), 0, false, false},  // LEFT -> X+, no rotation
//        {static_cast<uint8>(EFaceDirection::X_NEG), 1, false, false},  // RIGHT -> X-, no rotation
//        {static_cast<uint8>(EFaceDirection::Z_POS), 0, false, false},  // UP -> Z+, no rotation
//        {static_cast<uint8>(EFaceDirection::Z_NEG), 2, false, true}    // DOWN -> Z-, 180° rotation, flip Y
//    },
//
//    // FACE: Z+ (4)
//    {
//        {static_cast<uint8>(EFaceDirection::X_NEG), 1, false, false},  // LEFT -> X-, no rotation
//        {static_cast<uint8>(EFaceDirection::X_POS), 0, false, false},  // RIGHT -> X+, no rotation
//        {static_cast<uint8>(EFaceDirection::Y_POS), 2, false, false},  // UP -> Y+, no rotation
//        {static_cast<uint8>(EFaceDirection::Y_NEG), 0, false, false}   // DOWN -> Y-, no rotation
//    },
//
//    // FACE: Z- (5)
//    {
//        {static_cast<uint8>(EFaceDirection::X_POS), 0, false, false},  // LEFT -> X+, 90° rotation
//        {static_cast<uint8>(EFaceDirection::X_NEG), 1, false, false},  // RIGHT -> X-, 270° rotation
//        {static_cast<uint8>(EFaceDirection::Y_POS), 2, false, true},   // UP -> Y+, 180° rotation, flip Y
//        {static_cast<uint8>(EFaceDirection::Y_NEG), 0, false, false}   // DOWN -> Y-, no rotation
//    }
//};


// Directions: LEFT, RIGHT, UP, DOWN
const FaceTransition FQuadIndex::FaceTransitions[6][4] = {
    // X+ (0)
    {   {(uint8)EFaceDirection::Y_POS, 0, false, false}, //LEFT Y+
        {(uint8)EFaceDirection::Y_NEG, 0, false, false}, //RIGHT Y-
        {(uint8)EFaceDirection::Z_POS, 2, false, false}, //UP 90*
        {(uint8)EFaceDirection::Z_NEG, 1, false, false}  //DOWN 90*
    },

    // X- (1)
    {
        {(uint8)EFaceDirection::Y_NEG, 0, false, false}, // LEFT -> Y-
        {(uint8)EFaceDirection::Y_POS, 0, false, false}, // RIGHT-> Y+
        {(uint8)EFaceDirection::Z_POS, 3, false, false},   // UP-> Z+ 270
        {(uint8)EFaceDirection::Z_NEG, 3, false, false}    // DOWN-> Z-, 270° flip XY
    },

    // Y+ (2)
    {
        {(uint8)EFaceDirection::X_NEG, 0, false, false}, // LEFT -> X-
        {(uint8)EFaceDirection::X_POS, 0, false, false}, // RIGHT-> X+
        {(uint8)EFaceDirection::Z_POS, 2, false, false}, // UP   -> Z+ 180
        {(uint8)EFaceDirection::Z_NEG, 0, false, false}    // DOWN-> Z-
    },

    // Y- (3)
    {
        {(uint8)EFaceDirection::X_POS, 0, false, false}, // LEFT -> X+
        {(uint8)EFaceDirection::X_NEG, 0, false, false}, // RIGHT-> X-
        {(uint8)EFaceDirection::Z_POS, 0, false, false},   // UP   -> Z+
        {(uint8)EFaceDirection::Z_NEG, 2, false, false}    // DOWN -> Z- 180°
    },

    // FACE Z+ (4)
    {
        {(uint8)EFaceDirection::X_POS, 3, false, false},  // LEFT -> Y+, rot 90°
        {(uint8)EFaceDirection::X_NEG, 1, false, false},  // RIGHT-> Y-, rot 270°
        {(uint8)EFaceDirection::Y_POS, 2, false, false}, // UP -> Y+ 180
        {(uint8)EFaceDirection::Y_NEG, 0, false, false}  // DOWN -> Y-
    },

    // FACE: Z- (5)
    {
        {(uint8)EFaceDirection::X_NEG, 1, false, false},  // LEFT -> Y-, rot 90°
        {(uint8)EFaceDirection::X_POS, 3, false, false},  // RIGHT-> Y+, rot 270°
        {(uint8)EFaceDirection::Y_POS, 0, false, false},   // UP -> X+, rot 180°, flip XY
        {(uint8)EFaceDirection::Y_NEG, 2, false, false}  // DOWN -> X-, no rotation
    }
};