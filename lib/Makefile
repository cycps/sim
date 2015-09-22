#!/usr/bin/env bash

SHELL=/bin/bash

BLUE=\e[34m
GREEN=\e[32m
DEFAULT=\e[39m

.PHONY: all
all: sundials boost yaml rympi

#yaml --------------------------------------------------------------------------
.PHONY: yaml
yaml: yaml-cpp-release-0.5.2
	@printf "${BLUE}Installing libyaml${DEFAULT}\n"
	@cd yaml-cpp-release-0.5.2; \
		cmake \
			-DCMAKE_CXX_COMPILER=clang++ \
			-DCMAKE_C_COMPILER=clang \
			-DCMAKE_CXX_FLAGS="-stdlib=libc++" \
			. > /dev/null && \
		make > /dev/null && \
		make install > /dev/null
	
	@printf "${GREEN}Finished installing libyaml${DEFAULT}\n"

yaml-cpp-release-0.5.2:
	@printf "${BLUE}Unpacking libyaml${DEFAULT}\n"
	@tar xzf yaml-cpp-release-0.5.2.tar.gz

rympi: RyMPI-0.1
	@printf "${BLUE}Installing rympi${DEFAULT}\n"
	
	@cd RyMPI-0.1; \
		cmake -G Ninja > /dev/null && \
		ninja > /dev/null && \
		ninja > /dev/null install	
	
	@printf "${GREEN}Finished installing rympi${DEFAULT}\n"



RyMPI-0.1:
	@printf "${BLUE}Unpacking rympi${DEFAULT}\n"
	@tar xzf RyMPI-0.1.tar.gz



#boost -------------------------------------------------------------------------
.PHONY: boost
boost: boost_1_58_0
	@printf "${BLUE}Installing required boost libraries${DEFAULT}\n"
	
	@cd boost_1_58_0; \
		./bootstrap.sh \
			--with-toolset=clang --with-libraries=program_options,filesystem,system \
			mcxxflags="-std=c++1y -stdlib=libc++" \
			linkflags="-stdlib=libc++" > /dev/null && \
		./b2 \
			toolset=clang \
			cxxflags="-std=c++1y -stdlib=libc++" \
			linkflags="-stdlib=libc++" \
			-j 4 stage release  > /dev/null && \
		./b2 install \
			toolset=clang \
			cxxflags="-std=c++1y -stdlib=libc++" \
			linkflags="-stdlib=libc++" > /dev/null
	
	@printf "${GREEN}Finished installing boost libraries${DEFAULT}\n"
	
boost_1_58_0:
	@printf "${BLUE}Unpacking boost${DEFAULT}\n"
	@tar xzf boost_1_58_0.tar.gz

#libxml ------------------------------------------------------------------------
.PHONY: libxml
libxml: libxml2-2.7.2
	@printf '$(BLUE)%s$(DEFAULT)\n' "Installing libxml" 
	
	@cd libxml2-2.7.2; \
		./configure > /dev/null && \
		make > /dev/null && \
		make install > /dev/null
	
	@printf "${GREEN}Finished installing libxml${DEFAULT}\n"

libxml2-2.7.2:
	@printf "${BLUE}Unpacking libxml${DEFAULT}\n"
	@tar xzf libxml2-2.7.2.tar.gz 

#mpich -------------------------------------------------------------------------
.PHONY: mpich
mpich: libxml mpich-3.1.4
	@printf "${BLUE}Installing libmpich (this one takes a while :/)${DEFAULT}\n"
	
	@cd mpich-3.1.4; \
		./configure --disable-fortran > /dev/null && \
		make -g4 > /dev/null && \
		make install > /dev/null
	
	@printf "${GREEN}Finished installing libmpich${DEFAULT}\n"

mpich-3.1.4:
	@printf "${BLUE}Unpacking mpich${DEFAULT}\n"
	@tar xzf mpich-3.1.4.tar.gz

#sundials ----------------------------------------------------------------------
.PHONY: sundials
sundials: mpich sundials-2.5.0
	@printf "${BLUE}Installing sundials${DEFAULT}\n"
	
	@cd sundials-2.5.0; \
		./configure --enable-shared > /dev/null && \
		make > /dev/null && \
		make install > /dev/null
	
	@bash apply_patches.sh > /dev/null \
	
	@printf "${GREEN}Finished installing sundials${DEFAULT}\n"

sundials-2.5.0:
	@printf "${BLUE}Unpacking sundials${DEFAULT}\n"
	@tar xzf sundials-2.5.0.tar.gz


.PHONY: clean
clean:
	rm -rf sundials-2.5.0
	rm -rf libxml2-2.7.2
	rm -rf boost_1_58_0
	rm -rf mpich-3.1.4
	rm -rf yaml-cpp-release-0.5.2

