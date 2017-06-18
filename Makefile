.PHONY: all 3ds vita clean
all:
	@echo "Please select a platform:"
	@echo "3ds"
	@echo "vita"
3ds:
	@make -f 3DSMakefile
vita:
	@make -f VitaMakefile
clean:
	@make clean -f 3DSMakefile
	@make clean -f VitaMakefile