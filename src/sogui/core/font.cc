#include <sogui/sogui_internal.h>

#include <imgui.h>
#include <imgui_internal.h>

#include <climits>
#include <cstdint>
#include <cstdio>

namespace SoGui {
namespace {

constexpr float kDefaultEmSize = 14.0f; // 作为 ImGui 默认字体时的字号

struct FaceMetrics {
    float heightPerEm = 0.0f;
    float ascentRatio = 0.0f;
};

struct FontFile {
    void* data = nullptr;
    size_t size = 0;
    FaceMetrics metrics;
};

bool ReadU16(const unsigned char* data, size_t size, size_t offset, uint16_t& out) {
    if (offset > size || size - offset < 2)
        return false;
    out = static_cast<uint16_t>((data[offset] << 8) | data[offset + 1]);
    return true;
}

bool ReadU32(const unsigned char* data, size_t size, size_t offset, uint32_t& out) {
    if (offset > size || size - offset < 4)
        return false;
    out = (uint32_t{data[offset]} << 24) | (uint32_t{data[offset + 1]} << 16) | (uint32_t{data[offset + 2]} << 8) |
          uint32_t{data[offset + 3]};
    return true;
}

// 读取 head / hhea，与 ImGui 按像素高度映射字号的方式一致
bool ReadFaceMetrics(const unsigned char* data, size_t size, int index, FaceMetrics& out) {
    constexpr uint32_t kCollection = 0x74746366; // 'ttcf'
    constexpr uint32_t kHead = 0x68656164;       // 'head'
    constexpr uint32_t kHhea = 0x68686561;       // 'hhea'

    uint32_t tag = 0;
    size_t face = 0;
    if (index < 0 || !ReadU32(data, size, 0, tag))
        return false;
    if (tag == kCollection) {
        uint32_t count = 0;
        uint32_t offset = 0;
        if (!ReadU32(data, size, 8, count) || static_cast<uint32_t>(index) >= count ||
            !ReadU32(data, size, 12 + 4 * static_cast<size_t>(index), offset))
            return false;
        face = offset;
    } else if (index != 0) {
        return false;
    }

    uint16_t tableCount = 0;
    if (!ReadU16(data, size, face + 4, tableCount))
        return false;
    size_t head = 0;
    size_t hhea = 0;
    for (uint16_t i = 0; i < tableCount; ++i) {
        const size_t record = face + 12 + 16 * static_cast<size_t>(i);
        uint32_t tableTag = 0;
        uint32_t offset = 0;
        uint32_t length = 0;
        if (!ReadU32(data, size, record, tableTag) || !ReadU32(data, size, record + 8, offset) ||
            !ReadU32(data, size, record + 12, length) || offset > size || size - offset < length)
            return false;
        if (tableTag == kHead && length >= 54)
            head = offset;
        else if (tableTag == kHhea && length >= 36)
            hhea = offset;
    }

    uint16_t unitsPerEm = 0;
    uint16_t ascender = 0;
    uint16_t descender = 0;
    if (head == 0 || hhea == 0 || !ReadU16(data, size, head + 18, unitsPerEm) ||
        !ReadU16(data, size, hhea + 4, ascender) || !ReadU16(data, size, hhea + 6, descender))
        return false;
    const float ascent = static_cast<int16_t>(ascender);
    const float height = ascent - static_cast<int16_t>(descender);
    if (unitsPerEm == 0 || ascent <= 0.0f || height <= 0.0f)
        return false;
    out.heightPerEm = height / unitsPerEm;
    out.ascentRatio = ascent / height;
    return true;
}

bool LoadFontFile(const FontSource& source, FontFile& out) {
    if (source.path == nullptr || source.path[0] == '\0')
        return false;
    size_t size = 0;
    void* data = ImFileLoadToMemory(source.path, "rb", &size);
    if (data == nullptr)
        return false;
    // ImGui 要求字体数据大于 100 字节
    if (size <= 100 || size > static_cast<size_t>(INT_MAX) ||
        !ReadFaceMetrics(static_cast<const unsigned char*>(data), size, source.index, out.metrics)) {
        IM_FREE(data);
        return false;
    }
    out.data = data;
    out.size = size;
    return true;
}

ImFont* AddFontData(const FontFile& file, const FontSource& source, ImFontConfig& config) {
    const char* name = source.path;
    for (const char* p = source.path; *p != '\0'; ++p) {
        if (*p == '/' || *p == '\\')
            name = p + 1;
    }
    std::snprintf(config.Name, IM_ARRAYSIZE(config.Name), "%s", name);
    config.FontNo = static_cast<ImU32>(source.index);
    // 图集接管 data，失败时也由图集释放
    return ImGui::GetIO().Fonts->AddFontFromMemoryTTF(file.data, static_cast<int>(file.size), 0.0f, &config);
}

int IndexOf(FontWeight weight) {
    const int index = static_cast<int>(weight);
    IM_ASSERT(index >= 0 && index < kFontWeightCount && "Invalid FontWeight");
    return index;
}

Context& CurrentContext() {
    Context* ctx = GetCurrentContext();
    IM_ASSERT(ctx != nullptr && "No current SoGui context. Did you call SoGui::CreateContext()?");
    IM_ASSERT(ImGui::GetCurrentContext() != nullptr && "No current ImGui context");
    return *ctx;
}

} // namespace

// sogui_internal.h
const FontEntry* FindFont(FontWeight weight) {
    const FontEntry& entry = CurrentContext().fonts.entries[IndexOf(weight)];
    if (entry.font == nullptr || !ImGui::GetIO().Fonts->Fonts.contains(entry.font))
        return nullptr;
    return &entry;
}

const FontEntry* FindIconFont() {
    const FontEntry& entry = CurrentContext().fonts.iconEntry;
    if (entry.font == nullptr || !ImGui::GetIO().Fonts->Fonts.contains(entry.font))
        return nullptr;
    return &entry;
}

// font.h
bool AddFont(FontWeight weight, const FontSource& source, std::span<const FontSource> fallbacks) {
    const int index = IndexOf(weight);
    Context& ctx = CurrentContext();
    IM_ASSERT(!ImGui::GetIO().Fonts->Locked && "SoGui::AddFont() must be called outside of a frame");

    FontFile file;
    if (!LoadFontFile(source, file))
        return false;
    ImFontConfig config;
    config.SizePixels = kDefaultEmSize * file.metrics.heightPerEm;
    ImFont* font = AddFontData(file, source, config);
    if (font == nullptr)
        return false;

    for (const FontSource& fallback : fallbacks) {
        FontFile extra;
        if (!LoadFontFile(fallback, extra))
            continue;
        ImFontConfig merge;
        merge.MergeMode = true;
        // 让补字字体的 em 与主字体一致
        merge.SizePixels = config.SizePixels * extra.metrics.heightPerEm / file.metrics.heightPerEm;
        AddFontData(extra, fallback, merge);
    }

    ctx.fonts.entries[index] = {font, file.metrics.heightPerEm, file.metrics.ascentRatio};
    return true;
}

bool AddIconFont(const FontSource& source) {
    Context& ctx = CurrentContext();
    IM_ASSERT(!ImGui::GetIO().Fonts->Locked && "SoGui::AddIconFont() must be called outside of a frame");

    FontFile file;
    if (!LoadFontFile(source, file))
        return false;
    ImFontConfig config;
    config.SizePixels = kDefaultEmSize * file.metrics.heightPerEm;
    ImFont* font = AddFontData(file, source, config);
    if (font == nullptr)
        return false;
    ctx.fonts.iconEntry = {font, file.metrics.heightPerEm, file.metrics.ascentRatio};
    return true;
}

ImFont* GetFont(FontWeight weight) {
    const FontEntry* entry = FindFont(weight);
    return entry != nullptr ? entry->font : nullptr;
}

} // namespace SoGui
