#!/usr/bin/env python3
"""从 Geist 导出的 CSS 变量生成 sRGB 色表（方案 A，Python 3.9+）。"""

import argparse
import colorsys
import json
import math
import re
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, Tuple


ROOT = Path(__file__).resolve().parents[2]
HUES = ("gray", "gray-alpha", "blue", "red", "amber", "green", "teal", "purple", "pink")
RGBA = Tuple[float, float, float, float]
NUMBER = re.compile(r"[+-]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][+-]?\d+)?")


@dataclass(frozen=True)
class Color:
    key: str
    source: str
    rgba: RGBA


@dataclass(frozen=True)
class Palette:
    background: Tuple[Color, ...]
    scales: Tuple[Tuple[Color, ...], ...]


def number(value: str) -> float:
    if not NUMBER.fullmatch(value.strip()):
        raise ValueError("需要有限数值")
    result = float(value)
    if not math.isfinite(result):
        raise ValueError("需要有限数值")
    return result


def unit_value(value: str, percent_required: bool = False) -> float:
    value = value.strip()
    if value.endswith("%"):
        result = number(value[:-1]) / 100.0
    elif percent_required:
        raise ValueError("饱和度和亮度必须使用百分比")
    else:
        result = number(value)
    if not 0.0 <= result <= 1.0:
        raise ValueError("分量超出范围 0~1（或 0%~100%）")
    return result


def parse_color(value: str) -> RGBA:
    """接受 HSL 分量列表、逗号形式的 hsl/hsla 和十六进制颜色。"""
    if not isinstance(value, str) or not value.strip():
        raise ValueError("色值必须是非空字符串")
    value = value.strip()
    if value.startswith("#"):
        digits = value[1:]
        if len(digits) not in (3, 4, 6, 8) or not re.fullmatch(r"[0-9a-fA-F]+", digits):
            raise ValueError("无效的十六进制色值")
        if len(digits) in (3, 4):
            digits = "".join(character * 2 for character in digits)
        if len(digits) == 6:
            digits += "ff"
        return tuple(int(digits[index:index + 2], 16) / 255.0 for index in range(0, 8, 2))

    match = re.fullmatch(r"(hsl|hsla)\((.*)\)", value, re.IGNORECASE | re.DOTALL)
    components = (match.group(2) if match else value).split(",")
    expected = 4 if match and match.group(1).lower() == "hsla" else 3
    if len(components) != expected:
        raise ValueError("仅支持 HSL 分量列表、hsl()/hsla()（逗号分隔）和十六进制色值")
    hue_text = components[0].strip()
    if hue_text.endswith("deg"):
        hue_text = hue_text[:-3]
    hue = (number(hue_text) % 360.0) / 360.0
    saturation = unit_value(components[1], percent_required=True)
    lightness = unit_value(components[2], percent_required=True)
    alpha = unit_value(components[3]) if expected == 4 else 1.0
    red, green, blue = colorsys.hls_to_rgb(hue, lightness, saturation)
    return red, green, blue, alpha


def unique_object(pairs):
    result = {}
    for key, value in pairs:
        if key in result:
            raise ValueError(f"重复的 JSON key：{key}")
        result[key] = value
    return result


def load_tokens(path: Path) -> Dict[str, str]:
    with path.open(encoding="utf-8") as source:
        tokens = json.load(source, object_pairs_hook=unique_object)
    if not isinstance(tokens, dict):
        raise ValueError("JSON 顶层必须是对象")
    return tokens


def scale_key(hue: str, level: int) -> str:
    suffix = "" if hue == "gray-alpha" else "-value"
    return f"--ds-{hue}-{level * 100}{suffix}"


def build_palette(tokens: Dict[str, str]) -> Palette:
    backgrounds = ("--ds-background-100-value", "--ds-background-200-value")
    scales = tuple(tuple(scale_key(hue, level) for level in range(1, 11)) for hue in HUES)
    required = backgrounds + tuple(key for scale in scales for key in scale)
    missing = [key for key in required if key not in tokens]
    if missing:
        raise ValueError("缺少变量：" + ", ".join(missing))

    def color(key: str) -> Color:
        value = tokens[key]
        try:
            rgba = parse_color(value)
        except ValueError as error:
            raise ValueError(f"{key}：{error}") from error
        return Color(key, " ".join(value.split()), rgba)

    return Palette(tuple(color(key) for key in backgrounds),
                   tuple(tuple(color(key) for key in scale) for scale in scales))


def contrast_ratio(foreground: RGBA, background: RGBA) -> float:
    """半透明前景先在 sRGB 中与不透明背景合成，再计算 WCAG 对比度。"""
    def luminance(rgb):
        linear = [value / 12.92 if value <= 0.04045 else ((value + 0.055) / 1.055) ** 2.4
                  for value in rgb]
        return sum(value * weight for value, weight in zip(linear, (0.2126, 0.7152, 0.0722)))

    blended = tuple(foreground[index] * foreground[3] + background[index] * (1.0 - foreground[3])
                    for index in range(3))
    first, second = luminance(blended), luminance(background[:3])
    return (max(first, second) + 0.05) / (min(first, second) + 0.05)


def report_contrast(theme: str, palette: Palette) -> None:
    print(f"{theme} 对比度（Background 1 / 2，正文参考阈值 4.5）：")
    for hue, scale in zip(HUES, palette.scales):
        for color in scale:
            if hue != "gray-alpha" and color.rgba[3] != 1.0:
                print(f"警告：{theme} {color.key} 不是不透明颜色", file=sys.stderr)
        for level in (9, 10):
            color = scale[level - 1]
            ratios = [contrast_ratio(color.rgba, background.rgba) for background in palette.background]
            print(f"  {hue:10} {level:2}  {ratios[0]:5.2f} / {ratios[1]:5.2f}")
            if min(ratios) < 4.5:
                print(f"警告：{theme} {color.key} 对比度 {ratios[0]:.2f} / {ratios[1]:.2f} < 4.5",
                      file=sys.stderr)


def color_line(color: Color, indent: int) -> str:
    rounded = tuple(round(value, 4) for value in color.rgba)
    values = ", ".join(f"{value:.4f}f" for value in rounded)
    hexadecimal = "#" + "".join(f"{math.floor(value * 255.0 + 0.5):02x}" for value in rounded)
    return f"{' ' * indent}{{{values}}},  // {color.key} {hexadecimal} | {color.source}"


def render_palettes(light: Palette, dark: Palette) -> str:
    lines = [
        "// Generated by tools/tokengen/tokengen.py; do not edit.",
        "// Sources: tokens/geist/light.json, tokens/geist/dark.json (sRGB -value / Gray Alpha).",
        "// Palette requires RGBA background[2] and RGBA scales[9][10].",
        "// clang-format off",
    ]
    for name, palette in (("Light", light), ("Dark", dark)):
        lines += [f"static constexpr Palette kPalette{name} = {{", "    .background = {"]
        lines += [color_line(color, 8) for color in palette.background]
        lines += ["    },", "    .scales = {"]
        for hue, scale in zip(HUES, palette.scales):
            lines.append(f"        {{ // {hue}")
            lines += [color_line(color, 12) for color in scale]
            lines.append("        },")
        lines += ["    },", "};", ""]
    lines.append("// clang-format on")
    return "\n".join(lines) + "\n"


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input-dir", type=Path, default=ROOT / "tokens" / "geist",
                        help="light.json 和 dark.json 所在目录")
    parser.add_argument("--output", type=Path,
                        default=ROOT / "src" / "sogui" / "core" / "generated" / "tokens_geist.inc",
                        help="C++ 色表输出路径")
    parser.add_argument("--check", action="store_true", help="只检查是否过期，不写文件")
    args = parser.parse_args(argv)

    palettes = []
    errors = []
    for theme in ("light", "dark"):
        path = args.input_dir / f"{theme}.json"
        try:
            palette = build_palette(load_tokens(path))
        except (OSError, ValueError) as error:
            errors.append(f"{path}：{error}")
        else:
            palettes.append(palette)
    if errors:
        for error in errors:
            print(f"错误：{error}", file=sys.stderr)
        return 1

    content = render_palettes(*palettes)
    try:
        sources = [args.input_dir / f"{theme}.json" for theme in ("light", "dark")]
        if any(args.output.resolve() == source.resolve() or
               (args.output.exists() and args.output.samefile(source)) for source in sources):
            print("错误：输出路径不能覆盖输入 JSON", file=sys.stderr)
            return 1
        if args.check:
            if not args.output.is_file() or args.output.read_text(encoding="utf-8") != content:
                print(f"错误：生成结果缺失或过期：{args.output}", file=sys.stderr)
                return 1
            print(f"生成结果一致：{args.output}")
        else:
            args.output.parent.mkdir(parents=True, exist_ok=True)
            with args.output.open("w", encoding="utf-8", newline="\n") as output:
                output.write(content)
            print(f"已生成：{args.output}（亮色 / 暗色各 92 个颜色）")
    except (OSError, ValueError) as error:
        print(f"错误：{args.output}：{error}", file=sys.stderr)
        return 1

    for theme, palette in zip(("light", "dark"), palettes):
        report_contrast(theme, palette)
    return 0


if __name__ == "__main__":
    sys.exit(main())
