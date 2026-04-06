#pragma once
#include "NbtTypes.hpp"
#include <span>
#include <cstring>
#include <stdexcept>

namespace xnbt {

class BinaryReader {
public:
    explicit BinaryReader(std::span<const uint8_t> data):mData(data),mPos(0){}
    template<typename T> T read(){
        if(mPos+sizeof(T)>mData.size()) throw std::runtime_error("unexpected end");
        T val; std::memcpy(&val,mData.data()+mPos,sizeof(T)); mPos+=sizeof(T); return val;
    }
    std::string readString(){
        uint16_t len=read<uint16_t>();
        std::string s(reinterpret_cast<const char*>(mData.data()+mPos),len);
        mPos+=len; return s;
    }
    bool eof() const { return mPos>=mData.size(); }
private:
    std::span<const uint8_t> mData; size_t mPos;
};

class BinaryWriter {
public:
    template<typename T> void write(T val){
        const uint8_t* p=reinterpret_cast<const uint8_t*>(&val);
        mBuf.insert(mBuf.end(),p,p+sizeof(T));
    }
    void writeString(const std::string& s){
        write<uint16_t>((uint16_t)s.size());
        mBuf.insert(mBuf.end(),s.begin(),s.end());
    }
    std::vector<uint8_t> take(){ return std::move(mBuf); }
private:
    std::vector<uint8_t> mBuf;
};

class NbtParser {
public:
    static NbtTagPtr parse(std::span<const uint8_t> data){
        BinaryReader r(data); return readNamedTag(r);
    }
    static std::vector<uint8_t> serialize(const std::string& name, const NbtTagPtr& tag){
        BinaryWriter w; writeNamedTag(w,name,tag); return w.take();
    }
private:
    static NbtTagPtr readNamedTag(BinaryReader& r){
        uint8_t typeId=r.read<uint8_t>();
        if(typeId==0){ auto t=std::make_shared<NbtTag>(); t->type=TagType::End; return t; }
        r.readString();
        return readPayload(r,static_cast<TagType>(typeId));
    }
    static NbtTagPtr readPayload(BinaryReader& r, TagType type){
        auto tag=std::make_shared<NbtTag>(); tag->type=type;
        switch(type){
            case TagType::Byte:   tag->payload=r.read<int8_t>();  break;
            case TagType::Short:  tag->payload=r.read<int16_t>(); break;
            case TagType::Int:    tag->payload=r.read<int32_t>(); break;
            case TagType::Long:   tag->payload=r.read<int64_t>(); break;
            case TagType::Float:  tag->payload=r.read<float>();   break;
            case TagType::Double: tag->payload=r.read<double>();  break;
            case TagType::String: tag->payload=r.readString();    break;
            case TagType::ByteArray:{ int32_t l=r.read<int32_t>(); ByteArrayPayload a(l); for(auto& b:a) b=r.read<int8_t>(); tag->payload=std::move(a); break; }
            case TagType::IntArray:{ int32_t l=r.read<int32_t>(); IntArrayPayload a(l); for(auto& v:a) v=r.read<int32_t>(); tag->payload=std::move(a); break; }
            case TagType::LongArray:{ int32_t l=r.read<int32_t>(); LongArrayPayload a(l); for(auto& v:a) v=r.read<int64_t>(); tag->payload=std::move(a); break; }
            case TagType::List:{
                uint8_t it=r.read<uint8_t>(); int32_t l=r.read<int32_t>();
                tag->listInnerType=static_cast<TagType>(it);
                ListPayload lp; lp.reserve(l);
                for(int32_t i=0;i<l;i++) lp.push_back(readPayload(r,tag->listInnerType));
                tag->payload=std::move(lp); break;
            }
            case TagType::Compound:{
                CompoundPayload cp;
                while(true){ uint8_t ct=r.read<uint8_t>(); if(ct==0) break;
                    std::string k=r.readString();
                    cp.emplace_back(k,readPayload(r,static_cast<TagType>(ct))); }
                tag->payload=std::move(cp); break;
            }
            default: break;
        }
        return tag;
    }
    static void writeNamedTag(BinaryWriter& w, const std::string& name, const NbtTagPtr& tag){
        if(!tag||tag->type==TagType::End){w.write<uint8_t>(0);return;}
        w.write<uint8_t>((uint8_t)tag->type); w.writeString(name); writePayload(w,tag);
    }
    static void writePayload(BinaryWriter& w, const NbtTagPtr& tag){
        switch(tag->type){
            case TagType::Byte:   w.write(std::get<int8_t>(tag->payload));   break;
            case TagType::Short:  w.write(std::get<int16_t>(tag->payload));  break;
            case TagType::Int:    w.write(std::get<int32_t>(tag->payload));  break;
            case TagType::Long:   w.write(std::get<int64_t>(tag->payload));  break;
            case TagType::Float:  w.write(std::get<float>(tag->payload));    break;
            case TagType::Double: w.write(std::get<double>(tag->payload));   break;
            case TagType::String: w.writeString(std::get<std::string>(tag->payload)); break;
            case TagType::ByteArray:{ auto& a=std::get<ByteArrayPayload>(tag->payload); w.write<int32_t>(a.size()); for(auto b:a) w.write(b); break; }
            case TagType::IntArray:{ auto& a=std::get<IntArrayPayload>(tag->payload); w.write<int32_t>(a.size()); for(auto v:a) w.write(v); break; }
            case TagType::LongArray:{ auto& a=std::get<LongArrayPayload>(tag->payload); w.write<int32_t>(a.size()); for(auto v:a) w.write(v); break; }
            case TagType::List:{ auto& l=std::get<ListPayload>(tag->payload); w.write<uint8_t>((uint8_t)tag->listInnerType); w.write<int32_t>(l.size()); for(auto& c:l) writePayload(w,c); break; }
            case TagType::Compound:{ auto& cp=std::get<CompoundPayload>(tag->payload); for(auto& [k,c]:cp){w.write<uint8_t>((uint8_t)c->type);w.writeString(k);writePayload(w,c);} w.write<uint8_t>(0); break; }
            default: break;
        }
    }
};

} // namespace xnbt
