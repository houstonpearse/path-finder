# Path finder

```
 0123456789
0I****
1    *
2    *
3    *
4  ***
5  *###
6  ***
7    *
8    *
9    *****G
```

A program that finds a path in a grid-like map. A grid with a path is supplied and the program determines if the route is valid - i.e. not blocked and stays within the map - and repairs the path if its not valid.

This was a program written by me for a university assignment. As such the path finding logic had to meet specific requirements so that the output was predetermined. This meant that even if the path was repaired correctly, but used a different algorithm, it would be marked as incorrect.

## Program Input

This program reads input from stdin. the structure of the input is as follows:

- The first line read is the size of the grid e.g.`20x20`
- the second line is the start of the path e.g.`[0,0]`
- the third line is the end of the path e.g.`[19,19]`
- the fourth up to `$` contain the sqaures on the grid that are blocked
- after the first `$` and up to the second `$` contain the initial route
- after the second `$` contain a set of new blocked squares. There can be multiple sets each seperated with a `$`

## Program Output

The program will output the current route and then repaired if required. This will be repeated for each set of blocked squares.

### Example

blocked set 1

```
 0123456789
0I****
1    *
2    *
3    *
4    *
5   ###
6    *
7    *
8    *
9    *****G
```

repaired route

```
 0123456789
0I****
1    *
2    *
3    *
4  ***
5  *###
6  ***
7    *
8    *
9    *****G
```

Blocked set 2

```
 0123456789
0I****
1    *
2    *
3    *
4  ***
5######## #
6  ***
7    *
8    #
9  #####**G
```

repaired route 2

```
 0123456789
0I****
1    *
2    *
3    *
4  *******
5########*#
6        *
7        *
8    #   *
9  ##### *G
```

## Usage

Compile by running
`make path-finder`

Then run with input
`./path-finder < input.txt`

Run tests `make test`
