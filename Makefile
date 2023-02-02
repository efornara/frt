# Makefile
#
# FRT - A Godot platform targeting single board computers
# Copyright (c) 2017-2023  Emanuele Fornara
# SPDX-License-Identifier: MIT
#

all:
	@echo 
	@echo FRT is a platform for godot.
	@echo 
	@echo You need to put it in the \"platform\" directory of godot and use scons as usual\;
	@echo then, from the top level godot directory, you can type something like this:
	@echo
	@echo "   " scons platform=frt tools=no target=release
	@echo 
	@echo 

clean:
	rm -f *.o *.gen.* dl/*.gen.* import/*.o
