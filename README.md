# MieruCache

## Usage

Clone this project and move into the cloned directory.
Then, type the following commands:

```
make
./build/mierucache example/example.txt
```

The expected result is as follows:

```
Input lines: 97
14775000
	1 1 2047 87fffffc I M
15125000
	1 2 2047 87fffffc M I
16075000
	2 2 2047 87fffffc I M
16425000
	2 3 2047 87fffffc M I
...
88425000
	0 3 123 800007b0 S S
	1 3 123 800007b0 S S
	2 3 123 800007b0 S S
88575000
	3 3 123 800007b0 I S
89875000
	0 0 9 80000090 I E
90225000
	0 1 124 800007c0 E S
90375000
	1 1 124 800007c0 I S
90725000
	0 2 124 800007c0 S S
	1 2 124 800007c0 S S
90875000
	2 2 124 800007c0 I S
91225000
	0 3 124 800007c0 S S
	1 3 124 800007c0 S S
	2 3 124 800007c0 S S
...
114625000
	1 2 48 80000300 E S
114775000
	2 2 48 80000300 I S
115125000
	1 3 48 80000300 S S
```

## TODO

- [X] Implement a simple parser.
- [ ] Implement GUI.
