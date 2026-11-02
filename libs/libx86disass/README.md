# x86_disass
A Disassembler for x86_64 architecture, supporting instruction rewriting, wrote in C&amp;C++
Originally written in C, later on, a C++ API has been introduced. (see at [cpp_uapi.hpp](https://github.com/HeX0Rci5T/x86_disass/blob/main/uapi/cpp_uapi.hpp))

---
```c++
insn_t in("\xe9\xad\xde\x00\x00");
uint64_t dest = in.PtrAddr(0x55555555050);
// [rip+0xdead]   -> 0x55555555050+0xdead+<instruction size>
// [rax]          -> -1
in.stick_ptr(0xcafe, ReAsmT::FXD);
in[0]->Value() // ->[rip+0xcafe]
```
---

###### sample output
```
55                            PUSH    RBP
48 89 e5                      MOV     RBP, RSP
53                            PUSH    RBX
48 83 ec 28                   SUB     RSP, 0x28
66 0f ef c0                   PXOR    MMX0, MMX0
0f 29 45 d0                   MOVAPS  XMM ptr [RBP-0x30], XMM0
66 0f d6 45 e0                MOVQ    QWORD ptr [RBP-0x20], XMM0
48 8d 45 d0                   LEA     RAX, QWORD ptr [RBP-0x30]
48 89 c7                      MOV     RDI, RAX
e8 1f 02 00 00                CALL    [RIP+0x21f]
48 8d 45 d0                   LEA     RAX, QWORD ptr [RBP-0x30]
ba 50 00 00 00                MOV     EDX, 0x50
48 8d 0d ce ff ff ff          LEA     RCX, QWORD ptr [RIP-0x32]
48 89 ce                      MOV     RSI, RCX
48 89 c7                      MOV     RDI, RAX
e8 36 00 00 00                CALL    [RIP+0x36]
bb 00 00 00 00                MOV     EBX, 0x0
48 8d 45 d0                   LEA     RAX, QWORD ptr [RBP-0x30]
48 89 c7                      MOV     RDI, RAX
e8 0f 02 00 00                CALL    [RIP+0x20f]
89 d8                         MOV     EAX, EBX
```

view pybind11 bindings for this project - [here](https://github.com/HeX0Rci5T/x86_pybindings)
