all:
	@echo 
	@echo FRT is not meant to be compiled stand alone out of the box.
	@echo You need to put it in the \"platform\" directory of godot and use
	@echo scons as usual.
	@echo 
	@echo For example, from the main top level godot directory, type:
	@echo
	@echo "  " scons platform=frt target=release tools=no frt_arch=pi3
	@echo 
	@echo 
