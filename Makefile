all: build/directed_is_hypohamiltonian

clean:

build/directed_is_hypohamiltonian: hypospanning/directed_is_hypohamiltonian.c
	mkdir -p build
	$(CC) -o build/directed_is_hypohamiltonian -O4 hypospanning/directed_is_hypohamiltonian.c