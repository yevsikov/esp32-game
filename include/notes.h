#pragma once

struct Note {
  int frequency;
  int durationMs;
};

// Notes (Hz)
constexpr int REST     = 0;

constexpr int NOTE_C3  = 131;
constexpr int NOTE_CS3 = 139;
constexpr int NOTE_D3  = 147;
constexpr int NOTE_DS3 = 156;
constexpr int NOTE_E3  = 165;
constexpr int NOTE_F3  = 175;
constexpr int NOTE_FS3 = 185;
constexpr int NOTE_G3  = 196;
constexpr int NOTE_GS3 = 208;
constexpr int NOTE_A3  = 220;
constexpr int NOTE_AS3 = 233;
constexpr int NOTE_B3  = 247;

constexpr int NOTE_C4  = 262;
constexpr int NOTE_CS4 = 277;
constexpr int NOTE_D4  = 294;
constexpr int NOTE_DS4 = 311;
constexpr int NOTE_E4  = 330;
constexpr int NOTE_F4  = 349;
constexpr int NOTE_FS4 = 370;
constexpr int NOTE_G4  = 392;
constexpr int NOTE_GS4 = 415;
constexpr int NOTE_A4  = 440;
constexpr int NOTE_AS4 = 466;
constexpr int NOTE_B4  = 494;

constexpr int NOTE_C5 = 523;
constexpr int NOTE_E5 = 659;
constexpr int NOTE_G5 = 784;

constexpr int NOTE_Bb3 = 233;
constexpr int NOTE_Bb4 = 466;
constexpr int NOTE_D5  = 587;
constexpr int NOTE_CS5 = 554;
constexpr int NOTE_F5  = 698;

constexpr int NOTE_FS5 = 740; 
constexpr int NOTE_GS5 = 831;
constexpr int NOTE_A5  = 880;
constexpr int NOTE_B5  = 988;