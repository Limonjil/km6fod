#!/usr/bin/env python3
"""AArch64 .ko/disassembler helper: symbols, strings, annotated disassembly.

objdump is not installed on this workstation, so this uses pyelftools + capstone
and resolves relocations (a relocatable .ko calls through R_AARCH64_CALL26 etc.).
"""
import argparse
import re
import sys

from capstone import Cs, CS_ARCH_ARM64, CS_MODE_LITTLE_ENDIAN
from elftools.elf.elffile import ELFFile
from elftools.elf.relocation import RelocationSection

try:
    from elftools.elf.enums import ENUM_RELOC_TYPE_AARCH64 as _RA
    RELOC_NUM = {v: k.replace("R_AARCH64_", "") for k, v in _RA.items()}
except Exception:  # pragma: no cover - older pyelftools
    RELOC_NUM = {}

RELOC = {
    "R_AARCH64_CALL26": "bl", "R_AARCH64_JUMP26": "b",
    "R_AARCH64_ADR_PREL_PG_HI21": "adrp", "R_AARCH64_ADD_ABS_LO12_NC": ":lo12:",
    "R_AARCH64_LDST64_ABS_LO12_NC": ":lo12:", "R_AARCH64_LDST32_ABS_LO12_NC": ":lo12:",
    "R_AARCH64_LDST16_ABS_LO12_NC": ":lo12:", "R_AARCH64_LDST8_ABS_LO12_NC": ":lo12:",
    "R_AARCH64_TSTBR14": "tbz", "R_AARCH64_CONDBR19": "b.cond",
    "R_AARCH64_ABS64": ".quad", "R_AARCH64_ABS32": ".long", "R_AARCH64_PREL32": ".word",
    "R_AARCH64_PREL64": ".quad",
}


class Ko:
    def __init__(self, path):
        self.f = ELFFile(open(path, "rb"))
        self.symtab = self.f.get_section_by_name(".symtab")
        self.syms = [s for s in self.symtab.iter_symbols()]
        self.by_addr = {}
        for s in self.syms:
            if s.name and s["st_value"] and s["st_info"]["type"] != "STT_NOTYPE":
                self.by_addr.setdefault(s["st_value"], s)
        # relocation maps: section index -> {offset: "sym+addend"}
        self.relocs = {}
        for sec in self.f.iter_sections():
            if not isinstance(sec, RelocationSection) or not sec.name.startswith(".rela"):
                continue
            target = self.f.get_section(sec["sh_info"]).name
            m = {}
            for r in sec.iter_relocations():
                info = self.symtab.get_symbol(r["r_info_sym"])
                addend = r["r_addend"] if r.is_RELA() else 0
                name = info.name
                if not name:
                    try:
                        name = self.f.get_section(info["st_shndx"]).name
                    except Exception:
                        name = f"<sec{info['st_shndx']}>"
                if addend:
                    name += f"+{addend:#x}"
                kind = RELOC.get(r["r_info_type"]) or RELOC_NUM.get(r["r_info_type"], "?")
                m[r["r_offset"]] = (kind, name)
            self.relocs[target] = m

    def section(self, name):
        return self.f.get_section_by_name(name)

    def func_span(self, sym):
        """Symbol name -> (section name, start offset, end offset)."""
        if sym["st_info"]["type"] != "STT_FUNC":
            return None
        sec = self.f.get_section(sym["st_shndx"])
        start = sym["st_value"] - sec["sh_addr"]
        size = sym["st_size"]
        if not size:
            funcs = sorted(s["st_value"] - sec["sh_addr"] for s in self.syms
                           if s["st_shndx"] == sym["st_shndx"] and s["st_info"]["type"] == "STT_FUNC"
                           and s["st_value"] > sym["st_value"])
            size = (funcs[0] - start) if funcs else 512
        return sec.name, start, start + size

    def find(self, pattern):
        rx = re.compile(pattern, re.I)
        return [s for s in self.syms if s.name and s["st_info"]["type"] != "STT_SECTION" and rx.search(s.name)]


def dump(ko, sym, addr_filter=None):
    span = ko.func_span(sym)
    if not span:
        print(f"!! {sym.name}: not a function (type={sym['st_info']['type']})")
        return
    sect, start, end = span
    data = ko.section(sect).data()[start:end]
    rel = ko.relocs.get(sect, {})
    md = Cs(CS_ARCH_ARM64, CS_MODE_LITTLE_ENDIAN)
    md.detail = False
    print(f"===== {sym.name}  ({sect}+{start:#x} .. {end:#x}, {end - start} bytes) =====")
    for ins in md.disasm(data, start):
        line = f"{ins.address:06x}: {ins.mnemonic:<8} {ins.op_str}"
        ops = ins.op_str
        target = None
        m = re.match(r"#(-?0x[0-9a-f]+|-?\d+)$", ops)
        if ins.mnemonic in ("bl", "b") and m:
            try:
                target = int(m.group(1), 0)
            except ValueError:
                target = None
            if target is not None:
                named = rel.get(target)
                if named:
                    line += f"   ; {named[1]}"
        r = rel.get(ins.address)
        if r:
            line += f"   ; {r[0]} {r[1]}"
        print(line)


def strings(ko, pattern):
    rx = re.compile(pattern, re.I)
    buf = ko.section(".rodata")
    data = buf.data()
    base = buf["sh_addr"]
    for m in re.finditer(rb"[ -~]{4,}", data):
        s = m.group().decode()
        if rx.search(s):
            print(f"{base + m.start():06x}  {s}")


def xrefs(ko, addr_hint, width=8):
    """Find instructions in .text referencing the given absolute address."""
    sect = ko.section(".text")
    data = sect.data()
    lo, hi = addr_hint & ~0xFFF, addr_hint
    md = Cs(CS_ARCH_ARM64, CS_MODE_LITTLE_ENDIAN)
    hits = []
    for ins in md.disasm(data, 0):
        if ins.mnemonic == "adrp" and ins.op_str.split(",")[-1].strip().startswith("#0x"):
            try:
                if int(ins.op_str.split(",")[-1].strip().lstrip("#"), 16) == lo:
                    hits.append(ins.address)
            except ValueError:
                pass
    return hits


def refs(ko, spec):
    """spec = "section:addend" -> list functions whose relocations point at it."""
    sect, addend = spec.split(":")
    addend = int(addend, 0)
    wanted = {addend, addend & ~0xFFF}
    text = ko.section(".text")
    rel = ko.relocs.get(".text", {})
    out = []
    for off, (kind, name) in rel.items():
        base, _, add = name.partition("+")
        if base != sect:
            continue
        a = int(add, 16) if add else 0
        if a in wanted:
            out.append((off, name))
    for off, name in sorted(out):
        for s in ko.syms:
            if s["st_info"]["type"] != "STT_FUNC":
                continue
            span = ko.func_span(s)
            if span and span[0] == ".text" and span[1] <= off < span[2]:
                print(f"{off:06x} {name:<24} in {s.name}+{off - span[1]:#x}")
                break
        else:
            print(f"{off:06x} {name:<24} in <no symbol>")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("ko")
    ap.add_argument("what", nargs="?", default="")
    ap.add_argument("--list", action="store_true", help="list matching symbols")
    ap.add_argument("--str", action="store_true", help="search .rodata strings")
    ap.add_argument("--xref", action="store_true", help="find adrp xrefs to an address")
    ap.add_argument("--refs", action="store_true", help="who references section:addend, e.g. .rodata:0x1baf")
    a = ap.parse_args()
    ko = Ko(a.ko)
    if a.list:
        for s in ko.find(a.what):
            print(f"{s['st_value']:08x} {s['st_size']:6d} {s['st_info']['type']:<12} {s.name}")
        return
    if a.str:
        strings(ko, a.what)
        return
    if a.refs:
        refs(ko, a.what)
        return
    if a.xref:
        addrs = xrefs(ko, int(a.what, 16))
        for s in ko.syms:
            span = ko.func_span(s) if s["st_info"]["type"] == "STT_FUNC" else None
            if not span:
                continue
            sec, start, end = span
            hit = [x for x in addrs if start <= x < end]
            if hit:
                print(f"{s.name}  @ {[hex(h) for h in hit]}")
        return
    for s in ko.find("^" + re.escape(a.what) + "$"):
        dump(ko, s)
        return
    for s in ko.find(a.what):
        dump(ko, s)


if __name__ == "__main__":
    sys.exit(main())
