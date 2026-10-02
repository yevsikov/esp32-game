#include "sounds.h"

namespace {

constexpr uint8_t BUZZER_PIN = 5;
constexpr uint8_t PWM_RESOLUTION_BITS = 8;
constexpr int NOTE_GAP_MS = 50;
constexpr int SONG_GAP_MS = 2000;

struct SequencePlayer {
  const Note *sequence = nullptr;
  size_t sequenceLength = 0;
  size_t noteIndex = 0;
  int remainingMs = 0;
  int gapRemainingMs = 0;
  bool loop = false;
  bool active = false;
};

SequencePlayer backgroundPlayer;
SequencePlayer effectPlayer;
bool backgroundRequested = false;

const Note backgroundMelody[] = {
  {NOTE_E4, 150}, {NOTE_F4, 150}, {NOTE_G4, 300}, {NOTE_C5, 800},
  {REST, 100},
  {NOTE_D4, 150}, {NOTE_E4, 150}, {NOTE_F4, 800},
  {REST, 100},

  {NOTE_G4, 150}, {NOTE_A4, 150}, {NOTE_B4, 300}, {NOTE_F5, 800},
  {REST, 100},
  {NOTE_A4, 150}, {NOTE_B4, 150}, {NOTE_C5, 300}, {NOTE_D5, 300}, {NOTE_E5, 300},
  {REST, 100},

  {NOTE_E4, 150}, {NOTE_F4, 150}, {NOTE_G4, 300}, {NOTE_C5, 800},
  {REST, 100},
  {NOTE_D5, 150}, {NOTE_E5, 150}, {NOTE_F5, 800},
  {REST, 100},

  {NOTE_G4, 150}, {NOTE_G4, 150}, {NOTE_E5, 300}, {NOTE_D5, 150}, {NOTE_G4, 150}, {NOTE_E5, 300}, {NOTE_D5, 150},
  {NOTE_G4, 150}, {NOTE_E5, 300}, {NOTE_D5, 150}, {NOTE_G4, 150}, {NOTE_F5, 300}, {NOTE_E5, 150}, {NOTE_C5, 800},
  {REST, 400}
};

const Note jumpClassic[] = {
  {NOTE_C5, 90}
};

const Note gameOverMelody[] = {
  {NOTE_CS3, 40},
  {NOTE_C3, 120},
  {REST, 100},
  {NOTE_C5, 180},
  {NOTE_G4, 180},
  {NOTE_E4, 180},
  {NOTE_A4, 250},
  {NOTE_G4, 250},
  {NOTE_F4, 250},
  {NOTE_E4, 600},
};

const size_t backgroundMelodyLength = sizeof(backgroundMelody) / sizeof(backgroundMelody[0]);
const size_t jumpClassicLength = sizeof(jumpClassic) / sizeof(jumpClassic[0]);
const size_t gameOverMelodyLength = sizeof(gameOverMelody) / sizeof(gameOverMelody[0]);

void stopNote() {
  ledcWriteTone(BUZZER_PIN, 0);
  ledcWrite(BUZZER_PIN, 0);
}

void playFrequency(int frequency) {
  if (frequency <= 0) {
    stopNote();
    return;
  }

  ledcWriteTone(BUZZER_PIN, frequency);
  ledcWrite(BUZZER_PIN, 128);
}

const Note *currentNote(const SequencePlayer &player) {
  if (!player.active || player.noteIndex == 0 || player.sequence == nullptr) {
    return nullptr;
  }

  return &player.sequence[player.noteIndex - 1];
}

void syncOutputToPlayer(const SequencePlayer &player) {
  if (!player.active) {
    stopNote();
    return;
  }

  if (player.gapRemainingMs > 0) {
    stopNote();
    return;
  }

  const Note *note = currentNote(player);
  if (note == nullptr) {
    stopNote();
    return;
  }

  playFrequency(note->frequency);
}

void startSequence(SequencePlayer &player, const Note *sequence, size_t sequenceLength, bool loop) {
  if (sequence == nullptr || sequenceLength == 0) {
    player = {};
    stopNote();
    return;
  }

  player.sequence = sequence;
  player.sequenceLength = sequenceLength;
  player.noteIndex = 0;
  player.remainingMs = 0;
  player.gapRemainingMs = 0;
  player.loop = loop;
  player.active = true;

  const Note &note = player.sequence[player.noteIndex++];
  player.remainingMs = note.durationMs;
  playFrequency(note.frequency);
}

void stopSequence(SequencePlayer &player) {
  player = {};
}

void advanceSequence(SequencePlayer &player, uint32_t elapsedMs) {
  if (!player.active || player.sequence == nullptr) {
    return;
  }

  uint32_t remainingMs = elapsedMs;
  while (remainingMs > 0 && player.active) {
    if (player.remainingMs > 0) {
      if (remainingMs < static_cast<uint32_t>(player.remainingMs)) {
        player.remainingMs -= static_cast<int>(remainingMs);
        return;
      }

      remainingMs -= static_cast<uint32_t>(player.remainingMs);
      player.remainingMs = 0;
      stopNote();
      player.gapRemainingMs = NOTE_GAP_MS;
      continue;
    }

    if (player.gapRemainingMs > 0) {
      if (remainingMs < static_cast<uint32_t>(player.gapRemainingMs)) {
        player.gapRemainingMs -= static_cast<int>(remainingMs);
        return;
      }

      remainingMs -= static_cast<uint32_t>(player.gapRemainingMs);
      player.gapRemainingMs = 0;
    }

    if (player.noteIndex >= player.sequenceLength) {
      if (player.loop) {
        player.noteIndex = 0;
        player.gapRemainingMs = SONG_GAP_MS;
        continue;
      }

      player.active = false;
      stopNote();
      return;
    }

    const Note &note = player.sequence[player.noteIndex++];
    player.remainingMs = note.durationMs;
    playFrequency(note.frequency);
  }
}

void ensureBackgroundRunning() {
  if (backgroundRequested && !backgroundPlayer.active) {
    startSequence(backgroundPlayer, backgroundMelody, backgroundMelodyLength, true);
  }
}

} // namespace

void initSoundSystem() {
  if (!ledcAttach(BUZZER_PIN, 2000, PWM_RESOLUTION_BITS)) {
    Serial.println("Failed to attach LEDC to buzzer pin");
  }

  backgroundRequested = false;
  stopSequence(backgroundPlayer);
  stopSequence(effectPlayer);
  stopNote();
}

void updateSoundSystem(uint32_t elapsedMs) {
  if (effectPlayer.active) {
    advanceSequence(effectPlayer, elapsedMs);
    if (!effectPlayer.active) {
      ensureBackgroundRunning();
      syncOutputToPlayer(backgroundPlayer);
    }
    return;
  }

  ensureBackgroundRunning();
  advanceSequence(backgroundPlayer, elapsedMs);
}

void startBackgroundMusic() {
  backgroundRequested = true;
  ensureBackgroundRunning();
  syncOutputToPlayer(backgroundPlayer);
}

void stopBackgroundMusic() {
  backgroundRequested = false;
  stopSequence(backgroundPlayer);
  stopNote();
}

void stopAllSounds() {
  backgroundRequested = false;
  stopSequence(backgroundPlayer);
  stopSequence(effectPlayer);
  stopNote();
}

void playJumpSound() {
  startSequence(effectPlayer, jumpClassic, jumpClassicLength, false);
}

void playGameOverSound() {
  startSequence(effectPlayer, gameOverMelody, gameOverMelodyLength, false);
}
