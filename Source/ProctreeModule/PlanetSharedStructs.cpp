#include "PlanetSharedStructs.h"

// Then, define the static table in a .cpp file:
// Order of directions: LEFT, RIGHT, UP, DOWN
const FaceTransition FQuadIndex::FaceTransitions[6][4] = {
    // Face 0 (X_POS) neighbors
    {
        {static_cast<uint8>(EFaceDirection::Z_POS), 3, true, false},   // LEFT  -> Z+, rotate 270°, flip X
        {static_cast<uint8>(EFaceDirection::Z_NEG), 1, true, false},   // RIGHT -> Z-, rotate 90°, flip X
        {static_cast<uint8>(EFaceDirection::Y_POS), 0, false, false},  // UP    -> Y+, no rotation
        {static_cast<uint8>(EFaceDirection::Y_NEG), 2, false, true}    // DOWN  -> Y-, rotate 180°, flip Y
    },

    // Face 1 (X_NEG) neighbors
    {
        {static_cast<uint8>(EFaceDirection::Z_NEG), 3, true, false},   // LEFT  -> Z-, rotate 270°, flip X
        {static_cast<uint8>(EFaceDirection::Z_POS), 1, true, false},   // RIGHT -> Z+, rotate 90°, flip X
        {static_cast<uint8>(EFaceDirection::Y_POS), 2, true, true},    // UP    -> Y+, rotate 180°, flip both
        {static_cast<uint8>(EFaceDirection::Y_NEG), 0, true, true}     // DOWN  -> Y-, no rotation, flip both
    },

    // Face 2 (Y_POS) neighbors
    {
        {static_cast<uint8>(EFaceDirection::X_NEG), 0, true, false},   // LEFT  -> X-, no rotation, flip X
        {static_cast<uint8>(EFaceDirection::X_POS), 0, false, false},  // RIGHT -> X+, no rotation
        {static_cast<uint8>(EFaceDirection::Z_POS), 0, false, false},  // UP    -> Z+, no rotation
        {static_cast<uint8>(EFaceDirection::Z_NEG), 2, false, true}    // DOWN  -> Z-, rotate 180°, flip Y
    },

    // Face 3 (Y_NEG) neighbors
    {
        {static_cast<uint8>(EFaceDirection::X_POS), 2, false, true},   // LEFT  -> X+, rotate 180°, flip Y
        {static_cast<uint8>(EFaceDirection::X_NEG), 2, true, true},    // RIGHT -> X-, rotate 180°, flip both
        {static_cast<uint8>(EFaceDirection::Z_NEG), 0, false, false},  // UP    -> Z-, no rotation
        {static_cast<uint8>(EFaceDirection::Z_POS), 2, false, true}    // DOWN  -> Z+, rotate 180°, flip Y
    },

    // Face 4 (Z_POS) neighbors
    {
        {static_cast<uint8>(EFaceDirection::X_POS), 1, true, false},   // LEFT  -> X+, rotate 90°, flip X
        {static_cast<uint8>(EFaceDirection::X_NEG), 3, true, false},   // RIGHT -> X-, rotate 270°, flip X
        {static_cast<uint8>(EFaceDirection::Y_POS), 2, false, true},   // UP    -> Y+, rotate 180°, flip Y
        {static_cast<uint8>(EFaceDirection::Y_NEG), 0, false, false}   // DOWN  -> Y-, no rotation
    },

    // Face 5 (Z_NEG) neighbors
    {
        {static_cast<uint8>(EFaceDirection::X_NEG), 1, true, false},   // LEFT  -> X-, rotate 90°, flip X
        {static_cast<uint8>(EFaceDirection::X_POS), 3, true, false},   // RIGHT -> X+, rotate 270°, flip X
        {static_cast<uint8>(EFaceDirection::Y_NEG), 2, false, true},   // UP    -> Y-, rotate 180°, flip Y
        {static_cast<uint8>(EFaceDirection::Y_POS), 0, false, false}   // DOWN  -> Y+, no rotation
    }
};