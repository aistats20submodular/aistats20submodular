#pragma once

enum Mode { NONE, PLUS_MAX, OR_MAX };

map<Mode, string> modeName = {
    { NONE, "" },
    { PLUS_MAX, "+Max"},
    { OR_MAX, "OrMax" }
};