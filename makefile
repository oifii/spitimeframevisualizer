current:
	@echo "Please use one of the following specifications:"
	@echo "          make osx"
	@echo "          make linux"
	
osx:
	-make -f makefile.osx
	
linux:
	-make -f makefile.linux