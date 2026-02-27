# 🎮 R36S/Linux/Windows/Web/Controller Cross-Platform Input Table

Implemented in the following:

- [`command.h`](../include/command.h) (logical command bits)
- `PollInput()` in [`input_manager.c`](../src/input_manager.c) (Raylib keyboard/gamepad (R36S | Xbox) mapping)
- `ReadR36SInputEvents()` in [`input_manager.c`](../src/input_manager.c) (R36S Linux EV_KEY mapping)

## 🧩 Logical Commands

| Command | Description |
|---------|-------------|
| `MOVE_UP` | Move Up |
| `MOVE_DOWN` | Move Down |
| `MOVE_LEFT` | Move Left |
| `MOVE_RIGHT` | Move Right |
| `ATTACK_PRIMARY` | Primary Attack |
| `ATTACK_SECONDARY` | Secondary Attack |
| `ACTION_SPECIAL_1` | Special 1 |
| `ACTION_SPECIAL_2` | Special 2 |
| `ACTION_SPECIAL_3` | Special 3 (L3 click) |
| `ACTION_SPECIAL_4` | Special 4 (R3 click) |
| `ACTION_JUMP` | Jump |
| `ACTION_CROUCH` | Crouch |
| `ACTION_PICKUP` | Pickup |
| `ACTION_RUN` | Run |
| `AIM` | Aim (Right Stick / Keypad) |
| `MENU_TOGGLE` | Toggle Menu / Back |
| `START_GAME` | Start (and/or Pause depending on Game State) |
| `EXIT_COMMAND` | Exit (ESC / FN) |
| `POWER_COMMAND` | Power (Save Game / Restore Game Placeholder) |
| `VOLUME_UP` | Volume up (R36S Gamepad System Button) |
| `VOLUME_DOWN` | Volume down (R36S Gamepad System Button) |

> Note: `VOLUME_*`, `POWER_COMMAND` are **non-gameplay system inputs**.

## 🎮 Input Mapping (Keyboard <-> Xbox Controller <-> PlayStation Controller <-> R36S Gamepad)

| Logical Command | Keyboard | Xbox Controller | PlayStation Controller | R36S Gamepad |
|----------------|----------|-----------------|------------------------|-------------|
| `MOVE_UP` | W / ⬆️ | ⬆️ D-Pad Up / 🕹️ Left Stick Up | ⬆️ D-Pad Up / 🕹️ Left Stick Up | ⬆️ D-Pad Up / 🕹️ Left Stick |
| `MOVE_DOWN` | S / ⬇️ | ⬇️ D-Pad Down / 🕹️ Left Stick Down | ⬇️ D-Pad Down / 🕹️ Left Stick Down | ⬇️ D-Pad Down / 🕹️ Left Stick |
| `MOVE_LEFT` | A / ⬅️ | ⬅️ D-Pad Left / 🕹️ Left Stick Left | ⬅️ D-Pad Left / 🕹️ Left Stick Left | ⬅️ D-Pad Left / 🕹️ Left Stick |
| `MOVE_RIGHT` | D / ➡️ | ➡️ D-Pad Right / 🕹️ Left Stick Right | ➡️ D-Pad Right / 🕹️ Left Stick Right | ➡️ D-Pad Right / 🕹️ Left Stick |
| `ATTACK_PRIMARY` | Space | RT (Analog) | R2 (Analog) | R1 |
| `ATTACK_SECONDARY` | Ctrl | RB | R1 | R2 |
| `ACTION_SPECIAL_1` | Z | LT (Analog) | L2 (Analog) | L1 |
| `ACTION_SPECIAL_2` | C | LB | L1 | L2 |
| `ACTION_SPECIAL_3` | V | L3 (Left Stick Click) | L3 (Left Stick Click) | L3 |
| `ACTION_SPECIAL_4` | B | R3 (Right Stick Click) | R3 (Right Stick Click) | R3 |
| `ACTION_JUMP` | P | A (Bottom / Green) | X (Cross) | A (Right / Red) |
| `ACTION_CROUCH` | L | B (Right / Red) | O (Circle) | B (Bottom / Yellow) |
| `ACTION_PICKUP` | O | X (Left / Blue) | □ (Square) | X (Top / Blue) |
| `ACTION_RUN` | K | Y (Top / Yellow) | △ (Triangle) | Y (Left / Green) |
| `AIM` | Numpad 8/6/2/4 | 🕹️ Right Stick | 🕹️ Right Stick | 🕹️ Right Stick |
| `MENU_TOGGLE` | Tab | Back | Share | Select |
| `START_GAME` | Enter | Start | Options | Start |
| `EXIT_COMMAND` | Esc | — | — | FN |
| `POWER_COMMAND` | — | — | — | Power |
| `VOLUME_UP` | — | — | — | Vol + |
| `VOLUME_DOWN` | — | — | — | Vol − |

## 🎮 Movement (D-Pad + Left Stick)

### D-Pad (Raylib Buttons)
| Logical Action | Keyboard | Xbox Controller | R36S Gamepad | Raylib Buttons |
|----------------|----------|-----------------|-----------------|--------------|
| `MOVE_UP` | W / ⬆️ | ⬆️ D-Pad Up | ⬆️ D-Pad Up | `GAMEPAD_BUTTON_LEFT_FACE_UP` |
| `MOVE_DOWN` | S / ⬇️ | ⬇️ D-Pad Down | ⬇️ D-Pad Down | `GAMEPAD_BUTTON_LEFT_FACE_DOWN` |
| `MOVE_LEFT` | A / ⬅️ | ⬅️ D-Pad Left | ⬅️ D-Pad Left | `GAMEPAD_BUTTON_LEFT_FACE_LEFT` |
| `MOVE_RIGHT` | D / ➡️ | ➡️ D-Pad Right | ➡️ D-Pad Right | `GAMEPAD_BUTTON_LEFT_FACE_RIGHT` |

### Left Stick (Raylib Axes)
| Stick Direction | Xbox Controller | Raylib Axis | Behavior |
|----------------|-----------------|-------------|--------------|
| Up | Left Stick Up | `GAMEPAD_AXIS_LEFT_Y` | **Negative Y** -> `MOVE_UP` |
| Down | Left Stick Down | `GAMEPAD_AXIS_LEFT_Y` | **Positive Y** -> `MOVE_DOWN` |
| Left | Left Stick Left | `GAMEPAD_AXIS_LEFT_X` | **Negative X** -> `MOVE_LEFT` |
| Right | Left Stick Right | `GAMEPAD_AXIS_LEFT_X` | **Positive X** -> `MOVE_RIGHT` |

> Deadzone thresholds come from `constants.h` `TUMBSTICK_DEADZONE_THRESHOLD`, `MOVE_VERTICAL_THRESHOLD`, `MOVE_HORIZONTAL_THRESHOLD`. Right Stick can be implemented in a similar fashion. Currently just one behaviour **_AIM_**.

## 🎮 Face Buttons (A/B/X/Y)

### Keyboard mapping (from `PollInput()`)
| Logical Command | Keyboard |
|----------------|----------|
| `ACTION_JUMP` | P |
| `ACTION_CROUCH` | L |
| `ACTION_PICKUP` | O |
| `ACTION_RUN` | K |

### Xbox mapping (from `PollInput()`)
| Logical Command | Xbox Button | Raylib Button |
|----------------|------------|--------------|
| `ACTION_JUMP` | A (Bottom / Green) | `GAMEPAD_BUTTON_RIGHT_FACE_DOWN` |
| `ACTION_CROUCH` | B (Right / Red) | `GAMEPAD_BUTTON_RIGHT_FACE_RIGHT` |
| `ACTION_PICKUP` | X (Left / Blue) | `GAMEPAD_BUTTON_RIGHT_FACE_UP` |
| `ACTION_RUN` | Y (Top / Yellow) | `GAMEPAD_BUTTON_RIGHT_FACE_LEFT` |

### R36S Gamepad mapping (from `PollInput()`)
| Logical Command | R36S Button | Raylib Button |
|----------------|--------------|--------------------|
| `ACTION_JUMP` | A (Right / Red) | `GAMEPAD_BUTTON_RIGHT_FACE_RIGHT` |
| `ACTION_CROUCH` | B (Bottom / Yellow) | `GAMEPAD_BUTTON_RIGHT_FACE_DOWN` |
| `ACTION_PICKUP` | X (Top / Blue) | `GAMEPAD_BUTTON_RIGHT_FACE_LEFT` |
| `ACTION_RUN` | Y (Left / Green) | `GAMEPAD_BUTTON_RIGHT_FACE_UP` |

> **Note:** R36S Gamepad layout differs from XBox Controller.

## 🎮 Triggers (L1 / L2 / R1 / R2)

### Keyboard mapping (from `PollInput()`)
| Logical Command | Keyboard |
|----------------|----------|
| `ACTION_SPECIAL_1` | Z |
| `ACTION_SPECIAL_2` | C |
| `ATTACK_PRIMARY` | Space |
| `ATTACK_SECONDARY` | Left Ctrl / Right Ctrl |

### Xbox Controller mapping (from `PollInput()`)
| Logical Command | Xbox Control | Raylib Trigger |
|----------------|-------------|--------------|
| `ACTION_SPECIAL_1` | LT (Analog) | `GAMEPAD_AXIS_LEFT_TRIGGER` > threshold |
| `ACTION_SPECIAL_2` | LB (Digital) | `GAMEPAD_BUTTON_LEFT_TRIGGER_1` |
| `ATTACK_PRIMARY` | RT (Analog) | `GAMEPAD_AXIS_RIGHT_TRIGGER` > threshold |
| `ATTACK_SECONDARY` | RB (Digital) | `GAMEPAD_BUTTON_RIGHT_TRIGGER_1` |

### R36S Gamepad mapping (from `PollInput()` Digital Triggers)
| Logical Command | R36S Gamepad | Raylib Trigger |
|----------------|-------------|--------------|
| `ACTION_SPECIAL_1` | L1 | `GAMEPAD_BUTTON_LEFT_TRIGGER_1` |
| `ACTION_SPECIAL_2` | L2 | `GAMEPAD_BUTTON_LEFT_TRIGGER_2` |
| `ATTACK_PRIMARY` | R1 | `GAMEPAD_BUTTON_RIGHT_TRIGGER_1` |
| `ATTACK_SECONDARY` | R2 | `GAMEPAD_BUTTON_RIGHT_TRIGGER_2` |

> Note: R36S triggers are digital (0/1).

## 🎮 Stick Buttons (L3 / R3)

### Keyboard mapping (from `PollInput()`)
| Logical Command | Keyboard |
|----------------|----------|
| `ACTION_SPECIAL_3` | V |
| `ACTION_SPECIAL_4` | B |

### Xbox Controller (Raylib)
| Logical Command | Xbox Controller | Raylib Tumbstick Button |
|----------------|------|--------|
| `ACTION_SPECIAL_3` | Left Stick Click | `GAMEPAD_BUTTON_LEFT_THUMB` |
| `ACTION_SPECIAL_4` | Right Stick Click | `GAMEPAD_BUTTON_RIGHT_THUMB` |

### R36S Gamepad
| Logical Command | R36S Gamepad | Linux EV_KEY |
|----------------|------|--------------|
| `ACTION_SPECIAL_3` | L3 | `706` |
| `ACTION_SPECIAL_4` | R3 | `707` |

> **Note:** R36S exposes stick clicks via Linux EV_KEY.

## 🎮 Menu / Start / System Buttons

### Keyboard
| Logical Command | Keyboard |
|----------------|----------|
| `MENU_TOGGLE` | Tab |
| `START_GAME` | Enter / Keypad Enter |
| `EXIT_COMMAND` | Esc |

> **Note:** Menu Toggle, Start Game and Exit Game behaviors have to be implemented, commands are passed but not implemented.

### Xbox (Raylib)
| Logical Command | Xbox Button | Raylib |
|----------------|------------|--------|
| `MENU_TOGGLE` | Back | `GAMEPAD_BUTTON_MIDDLE_LEFT` |
| `START_GAME` | Start | `GAMEPAD_BUTTON_MIDDLE` |

> **Note:** Menu Toggle and Start Game behaviors have to be implemented, commands are passed but not implemented.

### R36S Gamepad (Linux EV_KEY)
| Logical Command | R36S Gamepad | EV_KEY Code | Notes |
|----------------|-------------|-------------|------|
| `MENU_TOGGLE` | Select | `704` | Non-gameplay Menu |
| `START_GAME` | Start | `705` | Start / Pause |
| `EXIT_COMMAND` | FN | `708` | Exit Game |
| `POWER_COMMAND` | Power | `116` | Save / Restore Game |
| `VOLUME_UP` | Vol+ | `115` | Volume up (must be implemented in Game) |
| `VOLUME_DOWN` | Vol- | `114` | Volume up (must be implemented in Game) |

> **Note:** Menu, Start, Pause, Save, Restore and Volume +/- behaviors have to be implemented, commands are passed but not implemented.

## R36S Gamepad System Buttons
| Key | Keyboard | Xbox | PS | R36S Gamepad | EV_KEY Code * | Raylib ** |
|-----|----------|------|------|------------|-------------|--------|
| Volume Up | Key Volume Up | -- | -- | Vol + | `115` | `KEY_VOLUME_UP` ** |
| Volume Down | Key Volume Down | -- | -- | Vol - | `114` | `KEY_VOLUME_DOWN` ** |
| Power | F1 | -- | -- | Power | `116` | -- |

> **Note:** System Keys should not trigger gameplay.
    * ArkOS Linux EV_KEY Code.
    ** Keyboard mapping raylib.