# This script is written as a demo for the implemented physical synthesis optimization
# The used benchmark is GCD and the used library is ASAP7

yosys -import

set vtop "gcd"
set outputDir "demo/output"

if {[file exists $outputDir]} {
    exec rm -rf $outputDir
}
exec mkdir $outputDir

# Logic Synthesis Parameters
set sclib "demo/asap7.lib"
set constr_file "demo/gcd.sdc"
set abc_script  "+read_constr,-s,${constr_file};strash;ifraig;retime,-D,{D},-M,6;strash;dch,-f;map,-p,-M,1,{D},-f;"

# RePlace Parameters
set res_per_micron 4.881592
set cap_per_micron 0.165938e-15

# Def Generator Parameters
set die_width 30
set die_height 30

# Verilog to Def Translator Parameters
set lef1 "demo/lefs_asap7/asap7sc7p5t_24_L_4x_170912_mod.lef"
set lef2 "demo/lefs_asap7/asap7sc7p5t_24_R_4x_170912_mod.lef"
set lef3 "demo/lefs_asap7/asap7sc7p5t_24_SL_4x_170912_mod.lef"
set lef4 "demo/lefs_asap7/asap7sc7p5t_24_SRAM_4x_170912_mod.lef"
set lef5 "demo/lefs_asap7/asap7_tech_4x_170803.lef"

# read design
read_verilog "demo/gcd.v"

# check hierarchy
hierarchy -check -top $vtop

# flatten design
synth  -top $vtop -flatten

# clean
opt_clean -purge

# map generic flip flops to technology dependent flip flops
dfflibmap -liberty $sclib

# map generic gates to technology dependent gates
abc -constr -s "$constr_file" -liberty $sclib  -script $abc_script

# clean
opt_clean -purge

# run the physical synthesis command which runs floor planning then placement and 
# takes the output SPEF to use the actual wire capacitances into buffering and gate sizing
phys_abc -constr -s "$constr_file" -liberty $sclib \
         -clk_port CLK -lef $lef1 -lef $lef2 -lef $lef3 -lef $lef4 -lef $lef5 \
         -res_per_micron $res_per_micron -cap_per_micron $cap_per_micron \
         -die_width $die_width -die_height $die_height \
         -output tools/RePlAce/output \
         -PinsPlacer tools/Def_Analyzer/pins_placer.py \
         -DefGenerator tools/defgenerator -defDbu 4000 -siteName coreSite \
         -RePlAce tools/RePlAce/RePlAce

# clean
opt_clean -purge

# write verilog
write_verilog -noattr -noexpr "$outputDir/$vtop.gl.v"
