#### PlainRex
> .rela/.rel,
> .dynamic
> .symtab's, .dynsym's,
> .relr
> .hash/.gnu.hash
.gnu.version, .gnu.version_r, .gnu.version_d
> insn BasicBlock's;
> .strab's, .unknown_header,

## stuff insn touches
> .data, .rodata, .data.rel.ro
> .got, .got.plt
> (.init/.fini/.pre_init) array
> .eh_frame/exception tables, TLS, C++ vtables, RTTI structs

> # Ehdr(entry, ph/sh - off, ph/sh - num), Phdr, Shdr(.sh_link, .sh_info, .sh_name)

