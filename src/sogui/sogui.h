#pragma once

#include <imgui.h>

static_assert(IMGUI_VERSION_NUM >= 19200, "So-gui requires Dear ImGui 1.92 or newer");

#include <sogui/core/anim.h>
#include <sogui/core/context.h>
#include <sogui/core/draw.h>
#include <sogui/core/font.h>
#include <sogui/core/icon.h>
#include <sogui/core/metrics.h>
#include <sogui/core/scale.h>
#include <sogui/core/text.h>
#include <sogui/core/theme.h>
#include <sogui/demo/sogui_demo.h>
#include <sogui/widgets/badge.h>
#include <sogui/widgets/button.h>
#include <sogui/widgets/note.h>
#include <sogui/widgets/separator.h>
#include <sogui/widgets/skeleton.h>
#include <sogui/widgets/spinner.h>
