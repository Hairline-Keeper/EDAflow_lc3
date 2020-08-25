```
yosys -- Yosys Open SYnthesis Suite

Copyright (C) 2019 SCALE Lab, Brown University

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Original Copyright (C) 2012 - 2018  Clifford Wolf <clifford@clifford.at>

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

// [[CITE]] ABC
// Berkeley Logic Synthesis and Verification Group, ABC: A System for Sequential Synthesis and Verification
// http://www.eecs.berkeley.edu/~alanmi/abc/

// [[CITE]] RePlAce
// UCSD VLSI CAD LABORATORY, RePlAce: A Global Placement tool
// https://github.com/abk-openroad/RePlAce
```

# Physical yosys -- A Physical Flow Integration in Yosys
Physical yosys aims at improving the timing optimizaions of yosys/ABC 
through adding physical-awareness (along with other features) in the synthesis flow.
Physical yosys in implemented by Scale Lab at Brown University as part of [the OPENROAD
project](https://theopenroadproject.org/).

This is a fork and extension from open-source yosys tool as available here:
- https://github.com/YosysHQ/yosys

Along with the standard functionality provided by yosys, Physical yosys aims to add support for physical aware logic synthesis, and better timing constraint handling through ABC. 

The used ABC version is a forked and modified version that could be found [here](https://github.com/scale-lab/abc).

Physical Yosys along with Yosys is free software licensed under the ISC license (a GPL compatible license that is similar in terms to the MIT license or the 2-clause BSD license).


## Website and Other Resources

For original yosys documentation and installation please refer to:

- https://github.com/YosysHQ/yosys/blob/master/README.md
- http://www.clifford.at/yosys/


## Getting Started with Physical-Synthesis

Overall, the functionality of Physical yosys is very closely similar
to standard yosys with extra functionality added to the existing 
commands and some new commands. 

### Physical-aware gate sizing and buffering:
Physical yosys aims at enabling the integration of physical placement
Information into logic optimizations.

Currently, physical yosys allows for this information to be used for
gate sizing and buffering optimizations done inside ABC.

Physical yosys uses [RePlAce](https://github.com/abk-openroad/RePlAce) for placement, a [Verilog to Def Translator](https://github.com/abk-openroad/OpenROAD-Utilities/tree/master/verilog-to-def) and a [Pin Placer](https://github.com/scale-lab/yosys/blob/master/tools/Def_Analyzer/pins_placer.py) to generate
post-placement wire capacitances. These wire capacitances are then
passed in a spef file to ABC where the optimizations are performed.
physical flow is Implemented as a `phys_abc` command as 
it relies on ABC to perform physical aware gate sizing and buffering,
`-clk_port` flag must be set by the name of the used library flip-flop clock port,
and if there are multiple clock ports all must be set by `-clk_port clk1 -clk_port ....`
also to run the physical flow using RePlAce, the following arguments must be set:

1. For the def genetator: `-DefGenerator` needs to be set to the location of the def translator as well as the needed arguments used by the Def Translator as `-defDbu`, `siteName`, `-die_width`, `-die_height`, for more info about these arguments visit [The Def Translator Gihub Repo](https://github.com/abk-openroad/OpenROAD-Utilities/blob/master/verilog-to-def/README.md)

2. For the pins placer: `-PinsPlacer` needs to be set to the location of the pins placer, the metal layer on which the pins would be placed can be defined by the option `-layer` which should be an integer value, You can find the script [here](https://github.com/scale-lab/yosys/blob/master/tools/Def_Analyzer/pins_placer.py).

3. For RePlAce: `-RePlAce` needs to be set to the location to replace as well as the needed arguments used by RePlAce as `-liberty`, `-constr`, `-res_per_micron`, `-cap_per_micron`, `-output`, for more info about these arguments visit [RePlAce Gihub Repo](https://github.com/abk-openroad/RePlAce/blob/master/README.md)

More information can be found by looking at `help phys_abc` in
yosys. An example use case is as follows.

```    
read_verilog test.v

# the high-level stuff
synth -flatten

# mapping flip-flops to mycells.lib
dfflibmap -liberty mycells.lib

# mapping logic to mycells.lib
abc -liberty mycells.lib

# cleanup
opt_clean -purge

#enabling physical flow
phys_abc -liberty mycell s.lib -clk_port CLK -RePlAce ... -lef file1.lef -lef file2.lef -constr myfile.sdc
-res_per_micron 1.261261 -cap_per_micron 0.226596e-15 -output output -dpflag ... -dploc ... 
-PinsPlacer ... -DefGenerator ... -defDbu 2000 -siteName ... -die_width 42 -die_height 42

# another cleanup
opt_clean -purge

#writing output
write_verilog test_netlist.v
```

### Timing Constraints Through SDC Files
Physical yosys has added support for SDC file parsing through synopsys
open source sdc parser. The parser is integrated into ABC, enabling passing
of timing constraints through standard sdc syntax.

This works with both `abc` and `phys_abc` commands.

Generally, Yosys itself doesn't use the SDC file, It just passes it to ABC,
so the SDC parser can be enabled by one of two methods:

1. Through Yosys by adding the `-s` flag after the `-constr` flag in `abc` or `phys_abc` command, and Yosys will pass it to ABC.
	
2. Through ABC directly by adding `-s` flag to `read_constr` command in the ABC script.

If `-s` flag was not added, the typical constraints parser of ABC (non SDC) will be used.

A snippet of code demonstrating the use case of the SDC Parser in Yosys is as follows:
```
abc -liberty mycells.lib -script ${abc_script} -constr -s myfile.sdc
```

A snippet of code demonstrating the use case of the SDC Parser in ABC is as follows:
```
set constr_file myfile.sdc
set abc_script "+read_constr,-s,${constr_file};strash;ifraig;retime,-D,450,-M,6;strash;dch;map;"
abc -liberty mycells.lib -script ${abc_script}
```

Currently, while the parsing of all the sdc syntax is supported, due to
limited support of ABC for timing constraints, very few timing constraints 
are realized by ABC, while the rest are mainly **Ignored**.
The current list of supported timing constraints include:
```
create_clock        #Support for target clock period
set_max_fanout      #Support for global max fanout
set_max_transition  #Support for global target slew 
```

We are working on adding support for other timing constraints in ABC.

### Helper Scripts

#### Incremental DEF Writer Script
It takes Two Def files and Merge them while retaining the placement data found in the placed one.
- **Inputs:**
  1. DEF file with all the components Placed
  2. The new DEF file with the modified/added components which are all unplaced
  
- **Output:**
DEF file with all components including Modified/Added ones from the second def files, with all components placed in the locations from the first placed DEF if found, else the component is placed in the centroid of its fanins and fanouts

- **Usage:** 
`python incremental_def_writer.py -placed placed.def -unplaced unplaced.def -output output.def`

- The Script could be found [here](https://github.com/The-OpenROAD-Project/yosys/blob/master/tools/Def_Analyzer/incremental_def_writer.py).

#### Pins Placer Script
It takes a def file with the pins unplaced and place them around the die perimeter, It can also change the layer on which pins are placed through the option `-layer` which should be an integer value with the layer number.

- **Usage:**
`python pins_placer.py -def file.def -output output.def [-layer layer_number]`

- The Script could be found [here](https://github.com/The-OpenROAD-Project/yosys/blob/master/tools/Def_Analyzer/pins_placer.py).

## Demo Script
There is a [demo](https://github.com/The-OpenROAD-Project/yosys/blob/master/demo/run_yosys_physical_synthesis.tcl) script for the added options. This Demo runs Logic Synthesis then Physical Synthesis for the GCD benchmark using ASAP7 library files, more details can be found inside the script itself.

The Benchmark and the Library files can be found in the folder **demo**.

To run it this demo script:
1. Clone and build the repository.
2. Run the script using `./yosys demo/run_yosys_physical_synthesis.tcl`.

The output netlist could be found at `demo/output`

### Remark
This Repo is currently maintained by Marina Neseem <marina_neseem@brown.edu>.
Consider also Soheil Hashemi <soheil_hashemi@alumni.brown.edu> who has started this effort.
