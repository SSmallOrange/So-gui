#!/usr/bin/env python3
"""把 Noto Sans SC 裁剪为常用汉字子集，作为 SoGui 自带的中文补字字体（Python 3.9+，需要 fontTools）。

源字体固定为 notofonts/noto-cjk 标签 Sans2.004 的静态 OTF，按 SHA-256 校验：
  https://cdn.jsdelivr.net/gh/notofonts/noto-cjk@Sans2.004/Sans/SubsetOTF/SC/NotoSansSC-Regular.otf
  https://cdn.jsdelivr.net/gh/notofonts/noto-cjk@Sans2.004/Sans/SubsetOTF/SC/NotoSansSC-Medium.otf

字符集：GB2312 汉字（默认一级 + 二级共 6763 个）、GB2312 符号区，以及常用标点和全角字符。
ImGui 不做 shaping，因此删除 GSUB / GPOS 等排版表和 hinting。
"""

import argparse
import hashlib
import sys
from pathlib import Path
from typing import Iterable, List, Set


ROOT = Path(__file__).resolve().parents[2]
SOURCES = {
    "NotoSansSC-Regular.otf": "faa6c9df652116dde789d351359f3d7e5d2285a2b2a1f04a2d7244df706d5ea9",
    "NotoSansSC-Medium.otf": "7633f5a016d4dd95e685a69633d818aabc4644c4b08e26bd35b1b30c45ed5dda",
}
# EUC-CN 行号：符号区 0xA1-0xA9，一级汉字 0xB0-0xD7，二级汉字 0xD8-0xF7
SYMBOL_ROWS = (0xA1, 0xA9)
HANZI_ROWS = {1: (0xB0, 0xD7), 2: (0xB0, 0xF7)}
HANZI_COUNT = {1: 3755, 2: 6763}
EXTRA_RANGES = (
    (0x0020, 0x00FF),  # ASCII 与 Latin-1
    (0x2000, 0x206F),  # 通用标点：“”‘’…—
    (0x3000, 0x303F),  # 中日韩符号和标点：、。《》【】
    (0xFF00, 0xFFEF),  # 全角 ASCII 与全角标点
    (0xFFFD, 0xFFFD),  # 缺字时 ImGui 优先显示的替换符
)


def gb2312_chars(first_row: int, last_row: int) -> Iterable[int]:
    for row in range(first_row, last_row + 1):
        for cell in range(0xA1, 0xFF):
            try:
                char = bytes((row, cell)).decode("gb2312")
            except UnicodeDecodeError:
                continue
            yield ord(char)


def build_charset(level: int) -> Set[int]:
    hanzi = set(gb2312_chars(*HANZI_ROWS[level]))
    if len(hanzi) != HANZI_COUNT[level]:
        raise ValueError(f"GB2312 汉字数量异常：{len(hanzi)}，应为 {HANZI_COUNT[level]}")
    charset = hanzi | set(gb2312_chars(*SYMBOL_ROWS))
    for first, last in EXTRA_RANGES:
        charset.update(range(first, last + 1))
    return charset


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as file:
        for chunk in iter(lambda: file.read(1 << 20), b""):
            digest.update(chunk)
    return digest.hexdigest()


def subset_options(subset):
    options = subset.Options()
    options.layout_features = []
    options.drop_tables += ["GSUB", "GPOS", "GDEF", "BASE", "vhea", "vmtx", "VORG", "DSIG"]
    options.name_IDs = [0, 1, 2, 3, 4, 5, 6, 13, 14]  # 保留版权与许可证信息
    options.hinting = False
    options.notdef_outline = True
    options.recalc_timestamp = False  # 相同输入得到相同输出
    return options


def subset_font(subset, source: Path, output: Path, charset: Set[int]) -> List[int]:
    options = subset_options(subset)
    font = subset.load_font(str(source), options)
    try:
        subsetter = subset.Subsetter(options)
        subsetter.populate(unicodes=sorted(charset))
        subsetter.subset(font)
        subset.save_font(font, str(output), options)
        cmap = font.getBestCmap()
        return [code for code in sorted(charset) if code not in cmap]
    finally:
        font.close()


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--src", type=Path, required=True, help="存放原始 NotoSansSC-*.otf 的目录")
    parser.add_argument("--output-dir", type=Path, default=ROOT / "assets" / "fonts" / "noto-sans-sc",
                        help="子集字体输出目录")
    parser.add_argument("--level", type=int, choices=sorted(HANZI_ROWS), default=2,
                        help="GB2312 汉字级别：1 = 一级常用字 3755 个，2 = 一级 + 二级共 6763 个")
    args = parser.parse_args(argv)

    try:
        from fontTools import subset
    except ImportError:
        print("错误：缺少 fontTools，请先执行 python3 -m pip install fonttools", file=sys.stderr)
        return 1

    try:
        charset = build_charset(args.level)
        hanzi = set(gb2312_chars(*HANZI_ROWS[args.level]))
        args.output_dir.mkdir(parents=True, exist_ok=True)
        for name, expected in SOURCES.items():
            source = args.src / name
            output = args.output_dir / name
            if not source.is_file():
                print(f"错误：找不到源字体：{source}", file=sys.stderr)
                return 1
            if sha256(source) != expected:
                print(f"错误：源字体校验失败，需要 Sans2.004 版本：{source}", file=sys.stderr)
                return 1
            if output.exists() and output.resolve() == source.resolve():
                print("错误：输出路径不能覆盖源字体", file=sys.stderr)
                return 1
            missing = subset_font(subset, source, output, charset)
            missing_hanzi = [code for code in missing if code in hanzi]
            if missing_hanzi:
                print(f"错误：{name} 缺少 {len(missing_hanzi)} 个 GB2312 汉字", file=sys.stderr)
                return 1
            size = output.stat().st_size
            print(f"已生成：{output}（{size / 1024:.0f} KiB，汉字 {len(hanzi)} 个）")
    except (OSError, ValueError) as error:
        print(f"错误：{error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
