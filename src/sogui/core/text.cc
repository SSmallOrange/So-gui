#include <sogui/sogui_internal.h>

#include <imgui.h>
#include <imgui_internal.h>

#include <cfloat>
#include <cstring>
#include <iterator>

namespace SoGui {
namespace {

struct TextStyleSpec {
    const char* name;
    FontWeight weight;
    float size;          // CSS px
    float lineHeight;    // CSS px
    float letterSpacing; // CSS px，暂未使用
    bool mono;           // 暂未使用：Geist Mono 接入前回退到 Sans
    bool wrap;           // Heading、Copy 为 true；Label、Button 为 false
};

constexpr FontWeight kRegular = FontWeight::Regular;
constexpr FontWeight kMedium = FontWeight::Medium;
constexpr FontWeight kSemiBold = FontWeight::SemiBold;

// 数值取自 Geist Typography 的计算样式，顺序与 TextStyle 一致
constexpr TextStyleSpec kTextStyles[] = {
    {"Heading72", kSemiBold, 72.0f, 72.0f, -4.32f, false, true},
    {"Heading64", kSemiBold, 64.0f, 64.0f, -3.84f, false, true},
    {"Heading56", kSemiBold, 56.0f, 56.0f, -3.36f, false, true},
    {"Heading48", kSemiBold, 48.0f, 56.0f, -2.88f, false, true},
    {"Heading40", kSemiBold, 40.0f, 48.0f, -2.4f, false, true},
    {"Heading32", kSemiBold, 32.0f, 40.0f, -1.28f, false, true},
    {"Heading24", kSemiBold, 24.0f, 32.0f, -0.96f, false, true},
    {"Heading20", kSemiBold, 20.0f, 26.0f, -0.4f, false, true},
    {"Heading16", kSemiBold, 16.0f, 24.0f, -0.32f, false, true},
    {"Heading14", kSemiBold, 14.0f, 20.0f, -0.28f, false, true},
    {"Button16", kMedium, 16.0f, 20.0f, 0.0f, false, false},
    {"Button14", kMedium, 14.0f, 20.0f, 0.0f, false, false},
    {"Button12", kMedium, 12.0f, 16.0f, 0.0f, false, false},
    {"Label20", kRegular, 20.0f, 32.0f, 0.0f, false, false},
    {"Label18", kRegular, 18.0f, 20.0f, 0.0f, false, false},
    {"Label16", kRegular, 16.0f, 20.0f, 0.0f, false, false},
    {"Label14", kRegular, 14.0f, 20.0f, 0.0f, false, false},
    {"Label14Mono", kRegular, 14.0f, 20.0f, 0.0f, true, false},
    {"Label13", kRegular, 13.0f, 16.0f, 0.0f, false, false},
    {"Label13Mono", kRegular, 13.0f, 20.0f, 0.0f, true, false},
    {"Label12", kRegular, 12.0f, 16.0f, 0.0f, false, false},
    {"Label12Mono", kRegular, 12.0f, 16.0f, 0.0f, true, false},
    {"Copy24", kRegular, 24.0f, 36.0f, 0.0f, false, true},
    {"Copy20", kRegular, 20.0f, 36.0f, 0.0f, false, true},
    {"Copy18", kRegular, 18.0f, 28.0f, 0.0f, false, true},
    {"Copy16", kRegular, 16.0f, 24.0f, 0.0f, false, true},
    {"Copy14", kRegular, 14.0f, 20.0f, 0.0f, false, true},
    {"Copy13", kRegular, 13.0f, 18.0f, 0.0f, false, true},
    {"Copy13Mono", kRegular, 13.0f, 18.0f, 0.0f, true, true},
};

static_assert(std::size(kTextStyles) == static_cast<size_t>(TextStyle::COUNT), "Every TextStyle needs a spec");

const TextStyleSpec& SpecOf(TextStyle style) {
    const int index = static_cast<int>(style);
    IM_ASSERT(index >= 0 && index < static_cast<int>(TextStyle::COUNT) && "Invalid TextStyle");
    return kTextStyles[index];
}

// 不能出现在行首 / 行尾的标点
constexpr char32_t kNoLineStart[] = U",.;:!?%)]}，。、；：！？）」』】》〉〕］｝”’…";
constexpr char32_t kNoLineEnd[] = U"([{（「『【《〈〔［｛“‘";

bool Contains(const char32_t* set, unsigned int c) {
    for (; *set != 0; ++set) {
        if (*set == c)
            return true;
    }
    return false;
}

bool IsBlank(unsigned int c) { return c == ' ' || c == '\t' || c == 0x3000; }

bool IsCjk(unsigned int c) {
    return (c >= 0x2E80 && c <= 0x9FFF) || (c >= 0xAC00 && c <= 0xD7AF) || (c >= 0xF900 && c <= 0xFAFF) ||
           (c >= 0xFE30 && c <= 0xFE4F) || (c >= 0xFF00 && c <= 0xFFEF);
}

bool CanBreakBefore(unsigned int prev, unsigned int c) {
    if (Contains(kNoLineStart, c) || Contains(kNoLineEnd, prev))
        return false;
    return IsCjk(prev) || IsCjk(c);
}

// 压入样式字体，并计算当前缩放下的行盒
class StyleScope {
  public:
    explicit StyleScope(TextStyle style) : spec_(SpecOf(style)) {
        IM_ASSERT(ImGui::GetCurrentWindowRead() != nullptr && "SoGui text must be used inside a window");
        const FontEntry* entry = FindFont(spec_.weight);
        const float heightPerEm = entry != nullptr ? entry->heightPerEm : 1.0f;
        ImGui::PushFont(entry != nullptr ? entry->font : nullptr, spec_.size * heightPerEm);
        font_ = ImGui::GetFont();
        baked_ = ImGui::GetFontBaked();
        pixelSize_ = ImGui::GetFontSize();

        const float ascent = entry != nullptr ? pixelSize_ * entry->ascentRatio : baked_->Ascent;
        const float content = entry != nullptr ? pixelSize_ : baked_->Ascent - baked_->Descent;
        metrics_.fontSize = pixelSize_ / heightPerEm;
        metrics_.lineHeight = spec_.lineHeight * GetScale();
        metrics_.baseline = IM_ROUND((metrics_.lineHeight - content) * 0.5f + ascent);
    }

    ~StyleScope() { ImGui::PopFont(); }

    StyleScope(const StyleScope&) = delete;
    StyleScope& operator=(const StyleScope&) = delete;

    const TextMetrics& Metrics() const { return metrics_; }
    ImFont* Font() const { return font_; }
    float PixelSize() const { return pixelSize_; }
    // ImGui 把基线放在绘制位置 + Ascent 处
    float DrawOffset() const { return metrics_.baseline - baked_->Ascent; }

    template <typename Fn> int ForEachLine(std::string_view text, float wrapWidth, Fn&& fn) const {
        const char* p = text.empty() ? "" : text.data();
        const char* end = p + text.size();
        int count = 0;
        for (;;) {
            const char* paragraphEnd = static_cast<const char*>(std::memchr(p, '\n', static_cast<size_t>(end - p)));
            if (paragraphEnd == nullptr)
                paragraphEnd = end;
            const char* line = p;
            do {
                const char* next = paragraphEnd;
                const char* lineEnd = wrapWidth > 0.0f ? WrapLine(line, paragraphEnd, wrapWidth, &next) : paragraphEnd;
                fn(line, lineEnd, count++);
                line = next;
            } while (line < paragraphEnd);
            if (paragraphEnd == end)
                return count;
            p = paragraphEnd + 1;
        }
    }

    ImVec2 Measure(std::string_view text, float wrapWidth) const {
        float width = 0.0f;
        const int lines = ForEachLine(text, wrapWidth, [&](const char* begin, const char* end, int) {
            if (begin != end)
                width = ImMax(width, font_->CalcTextSizeA(pixelSize_, FLT_MAX, 0.0f, begin, end).x);
        });
        return ImVec2(IM_TRUNC(width + 0.99999f), static_cast<float>(lines) * metrics_.lineHeight);
    }

  private:
    // 返回本行结束位置（不含行尾空白），*next 为下一行起点
    const char* WrapLine(const char* begin, const char* end, float wrapWidth, const char** next) const {
        const float scale = pixelSize_ / baked_->Size;
        float width = 0.0f;
        const char* breakEnd = nullptr;
        const char* breakNext = nullptr;
        const char* blankStart = nullptr;
        unsigned prev = 0;
        for (const char* p = begin; p < end;) {
            unsigned int c = 0;
            const char* q = p + ImMax(ImTextCharFromUtf8(&c, p, end), 1);
            if (c == '\r') {
                p = q;
                continue;
            }
            const bool blank = IsBlank(c);
            if (blank) {
                if (blankStart == nullptr)
                    blankStart = p;
            } else {
                const char* breakAt = blankStart != nullptr ? blankStart : p;
                if (breakAt > begin && (blankStart != nullptr || CanBreakBefore(prev, c))) {
                    breakEnd = breakAt;
                    breakNext = p;
                }
            }
            width += baked_->GetCharAdvance(static_cast<ImWchar>(c)) * scale;
            if (!blank) {
                if (width > wrapWidth && p > begin) {
                    *next = breakEnd != nullptr ? breakNext : p;
                    return breakEnd != nullptr ? breakEnd : p;
                }
                blankStart = nullptr;
            }
            prev = c;
            p = q;
        }
        *next = end;
        return blankStart != nullptr ? blankStart : end;
    }

    const TextStyleSpec& spec_;
    TextMetrics metrics_;
    ImFont* font_ = nullptr;
    ImFontBaked* baked_ = nullptr;
    float pixelSize_ = 0.0f;
};

} // namespace

void Text(std::string_view text, TextStyle style, const TextOptions& options) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;

    const float nativeAscent = ImGui::GetFontBaked()->Ascent;
    const float wrapWidth = SpecOf(style).wrap ? ImMax(ImGui::GetContentRegionAvail().x, 1.0f) : 0.0f;
    const StyleScope scope(style);
    const ImVec2 size = scope.Measure(text, wrapWidth);

    // 与原生 ImGui 文字按基线对齐
    const float textBaseline = ImMax(0.0f, scope.Metrics().baseline - nativeAscent);
    ImVec2 pos = window->DC.CursorPos;
    pos.y += ImMax(0.0f, window->DC.CurrLineTextBaseOffset - textBaseline);
    ImGui::ItemSize(size, textBaseline);
    if (!ImGui::ItemAdd(ImRect(pos, ImVec2(pos.x + size.x, pos.y + size.y)), 0))
        return;

    const ImU32 color = GetColorU32(options.color);
    const float lineHeight = scope.Metrics().lineHeight;
    const float drawOffset = scope.DrawOffset();
    const bool logging = GImGui->LogEnabled;
    scope.ForEachLine(text, wrapWidth, [&](const char* begin, const char* end, int line) {
        if (begin == end)
            return;
        const ImVec2 linePos(pos.x, pos.y + static_cast<float>(line) * lineHeight + drawOffset);
        window->DrawList->AddText(scope.Font(), scope.PixelSize(), linePos, color, begin, end);
        if (logging)
            ImGui::LogRenderedText(&linePos, begin, end);
    });
}

ImVec2 CalcTextSize(std::string_view text, TextStyle style, float wrapWidth) {
    const StyleScope scope(style);
    return scope.Measure(text, wrapWidth);
}

// sogui_internal.h
void RenderText(ImDrawList* drawList, ImVec2 pos, std::string_view text, TextStyle style, ImU32 color) {
    if (text.empty())
        return;
    const StyleScope scope(style);
    const ImVec2 linePos(pos.x, pos.y + scope.DrawOffset());
    drawList->AddText(scope.Font(), scope.PixelSize(), linePos, color, text.data(), text.data() + text.size());
    if (GImGui->LogEnabled)
        ImGui::LogRenderedText(&linePos, text.data(), text.data() + text.size());
}

float RenderTextWrapped(ImDrawList* drawList, ImVec2 pos, std::string_view text, TextStyle style, float wrapWidth,
                        ImU32 color) {
    if (text.empty())
        return 0.0f;
    const StyleScope scope(style);
    const float lineHeight = scope.Metrics().lineHeight;
    const float drawOffset = scope.DrawOffset();
    const int lines = scope.ForEachLine(text, wrapWidth, [&](const char* begin, const char* end, int line) {
        if (begin == end)
            return;
        const ImVec2 linePos(pos.x, pos.y + static_cast<float>(line) * lineHeight + drawOffset);
        drawList->AddText(scope.Font(), scope.PixelSize(), linePos, color, begin, end);
        if (GImGui->LogEnabled)
            ImGui::LogRenderedText(&linePos, begin, end);
    });
    return static_cast<float>(lines) * lineHeight;
}

TextMetrics GetTextMetrics(TextStyle style) {
    const StyleScope scope(style);
    return scope.Metrics();
}

const char* GetTextStyleName(TextStyle style) { return SpecOf(style).name; }

} // namespace SoGui
