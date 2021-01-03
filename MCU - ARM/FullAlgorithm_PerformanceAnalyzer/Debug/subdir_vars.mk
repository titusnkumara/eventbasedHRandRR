################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Add inputs and outputs from these tool invocations to the build variables 
CFG_SRCS += \
../performance.cfg 

CMD_SRCS += \
../CC2640R2_LAUNCHXL_TIRTOS.cmd 

C_SRCS += \
../CC2640R2_LAUNCHXL.c \
../EventProcessor.c \
../SampleAndEventGen.c \
../ccfg.c \
../main_tirtos.c \
../utilFunctions.c 

GEN_CMDS += \
./configPkg/linker.cmd 

GEN_FILES += \
./configPkg/linker.cmd \
./configPkg/compiler.opt 

GEN_MISC_DIRS += \
./configPkg/ 

C_DEPS += \
./CC2640R2_LAUNCHXL.d \
./EventProcessor.d \
./SampleAndEventGen.d \
./ccfg.d \
./main_tirtos.d \
./utilFunctions.d 

GEN_OPTS += \
./configPkg/compiler.opt 

OBJS += \
./CC2640R2_LAUNCHXL.obj \
./EventProcessor.obj \
./SampleAndEventGen.obj \
./ccfg.obj \
./main_tirtos.obj \
./utilFunctions.obj 

GEN_MISC_DIRS__QUOTED += \
"configPkg\" 

OBJS__QUOTED += \
"CC2640R2_LAUNCHXL.obj" \
"EventProcessor.obj" \
"SampleAndEventGen.obj" \
"ccfg.obj" \
"main_tirtos.obj" \
"utilFunctions.obj" 

C_DEPS__QUOTED += \
"CC2640R2_LAUNCHXL.d" \
"EventProcessor.d" \
"SampleAndEventGen.d" \
"ccfg.d" \
"main_tirtos.d" \
"utilFunctions.d" 

GEN_FILES__QUOTED += \
"configPkg\linker.cmd" \
"configPkg\compiler.opt" 

C_SRCS__QUOTED += \
"../CC2640R2_LAUNCHXL.c" \
"../EventProcessor.c" \
"../SampleAndEventGen.c" \
"../ccfg.c" \
"../main_tirtos.c" \
"../utilFunctions.c" 


