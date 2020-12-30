# shake
Super small, simple and slow shake (almost sha3)

    $ make
    cc -Wall -O3 -DSHAKE_TEST=1 -o shake shake.c

    $ ./shake test
    time: 0.000002s
    shake128("test"): d3b0aa9cd8b7255622cebc631e867d4093d6f6010191a53973c45fec9b07c774
