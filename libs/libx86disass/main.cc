#include <stdio.h>
#include <x86disass/disass.hpp>

#define p(s,...) printf(s, __VA_ARGS__);

int main() {
	auto d = Disass();
	d.iter(main, 0x10, [](__u64 i, insn_t& in) {
		if (in.IsNull()) return;
		p("%s\n", in.Mnemo());

		for (int i = 0; i < in.OperCount(); i++) {
			auto op = in[i];
			p("Operand [%i] - %s\n", i, in[i]->Str());

			switch (in[i]->Type()) {
				case OperType::REG: p("%s\n", op->Reg().name());		break;
				case OperType::IMM: p("Immediate %lx\n", op->Value());	break;
				case OperType::FXD: p("Fixed %lx\n", op->Value());		break;
				case OperType::PTR:
					if (!!in.Sib()) {
						p("[%i * %s + %s + %lx]\n", op->Scale(), op->IndexReg(), op->BaseReg(), op->Value());
					} else if (op->IsRIP()) {
						p("[%s + %lx ]\n", in[i]->IsRIP() == 32 ? "EIP" : "RIP", in[i]->Value());
					}
			}
		}
		puts("\n");
	});
	return 0;
}
