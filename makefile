install:
	make -C assembler/ install
	make -C linker/    install
	make -C vm/        install
	make -C objdump/   install
	make -C libjobj/  install
