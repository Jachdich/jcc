all:
	+make -C libjtools/
	+make -C jld/
	+make -C jvm/
	+make -C jobjdump/
	+make -C jcc/
	+make -C jas/

install:
	+make -C libjtools/ install
	+make -C jas/      install
	+make -C jld/      install
	+make -C jvm/      install
	+make -C jobjdump/ install
	+make -C jcc/  	  install

clean:
	+make -C jas/      clean
	+make -C jld/      clean
	+make -C jvm/      clean
	+make -C jobjdump/ clean
	+make -C libjtools/  clean
	+make -C jcc/  	  clean
