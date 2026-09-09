#include "melody.h"

extern const Note melody[] = {
  // --- Куплетно-основне мотивне фанфари ---
  {NOTE_E4, 150}, {NOTE_F4, 150}, {NOTE_G4, 300}, {NOTE_C5, 800},
  {REST, 100},
  {NOTE_D4, 150}, {NOTE_E4, 150}, {NOTE_F4, 800},
  {REST, 100},

  {NOTE_G4, 150}, {NOTE_A4, 150}, {NOTE_B4, 300}, {NOTE_F5, 800},
  {REST, 100},
  {NOTE_A4, 150}, {NOTE_B4, 150}, {NOTE_C5, 300}, {NOTE_D5, 300}, {NOTE_E5, 300},
  {REST, 100},

  // Повтор першої фанфари
  {NOTE_E4, 150}, {NOTE_F4, 150}, {NOTE_G4, 300}, {NOTE_C5, 800},
  {REST, 100},
  {NOTE_D5, 150}, {NOTE_E5, 150}, {NOTE_F5, 800},
  {REST, 100},

  // Завершення головного мотиву
  {NOTE_G4, 150}, {NOTE_G4, 150}, {NOTE_E5, 300}, {NOTE_D5, 150}, {NOTE_G4, 150}, {NOTE_E5, 300}, {NOTE_D5, 150},
  {NOTE_G4, 150}, {NOTE_E5, 300}, {NOTE_D5, 150}, {NOTE_G4, 150}, {NOTE_F5, 300}, {NOTE_E5, 150}, {NOTE_C5, 800},
  {REST, 400}
};

extern const size_t melodyLength = sizeof(melody) / sizeof(melody[0]);