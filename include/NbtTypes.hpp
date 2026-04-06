#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <variant>
#include <unordered_map>
#include <functional>

namespace xnbt {

enum class TagType : uint8_t {
    End=0, Byte=1, Short=2, Int=3, Long=4,
    Float=5, Double=6, ByteArray=7, String=8,
    List=9, Compound=10, IntArray=11, LongArray=12,
};

struct NbtTag;
using NbtTagPtr = std::shared_ptr<NbtTag>;
using ByteArrayPayload = std::vector<int8_t>;
using IntArrayPayload  = std::vector<int32_t>;
using LongArrayPayload = std::vector<int64_t>;
using ListPayload      = std::vector<NbtTagPtr>;
using CompoundPayload  = std::vector<std::pair<std::string,NbtTagPtr>>;

using TagPayload = std::variant<
    std::monostate, int8_t, int16_t, int32_t, int64_t,
    float, double, ByteArrayPayload, std::string,
    ListPayload, CompoundPayload, IntArrayPayload, LongArrayPayload
>;

struct NbtTag {
    TagType    type    = TagType::End;
    TagPayload payload = std::monostate{};
    TagType    listInnerType = TagType::End;

    static NbtTagPtr makeByte(int8_t v)        { auto t=std::make_shared<NbtTag>(); t->type=TagType::Byte;     t->payload=v; return t; }
    static NbtTagPtr makeShort(int16_t v)      { auto t=std::make_shared<NbtTag>(); t->type=TagType::Short;    t->payload=v; return t; }
    static NbtTagPtr makeInt(int32_t v)        { auto t=std::make_shared<NbtTag>(); t->type=TagType::Int;      t->payload=v; return t; }
    static NbtTagPtr makeLong(int64_t v)       { auto t=std::make_shared<NbtTag>(); t->type=TagType::Long;     t->payload=v; return t; }
    static NbtTagPtr makeFloat(float v)        { auto t=std::make_shared<NbtTag>(); t->type=TagType::Float;    t->payload=v; return t; }
    static NbtTagPtr makeDouble(double v)      { auto t=std::make_shared<NbtTag>(); t->type=TagType::Double;   t->payload=v; return t; }
    static NbtTagPtr makeString(std::string v) { auto t=std::make_shared<NbtTag>(); t->type=TagType::String;   t->payload=std::move(v); return t; }
    static NbtTagPtr makeCompound()            { auto t=std::make_shared<NbtTag>(); t->type=TagType::Compound; t->payload=CompoundPayload{}; return t; }
    static NbtTagPtr makeList(TagType inner=TagType::End) {
        auto t=std::make_shared<NbtTag>(); t->type=TagType::List;
        t->payload=ListPayload{}; t->listInnerType=inner; return t;
    }

    NbtTagPtr compoundGet(const std::string& key) const {
        if(type!=TagType::Compound) return nullptr;
        for(auto& [k,v]:std::get<CompoundPayload>(payload))
            if(k==key) return v;
        return nullptr;
    }
    void compoundSet(const std::string& key, NbtTagPtr val) {
        if(type!=TagType::Compound) return;
        auto& cp=std::get<CompoundPayload>(payload);
        for(auto& [k,v]:cp){ if(k==key){v=val;return;} }
        cp.emplace_back(key,val);
    }
    bool compoundRemove(const std::string& key) {
        if(type!=TagType::Compound) return false;
        auto& cp=std::get<CompoundPayload>(payload);
        for(auto it=cp.begin();it!=cp.end();++it)
            if(it->first==key){cp.erase(it);return true;}
        return false;
    }
    void listAdd(NbtTagPtr child) {
        if(type!=TagType::List) return;
        auto& lp=std::get<ListPayload>(payload);
        if(lp.empty()) listInnerType=child->type;
        lp.push_back(child);
    }
    NbtTagPtr clone() const;
};

} // namespace xnbt
