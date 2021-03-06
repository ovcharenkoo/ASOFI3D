# Makefile for ASOFI3D

#--------------------------------------------------------
# edit here
# modules for KAUST

# source code for model generation (viscoelastic)
MODEL_SRC_V = model_visco.c

# source code for model generation (elastic)
MODEL_SRC_E = model_elastic.c

# source code for model generation (acoustic)
MODEL_SRC_A = model_acoustic.c

# source code for benchmark model
MODEL_SRC_BENCH = benchmod.c


# -----------------------------------------------------------------------------
# Configure stage

# Do not detect code prerequisites while executing commands
# `make clean` or `make clobber`.
ifeq "$(filter clean clobber, $(MAKECMDGOALS))" ""
    -include config-auto.mk
    -include config-manual.mk
endif

ifneq "$(wildcard config-auto.mk)" ""
    mpicheck = $(shell $(CC) testmpi.c -o /dev/null >/dev/null 2>&1 && echo 1)

    ifneq "$(mpicheck)" "1"
        $(error Compiler $(CC) cannot compile test MPI program. Cannot proceed)
    endif
endif

SILENT ?= @


SEISMERGE_SCR = seismerge.c


SNAPMERGE_SCR = \
	json_parser.c\
	merge.c \
	read_par_json.c \
	readdsk.c \
	snapmerge.c \
	util.c \
	writedsk.c


PARTMODEL_SCR = \
	json_parser.c\
	part_model.c \
	read_par_json.c \
	util.c


ASOFI3D_UTIL = \
		absorb.c \
		av_mat.c \
		comm_ini.c \
		catseis.c \
		info.c \
		initproc.c \
		initsour.c \
		json_parser.c\
		merge.c \
		mergemod.c \
		note.c \
		outseis.c \
		outseis_glob.c \
		output_source_signal.c \
		PML_ini.c \
		rd_sour.c \
		read_checkpoint.c\
		readdsk.c \
		read_par_json.c \
		exchange_par.c \
		receiver.c \
		save_checkpoint.c\
		saveseis.c \
		saveseis_glob.c \
		sources.c \
		splitrec.c \
		splitsrc.c \
		timing.c \
		util.c \
		wavelet.c \
		writedsk.c \
		writemod.c \
		writepar.c \
		zero_acoustic.c \
        zero.c \
		zero_elastic.c \
		zero_elastic_CPML.c \
		rwsegy.c

ASOFI3D_SRC = \
		sofi3D.c \
		comm_ini_s.c \
		checkfd.c \
		CPML_coeff.c \
		CPML_ini_elastic.c\
		eqsource.c \
		seismo.c \
		matcopy.c \
		memory.c \
		surface.c \
		surface_elastic.c \
		update_s.c \
		update_s_elastic.c \
		update_s_CPML.c \
		update_s_CPML_elastic.c \
		update_v.c \
		update_v_CPML.c \
		snap.c \
		exchange_v.c \
		exchange_s.c \
		psource.c \
		readmod.c \
		source_moment_tensor.c \
		source_random.c \
		$(MODEL_SRC_E) \
		$(MODEL_SRC_V) \
		$(ASOFI3D_UTIL) \
		merge_source_field.c \
		write_source_field.c \
		madinput.c 

SOFI3D_SRC_BENCH = \
		sofi3D.c \
		comm_ini_s.c \
		checkfd.c \
		CPML_coeff.c \
		CPML_ini_elastic.c\
		eqsource.c \
		seismo.c \
		matcopy.c \
		memory.c \
		surface.c \
		surface_elastic.c \
		update_s.c \
		update_s_elastic.c \
		update_s_CPML.c \
		update_s_CPML_elastic.c \
		update_v.c \
		update_v_CPML.c \
		snap.c \
		exchange_v.c \
		exchange_s.c \
		psource.c \
		readmod.c \
		$(MODEL_SRC_BENCH) \
		$(MODEL_SRC_V) \
		$(ASOFI3D_UTIL)

SOFI3D_SRC_RSG = \
		sofi3D_rsg.c \
		checkfd_rsg.c \
		seismo_rsg.c \
		update_s_rsg.c \
		matcopy.c \
		update_v_rsg.c \
		snap_rsg.c \
		exchange_v_rsg.c \
		exchange_s_rsg.c \
		psource_rsg.c \
		$(MODEL_SRC_E) \
		$(MODEL_SRC_V) \
		$(ASOFI3D_UTIL)

SOFI3D_SRC_ACOUSTIC = \
		av_mat_acoustic.c \
		sofi3D_acoustic.c \
		checkfd_acoustic.c \
		comm_ini_acoustic.c \
		seismo_acoustic.c \
		matcopy_acoustic.c \
		surface_acoustic.c \
		PML_ini_acoustic.c \
		absorb_PML.c \
		update_s_acoustic.c \
		update_v_acoustic.c \
		update_s_acoustic_PML.c \
		update_v_acoustic_PML.c \
		snap_acoustic.c \
		exchange_v_acoustic.c \
		exchange_s_acoustic.c \
		psource_acoustic.c \
		readmod_acoustic.c \
		$(MODEL_SRC_A) \
		$(ASOFI3D_UTIL)

ASOFI3D_OBJ = $(ASOFI3D_SRC:%.c=%.o)
SOFI3D_OBJ_RSG = $(SOFI3D_SRC_RSG:%.c=%.o)
SOFI3D_OBJ_BENCH = $(SOFI3D_SRC_BENCH:%.c=%.o)
SOFI3D_OBJ_ACOUSTIC = $(SOFI3D_SRC_ACOUSTIC:%.c=%.o)

SNAPMERGE_OBJ = $(SNAPMERGE_SCR:%.c=%.o)
PARTMODEL_OBJ = $(PARTMODEL_SCR:%.c=%.o)
SEISMERGE_OBJ = $(SEISMERGE_SCR:%.c=%.o)

program_list = asofi3D seismerge snapmerge part_model sofi3D_acoustic 

.PHONY: all
all: clean print-config-auto.mk $(program_list)

.PHONY: clean
clean:
	find . -name "*.o"   -exec rm {} \;
	find . -name "*.bck" -exec rm {} \;
	find . -name "core"  -exec rm {} \;

# Target `asofi3D` is defined only for user convenience and has empty recipe.
.PHONY : asofi3D
asofi3D: ../bin/asofi3D ;

# Using built-in variable LINK.c defined as
# LINK.c = $(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(TARGET_ARCH)

../bin/asofi3D: $(ASOFI3D_OBJ)
	$(CC) $(CFLAGS) $(CPPFLAGS) $^ $(LDLIBS) -o $@

seismerge:	$(SEISMERGE_OBJ)
	$(CC) $(CFLAGS) $(CPPFLAGS) $^ $(LDLIBS) -o ../bin/seismerge

snapmerge:	$(SNAPMERGE_OBJ)
	$(CC) $(CFLAGS) $(CPPFLAGS) $^ $(LDLIBS) -o ../bin/snapmerge

part_model:	$(PARTMODEL_OBJ)
	$(CC) $(CFLAGS) $(CPPFLAGS) $^ $(LDLIBS) -o ../bin/partmodel

#sofi3D_rsg: $(SOFI3D_OBJ_RSG)
#	$(CC) $(SOFI3D_OBJ_RSG) -o ../bin/sofi3D_rsg $(LDLIBS)

sofi3D_bench: $(SOFI3D_OBJ_BENCH)
	$(CC) $(CFLAGS) $(CPPFLAGS) $^ $(LDLIBS) -o ../bin/sofi3D_bench

sofi3D_acoustic: $(SOFI3D_OBJ_ACOUSTIC)
	$(CC) $(CFLAGS) $(CPPFLAGS) $^ $(LDLIBS) -o ../bin/sofi3D_acoustic

clobber: clean
	-$(RM) config-auto.mk
	-$(RM) -r craycc_pl

config-auto.mk: configure
	@./$< > $@

.PHONY : print-config-auto.mk
print-config-auto.mk : config-auto.mk
	@echo "*** config-auto.mk: begin"
	@cat $<
	@echo "*** config-auto.mk: end"

.SUFFIXES: .c .o
.c.o:
	$(SILENT)$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@
	@echo Making $@
