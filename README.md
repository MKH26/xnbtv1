# xnbt — In-Game NBT Editor Mod for LeviLamina (Android)

A fully featured, touch-friendly NBT editor mod for Minecraft Bedrock Edition
running on LeviLamina Android. Edit entity, block-entity, and item NBT live
while the game is running — no PC required.

---

## Features

| Feature | Description |
|---|---|
| 🌲 Tree editor | Collapsible compound/list tree with colour-coded type badges |
| ✏️ Live editing | Tap any tag → edit key or value inline |
| ➕ Add tags | All 12 NBT types supported |
| 🗑️ Delete tags | Remove any tag from a compound or list |
| 🔍 Search | Filter the tree by key name or value |
| ↩️ Undo / ↪️ Redo | Full edit history per session |
| 📋 Copy/Paste | Deep-clone any subtree |
| 💾 Save | Write NBT back to the entity/block/item immediately |
| 📦 SNBT export | Human-readable string output for debugging |
| 🎮 Commands | `/xnbt self` · `item` · `block` · `entity` |

---

## Project structure

```
xnbt/
├── CMakeLists.txt          — build definition
├── manifest.json           — LeviLamina plugin manifest
├── build.sh                — one-shot NDK cross-compile script
│
├── include/
│   ├── NbtTypes.hpp        — tag structs + helpers
│   ├── NbtParser.hpp       — binary LE parser + serialiser
│   ├── Snbt.hpp            — SNBT (pretty + compact) renderer
│   ├── NbtGui.hpp          — GUI state machine declaration
│   └── XnbtCommand.hpp     — /xnbt command dispatcher
│
├── nbt/
│   └── NbtBridge.hpp       — Bedrock Actor/BlockActor/Item glue
│
├── gui/
│   └── NbtGui.cpp          — GUI render + touch + action logic
│
└── src/
    └── main.cpp            — LeviLamina plugin lifecycle hooks
```

---

## Building

### Requirements

- **Android NDK r25c+**  
  Download from https://developer.android.com/ndk/downloads
- **CMake 3.21+**
- **LeviLamina SDK** ≥ 0.13.0  
  https://github.com/LiteLDev/LeviLamina
- A Linux / macOS / WSL2 shell

### Steps

```bash
# 1. Clone this repo
git clone https://github.com/your-name/xnbt
cd xnbt

# 2. Point to your NDK and LeviLamina SDK
export ANDROID_NDK_HOME=~/Android/Sdk/ndk/25.2.9519653
export LEVILAMINADIR=~/LeviLamina-sdk

# 3. Build (arm64-v8a release)
chmod +x build.sh
./build.sh release arm64-v8a

# Output: build/arm64-v8a-release/xnbt.so
```

---

## Installation

1. Copy `xnbt.so` and `manifest.json` into your LeviLamina plugins folder:
   ```
   /storage/emulated/0/games/com.mojang/minecraftpe/plugins/xnbt/
   ```
2. Start (or restart) the game via LeviLauncher.
3. Join a world and run `/xnbt help`.

---

## Usage

| Command | Opens |
|---|---|
| `/xnbt self` | Your own player NBT |
| `/xnbt item` | NBT of the item you're holding |
| `/xnbt block` | NBT of the block entity you're looking at |
| `/xnbt entity` | NBT of the nearest entity |

### GUI controls

| Gesture / button | Action |
|---|---|
| Tap a row | Select tag |
| Tap again on compound/list | Expand / collapse |
| Drag vertically | Scroll |
| ✏️ Edit Val | Edit the tag's value (opens virtual keyboard) |
| 🏷️ Edit Key | Rename the key |
| ➕ Add | Add a child tag (compound or list must be selected) |
| ✖️ Del | Delete the selected tag |
| 📋 Copy | Copy tag to clipboard |
| 📥 Paste | Paste clipboard tag into selected compound/list |
| ↩️ Undo / ↪️ Redo | Step through edit history |
| 🔍 Search bar | Filter visible tags |
| 💾 Save | Write NBT back to game object |
| ✕ | Close without saving |

---

## Integrating the draw primitives

`NbtGui.cpp` contains stub `drawRect`, `drawText`, `drawButton`, `drawLine`
methods. Replace these with your renderer of choice:

### Option A – LeviLamina RenderUI
```cpp
// In NbtGui.cpp, replace drawRect():
void NbtGui::drawRect(float x, float y, float w, float h, Color c, float r) {
    ll::gui::drawFilledRect({x, y, x+w, y+h}, {c.r, c.g, c.b, c.a}, r);
}
```

### Option B – Dear ImGui (if bundled with your LeviLamina build)
```cpp
void NbtGui::render(float sw, float sh) {
    ImGui::SetNextWindowSize({sw, sh});
    ImGui::Begin("xnbt", nullptr, ImGuiWindowFlags_NoMove);
    // ... render nodes via ImGui::TreeNode / ImGui::InputText
    ImGui::End();
}
```

### Option C – Custom OpenGL/Vulkan overlay
Draw into a transparent overlay surface acquired via
`ANativeWindow` / `EGL` hooks.

---

## Connecting to Bedrock NBT API

Open `nbt/NbtBridge.hpp` and fill in the real calls.  
With LeviLamina 0.13+:

```cpp
// readActorNbt
static NbtTagPtr readActorNbt(Actor* actor) {
    auto tag = std::make_unique<CompoundTag>();
    actor->save(*tag);
    return convertFromBedrock(*tag);   // write this converter
}

// writeActorNbt
static bool writeActorNbt(Actor* actor, const NbtTagPtr& nbt) {
    auto tag = convertToBedrock(nbt);  // write this converter
    actor->load(*tag);
    return true;
}
```

---

## NBT type colour reference

| Type | Colour |
|---|---|
| TAG_Byte | 🟡 Orange-yellow |
| TAG_Short | 🟡 Yellow |
| TAG_Int | 🔵 Light blue |
| TAG_Long | 🔵 Blue |
| TAG_Float | 🩷 Pink |
| TAG_Double | 🟣 Purple |
| TAG_String | 🟢 Green |
| TAG_Compound | 🟡 Gold |
| TAG_List | 🟠 Amber |
| TAG_*Array | 🩵 Teal |

---

## License

MIT — free to use, modify, and redistribute.
