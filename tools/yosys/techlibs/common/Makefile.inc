
ifneq ($(SMALL),1)
OBJS += techlibs/common/synth.o
OBJS += techlibs/common/prep.o
endif

GENFILES += techlibs/common/simlib_help.inc
GENFILES += techlibs/common/simcells_help.inc

techlibs/common/simlib_help.inc: techlibs/common/cellhelp.py techlibs/common/simlib.v
	$(Q) mkdir -p techlibs/common
	$(P) $(PYTHON_EXECUTABLE) $^ > $@.new
	$(Q) mv $@.new $@

techlibs/common/simcells_help.inc: techlibs/common/cellhelp.py techlibs/common/simcells.v
	$(Q) mkdir -p techlibs/common
	$(P) $(PYTHON_EXECUTABLE) $^ > $@.new
	$(Q) mv $@.new $@

kernel/register.o: techlibs/common/simlib_help.inc techlibs/common/simcells_help.inc

$(eval $(call add_share_file,share,techlibs/common/simlib.v))
$(eval $(call add_share_file,share,techlibs/common/simcells.v))
$(eval $(call add_share_file,share,techlibs/common/techmap.v))
$(eval $(call add_share_file,share,techlibs/common/pmux2mux.v))
$(eval $(call add_share_file,share,techlibs/common/adff2dff.v))
$(eval $(call add_share_file,share,techlibs/common/dff2ff.v))
$(eval $(call add_share_file,share,techlibs/common/gate2lut.v))
$(eval $(call add_share_file,share,techlibs/common/cmp2lut.v))
$(eval $(call add_share_file,share,techlibs/common/cells.lib))
$(eval $(call add_share_file,share,techlibs/common/Def_Analyzer/utils.py))
$(eval $(call add_share_file,share,techlibs/common/Def_Analyzer/merger.py))
$(eval $(call add_share_file,share,techlibs/common/Def_Analyzer/pins_placer.py))
$(eval $(call add_share_file,share,techlibs/common/Def_Analyzer/incremental_def_writer.py))
$(eval $(call add_share_file,share,techlibs/common/brown_parser.tcl))
$(eval $(call add_share_file,share/sdc2.1,techlibs/common/sdc2.1/eda_sample.bat))
$(eval $(call add_share_file,share/sdc2.1,techlibs/common/sdc2.1/eda_write_sdc.tcl))
$(eval $(call add_share_file,share/sdc2.1,techlibs/common/sdc2.1/sdc1.1.tcl))
$(eval $(call add_share_file,share/sdc2.1,techlibs/common/sdc2.1/sdc1.3.tcl))
$(eval $(call add_share_file,share/sdc2.1,techlibs/common/sdc2.1/sdc1.5.tcl))
$(eval $(call add_share_file,share/sdc2.1,techlibs/common/sdc2.1/sdc1.7.tcl))
$(eval $(call add_share_file,share/sdc2.1,techlibs/common/sdc2.1/sdc1.9.tcl))
$(eval $(call add_share_file,share/sdc2.1,techlibs/common/sdc2.1/sdc2.1.tcl))
$(eval $(call add_share_file,share/sdc2.1,techlibs/common/sdc2.1/sdcparsercore.tcl))
$(eval $(call add_share_file,share/sdc2.1,techlibs/common/sdc2.1/sdctest.bat))
$(eval $(call add_share_file,share/sdc2.1,techlibs/common/sdc2.1/eda_sample.tcl))
$(eval $(call add_share_file,share/sdc2.1,techlibs/common/sdc2.1/sdc1.2.tcl))
$(eval $(call add_share_file,share/sdc2.1,techlibs/common/sdc2.1/sdc1.4.tcl))
$(eval $(call add_share_file,share/sdc2.1,techlibs/common/sdc2.1/sdc1.6.tcl))
$(eval $(call add_share_file,share/sdc2.1,techlibs/common/sdc2.1/sdc1.8.tcl))
$(eval $(call add_share_file,share/sdc2.1,techlibs/common/sdc2.1/sdc2.0.tcl))
$(eval $(call add_share_file,share/sdc2.1,techlibs/common/sdc2.1/sdcparser.bat))
$(eval $(call add_share_file,share/sdc2.1,techlibs/common/sdc2.1/sdcparser.tcl))
$(eval $(call add_share_file,share,techlibs/common/mul2dsp.v))
$(eval $(call add_share_file,share,techlibs/common/abc9_model.v))
