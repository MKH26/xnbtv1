// ─────────────────────────────────────────────────────────────────────────────
//  xnbt  –  Plugin entry point
//  src/main.cpp
// ─────────────────────────────────────────────────────────────────────────────
#include "../include/NbtGui.hpp"
#include "../include/XnbtCommand.hpp"
#include "../include/NbtParser.hpp"
#include "../nbt/NbtBridge.hpp"
#include <android/log.h>

// ── LeviLamina plugin macros (adjust for your SDK version) ───────────────────
// #include <ll/api/plugin/NativePlugin.h>
// #include <ll/api/event/EventBus.h>
// #include <ll/api/event/player/PlayerUseItemEvent.h>
// #include <ll/api/event/player/PlayerChatEvent.h>

#define LOG(fmt, ...) __android_log_print(ANDROID_LOG_INFO, "xnbt", fmt, ##__VA_ARGS__)

// ─────────────────────────────────────────────────────────────────────────────
//  Plugin lifecycle
// ─────────────────────────────────────────────────────────────────────────────
extern "C" {

// Called when the plugin .so is loaded by LeviLamina
__attribute__((visibility("default")))
bool ll_plugin_load(void* /* NativePlugin* */) {
    LOG("xnbt v" XNBT_VERSION " loading…");
    return true;
}

// Called after all plugins are loaded
__attribute__((visibility("default")))
bool ll_plugin_enable() {
    LOG("xnbt enabling…");

    // 1. Register /xnbt commands
    xnbt::XnbtCommand::registerCommands();

    // 2. Hook render event so the GUI gets drawn each frame
    //    With LeviLamina ≥ 0.13 use:
    //
    //    ll::event::EventBus::getInstance().emplaceListener<RenderEvent>(
    //        [](RenderEvent& ev){
    //            auto& g = xnbt::XnbtCommand::gui();
    //            if (g.isVisible())
    //                g.render(ev.screenWidth(), ev.screenHeight());
    //        });
    //
    //    For older versions hook Minecraft::update() via symbol hook.

    // 3. Hook touch / keyboard events for GUI interaction
    //    ll::event::EventBus::getInstance().emplaceListener<TouchEvent>(
    //        [](TouchEvent& ev){
    //            auto& g = xnbt::XnbtCommand::gui();
    //            if (!g.isVisible()) return;
    //            switch (ev.type()) {
    //                case TouchEvent::Down: g.onTouchDown(ev.x(), ev.y()); break;
    //                case TouchEvent::Up:   g.onTouchUp(ev.x(), ev.y()); break;
    //                case TouchEvent::Move: g.onTouchMove(ev.x(), ev.y(), ev.dx(), ev.dy()); break;
    //            }
    //        });

    LOG("xnbt enabled");
    return true;
}

__attribute__((visibility("default")))
bool ll_plugin_disable() {
    LOG("xnbt disabling");
    // Clean up hooks
    return true;
}

__attribute__((visibility("default")))
bool ll_plugin_unload() {
    LOG("xnbt unloaded");
    return true;
}

} // extern "C"

// ─────────────────────────────────────────────────────────────────────────────
//  Self-test  (compiled out in release)
// ─────────────────────────────────────────────────────────────────────────────
#ifdef XNBT_SELFTEST
#include <cassert>
#include <cstring>

static void selfTest() {
    using namespace xnbt;

    // Build a compound tag
    auto root = NbtTag::makeCompound();
    root->compoundSet("Health",    NbtTag::makeFloat(20.f));
    root->compoundSet("PlayerName",NbtTag::makeString("Steve"));
    root->compoundSet("Level",     NbtTag::makeInt(42));

    auto inv = NbtTag::makeList(TagType::Compound);
    auto slot = NbtTag::makeCompound();
    slot->compoundSet("id",   NbtTag::makeShort(276));
    slot->compoundSet("Count",NbtTag::makeByte(1));
    inv->listAdd(slot);
    root->compoundSet("Inventory", inv);

    // Serialize
    auto bytes = NbtParser::serialize("", root);
    LOG("Serialized %zu bytes", bytes.size());

    // Round-trip parse
    auto parsed = NbtParser::parse(bytes);
    assert(parsed && parsed->type == TagType::Compound);

    auto health = parsed->compoundGet("Health");
    assert(health && health->type == TagType::Float);
    assert(std::abs(std::get<float>(health->payload) - 20.f) < 0.001f);

    // SNBT
    std::string snbt = Snbt::toString(parsed);
    LOG("SNBT:\n%s", snbt.c_str());

    LOG("xnbt self-test PASSED");
}

__attribute__((constructor))
static void runSelfTest() { selfTest(); }
#endif
