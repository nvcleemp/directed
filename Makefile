all: scripts build/directed_is_hypohamiltonian build/directed_is_hypotraceable

clean:

scripts: build/wcf2tikz.py build/wcf2multi.py 

build/directed_is_hypohamiltonian: hypospanning/directed_is_hypohamiltonian.c
	mkdir -p build
	$(CC) -o build/directed_is_hypohamiltonian -O4 hypospanning/directed_is_hypohamiltonian.c

build/directed_is_hypotraceable: hypospanning/directed_is_hypotraceable.c
	mkdir -p build
	$(CC) -o build/directed_is_hypotraceable -O4 hypospanning/directed_is_hypotraceable.c

build/wcf2tikz.py: tools/wcf2tikz.py
	mkdir -p build
	cp tools/wcf2tikz.py build/

build/wcf2multi.py: tools/wcf2multi.py
	mkdir -p build
	cp tools/wcf2multi.py build/

