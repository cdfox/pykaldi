# SHELL += -x

CXX = g++

# On Mac OS 10.9, g++ is actually clang in disguise which by default uses the
# new c++ standard library libc++. Since openfst uses stuff from the tr1
# namespace, we need to tell clang to use libstdc++ instead.
COMPILER = $(shell $(CXX) -v 2>&1 )
ifeq ($(findstring clang,$(COMPILER)),clang)
	CXXFLAGS += -stdlib=libstdc++
	LDFLAGS += -stdlib=libstdc++
endif

all: check_required_programs sph2pipe atlas irstlm_tgt sclite_tgt openfst_tgt


check_required_programs:
	which aclocal
	which automake
	which autoconf
	which libtoolize || which glibtoolize
	which svn
	which gcc
	which $(CXX)
	which patch

clean: openfst_cleaned sclite_cleaned  irstlm_cleaned

openfst_cleaned:
		$(MAKE) -C openfst-1.3.2 clean

irstlm_cleaned:
		$(MAKE) -C irstlm clean 

sclite_cleaned:
		$(MAKE) -C sctk-2.4.0 clean

distclean:
		rm -rf openfst-1.3.2/
		rm -rf sctk-2.4.0/
		rm -rf ATLAS/
		rm -rf irstlm/
		rm -rf sph2pipe_v2.5/
		rm -rf sph2pipe_v2.5.tar.gz
		rm -rf atlas3.8.3.tar.gz
		rm -rf sctk-2.4.0-20091110-0958.tar.bz2
		rm -rf openfst-1.3.2.tar.gz
		rm -f openfst

openfst_tgt: openfst_compiled openfst/lib

.PHONY: openfst_compiled
openfst_compiled: openfst-1.3.2/Makefile
	cd openfst-1.3.2/ && \
	$(MAKE) install

openfst/lib:
	-cd openfst && [ -d lib64 ] && [ ! -d lib ] && ln -s lib64 lib

openfst-1.3.2/Makefile: openfst-1.3.2/.patched
		cd openfst-1.3.2/; \
		./configure --prefix=`pwd` --enable-static --enable-shared --enable-far --enable-ngram-fsts CXX=$(CXX) CXXFLAGS="$(CXXFLAGS)" LDFLAGS="$(LDFLAGS)" LIBS="-ldl"


# patches for openfst. openfst_gcc41up.patch is a patch for openfst to \
# support multi-threads when compile with g++ (gcc) version above 4.1
.PHONY: openfst-1.3.2/.patched
openfst-1.3.2/.patched: | openfst-1.3.2_tgt
		-cd openfst-1.3.2/src/include/fst; \
		patch -p0 -N < ../../../../openfst.patch;
		$(CXX) -dumpversion | awk '{if(NR==1 && $$1>"4.1") print "cd openfst-1.3.2/src/include/fst; patch -p0 -N < ../../../../extras/openfst_gcc41up.patch"}' | sh -
		touch $@ 	

openfst-1.3.2_tgt: openfst-1.3.2.tar.gz
		tar xozf openfst-1.3.2.tar.gz
		-rm openfst
		-ln -s openfst-1.3.2 openfst 

openfst-1.3.2.tar.gz: 
		wget http://openfst.cs.nyu.edu/twiki/pub/FST/FstDownload/openfst-1.3.2.tar.gz || \
		wget -T 10 -t 3 --no-check-certificate http://www.danielpovey.com/files/data/openfst-1.3.2.tar.gz 

sclite_tgt: sclite_compiled

.PHONY: sclite_compiled
sclite_compiled: sctk_patched
		cd sctk-2.4.0; \
		patch -p0 -N < ../sctk.patch; \
		{ . ../extras/install_sctk_patched.sh; } && $(MAKE) config && $(MAKE) all && $(MAKE) install && $(MAKE) doc

.PHONY: sctk_patched
sctk_patched: | sctk-2.4.0
		cd sctk-2.4.0/; \
		for x in src/asclite/core/recording.h src/asclite/core/recording.cpp; do \
			sed 's/Filter::Filter/::Filter/' $$x > tmpf; mv tmpf $$x; \
		done;

sctk-2.4.0: sctk-2.4.0-20091110-0958.tar.bz2
		tar xojf sctk-2.4.0-20091110-0958.tar.bz2

sctk-2.4.0-20091110-0958.tar.bz2:
		wget -T 10 -t 3 ftp://jaguar.ncsl.nist.gov/pub/sctk-2.4.0-20091110-0958.tar.bz2 || \
    wget --no-check-certificate -T 10  https://sourceforge.net/projects/kaldi/files/sctk-2.4.0-20091110-0958.tar.bz2

irstlm_tgt: irstlm_compiled

.PHONY: irstlm_compiled
irstlm_compiled: irstlm/Makefile
		cd irstlm/; \
		make; $(MAKE) install	

irstlm/Makefile: irstlm/.patched
		cd irstlm; \
	    automake --version | grep 1.13.1 >/dev/null && \
         sed s:AM_CONFIG_HEADER:AC_CONFIG_HEADERS: <configure.in >configure.ac; \
		(./regenerate-makefiles.sh || ./regenerate-makefiles.sh) && \
		./configure --prefix `pwd` 


irstlm/.patched: | irstlm
		-cd irstlm;\
		patch --verbose -N -p0 < ../interpolatedwrite-5.60.02.patch; \
		patch --verbose -N -p0 < ../irstlm.patch; \
		touch .patched		

irstlm:
		svn -r 398 co --non-interactive --trust-server-cert https://svn.code.sf.net/p/irstlm/code/trunk irstlm

atlas: ATLAS/include/cblas.h

ATLAS/include/cblas.h: | atlas3.8.3.tar.gz
		tar xozf atlas3.8.3.tar.gz ATLAS/include

atlas3.8.3.tar.gz:
		wget -T 10  http://sourceforge.net/projects/math-atlas/files/Stable/3.8.3/atlas3.8.3.tar.gz || \
		wget --no-check-certificate -T 10 -t 3 http://www.danielpovey.com/files/kaldi/atlas3.8.3.tar.gz

sph2pipe: sph2pipe_compiled

sph2pipe_compiled: sph2pipe_v2.5/sph2pipe

sph2pipe_v2.5/sph2pipe: | sph2pipe_v2.5
		cd sph2pipe_v2.5/; \
		$(CC) -o sph2pipe  *.c -lm

sph2pipe_v2.5: sph2pipe_v2.5.tar.gz
		tar xzf sph2pipe_v2.5.tar.gz

sph2pipe_v2.5.tar.gz:
		wget --no-check-certificate -T 10  https://sourceforge.net/projects/kaldi/files/sph2pipe_v2.5.tar.gz || \
        wget -T 10 -t 3 http://www.danielpovey.com/files/kaldi/sph2pipe_v2.5.tar.gz

openblas: openblas_compiled

.PHONY: openblas_compiled

fortran_opt = $(shell gcc -v 2>&1 | perl -e '$$x = join(" ", <STDIN>); if($$x =~ m/target=\S+64\S+/) { print "BINARY=64"; }')

openblas_compiled:
	-git clone git://github.com/xianyi/OpenBLAS
	$(MAKE) PREFIX=`pwd`/OpenBLAS/install FC=gfortran $(fortran_opt) DEBUG=1 USE_THREAD=0 -C OpenBLAS all install
