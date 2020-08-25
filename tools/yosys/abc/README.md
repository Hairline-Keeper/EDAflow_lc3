# Physical ABC -- A Physical Aware Timing Optimization Tool
Physical ABC aims at improving the timing optimizaions done by ABC 
through adding physical-awareness by parsing a generated SPEF 
with the actual wire capacitances to be used during optimizations.
Physical ABC in implemented by Scale Lab at Brown University as part of [the OPENROAD
project](https://theopenroadproject.org/).

This is a fork and extension from open-source yosys tool as available here:
- https://github.com/berkeley-abc/abc

Along with the standard functionality provided by abc, Physical ABC aims to add support for physical aware timing optimizations, and better timing constraint handling by adding support to parse the sdc constraints file. 

## Website and Other Resources

For original abc documentation and installation please refer to:

- https://github.com/berkeley-abc/abc/blob/master/README.md
- https://people.eecs.berkeley.edu/~alanmi/abc/


## Getting Started with Physical-Synthesis

Overall, the functionality of Physical ABC is very closely similar
to standard abc with extra functionality added to the existing 
commands.

### Physical-aware gate sizing and buffering:
Physical ABC aims at enabling the use of the actual wire capacitances
from the generated SPEF file after placement into timing optimizations.

This feature is currently implemented within three commands `buffer`, `upsize` and `dnsize`,
and it can be enabled by adding the flag `-x` while using the command.

ABC reads the spef from the file `spef_output/netlist.spef` which should exist in the running directory.

More information can be found by looking at the help of every command in ABC,
for example `buffer -h`,

An example use case is as follows.

```    
dnsize -x
buffer -x -p
upsize -x
```

### Timing Constraints Parsing Through SDC Files
Physical ABC has added support for SDC file parsing through synopsys
open source sdc parser. The parser is integrated into ABC, enabling passing
of timing constraints through standard sdc syntax. the SDC parser in 
enabled by passing the -s flag to read_constr command in ABC.

A snippet of code demonstrating the use case is as follows.
```
read_constr -s myfile.sdc
```

Currently, while the parsing of all the sdc syntax is supported, due to
limited support of ABC for timing constraints, very few timing constraints 
are realized by ABC, while the rest are mainly ignored.
The current list of supported timing constraints include:
```
create_clock        #Support for target clock period
set_max_fanout      #Support for global max fanout
set_max_transition  #Support for global target slew 
```

We are working on adding support for other timing constraints in ABC.

### Remark
This Repo is currently maintained by Marina Neseem <marina_neseem@brown.edu>.
Consider also Soheil Hashemi <soheil_hashemi@alumni.brown.edu> who has started this effort.
