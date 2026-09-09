#pragma once

// Набори "земля"/"перешкода" для треку (крок 4): '_' - земля, '*' - перешкода
static const char* const GROUND_PATTERNS[] = {
    "__________________",
    "__________*_______",
    "_____*_______*____",
    "_________*____*___",
    "___*____*_____*___",
};

static constexpr int GROUND_PATTERNS_COUNT = sizeof(GROUND_PATTERNS) / sizeof(GROUND_PATTERNS[0]);
