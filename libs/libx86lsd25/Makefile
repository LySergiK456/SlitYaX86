WIN_BDIR = ./build/windows
LINUX_BDIR = ./build/linux

.PHONY: linux
linux:
	@mkdir -p $(LINUX_BDIR)
	@make -f Makefile.linux -j$(nproc)

.PHONY: linux_example
linux_example:
	@make -f Makefile.linux example -j$(nproc)

.PHONY: windows
windows:
	@mkdir -p $(WIN_BDIR)
	@make -f Makefile.windows -j$(nproc)

clean:
	rm -rf ./$(WIN_BDIR)/*
	rm -rf ./$(LINUX_BDIR)/*