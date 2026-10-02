#pragma once

#include <Arduino.h>
#include <cstddef>

#include "notes.h"

void initSoundSystem();
void updateSoundSystem(uint32_t elapsedMs);
void startBackgroundMusic();
void stopBackgroundMusic();
void stopAllSounds();
void playJumpSound();
void playGameOverSound();

