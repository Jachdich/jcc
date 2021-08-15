install:
	make -C assembler/ install
	make -C linker/    install
	make -C vm/        install
	make -C objdump/   install
	make -C libjobj/   install
	make -C jcc/  	   install

clean:
	make -C assembler/ clean
	make -C linker/    clean
	make -C vm/        clean
	make -C objdump/   clean
	make -C libjobj/   clean
	make -C jcc/  	   clean
