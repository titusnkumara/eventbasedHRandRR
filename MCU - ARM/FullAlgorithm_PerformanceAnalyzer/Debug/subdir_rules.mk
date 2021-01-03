################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.obj: ../%.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccs910/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/bin/armcl" -mv7M3 --code_state=16 --float_support=vfplib -me --include_path="C:/Users/Titus.000/Desktop/PHD/Workspaces/PaperPulseBand/FullAlgorithm_PerformanceAnalyzer" --include_path="C:/ti/simplelink_cc2640r2_sdk_1_50_00_58/kernel/tirtos/packages/ti/sysbios/posix" --include_path="C:/ti/ccs910/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/include" --define=DeviceFamily_CC26X0R2 -g --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

build-648193569:
	@$(MAKE) --no-print-directory -Onone -f subdir_rules.mk build-648193569-inproc

build-648193569-inproc: ../performance.cfg
	@echo 'Building file: "$<"'
	@echo 'Invoking: XDCtools'
	"C:/ti/xdctools_3_50_03_33_core/xs" --xdcpath="C:/ti/simplelink_cc2640r2_sdk_1_50_00_58/source;C:/ti/simplelink_cc2640r2_sdk_1_50_00_58/kernel/tirtos/packages;" xdc.tools.configuro -o configPkg -t ti.targets.arm.elf.M3 -p ti.platforms.simplelink:CC2640R2F -r release -c "C:/ti/ccs910/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS" --compileOptions "-mv7M3 --code_state=16 --float_support=vfplib -me --include_path=\"C:/Users/Titus.000/Desktop/PHD/Workspaces/PaperPulseBand/FullAlgorithm_PerformanceAnalyzer\" --include_path=\"C:/ti/simplelink_cc2640r2_sdk_1_50_00_58/kernel/tirtos/packages/ti/sysbios/posix\" --include_path=\"C:/ti/ccs910/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/include\" --define=DeviceFamily_CC26X0R2 -g --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on  " "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

configPkg/linker.cmd: build-648193569 ../performance.cfg
configPkg/compiler.opt: build-648193569
configPkg/: build-648193569


