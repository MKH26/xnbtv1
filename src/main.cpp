#include "../include/NbtGui.hpp"
#include "../include/XnbtCommand.hpp"
#include "../include/NbtParser.hpp"
#include "../nbt/NbtBridge.hpp"
#include <android/log.h>

#define LOG(fmt, ...) __android_log_print(ANDROID_LOG_INFO, "xnbt", fmt, ##__VA_ARGS__)

extern "C" {

__attribute__((visibility("default")))
bool ll_plugin_load(void*) {
    LOG("xnbt v1.0.0 loading...");
    return true;
}

__attribute__((visibility("default")))
bool ll_plugin_enable() {
    LOG("xnbt enabling...");
    xnbt::XnbtCommand::registerCommands();
    LOG("xnbt enabled");
    return true;
}

__attribute__((visibility("default")))
bool ll_plugin_disable() {
    LOG("xnbt disabling");
    return true;
}

__attribute__((visibility("default")))
bool ll_plugin_unload() {
    LOG("xnbt unloaded");
    return true;
}

} // extern "C"
