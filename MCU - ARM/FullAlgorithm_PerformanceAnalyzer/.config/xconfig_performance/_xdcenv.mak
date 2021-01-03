#
_XDCBUILDCOUNT = 0
ifneq (,$(findstring path,$(_USEXDCENV_)))
override XDCPATH = C:/ti/simplelink_cc2640r2_sdk_1_50_00_58/source;C:/ti/simplelink_cc2640r2_sdk_1_50_00_58/kernel/tirtos/packages;C:/Users/Titus.000/Desktop/PHD/Workspaces/PaperPulseBand/FullAlgorithm_PerformanceAnalyzer/.config
override XDCROOT = C:/ti/xdctools_3_50_03_33_core
override XDCBUILDCFG = ./config.bld
endif
ifneq (,$(findstring args,$(_USEXDCENV_)))
override XDCARGS = 
override XDCTARGETS = 
endif
#
ifeq (0,1)
PKGPATH = C:/ti/simplelink_cc2640r2_sdk_1_50_00_58/source;C:/ti/simplelink_cc2640r2_sdk_1_50_00_58/kernel/tirtos/packages;C:/Users/Titus.000/Desktop/PHD/Workspaces/PaperPulseBand/FullAlgorithm_PerformanceAnalyzer/.config;C:/ti/xdctools_3_50_03_33_core/packages;..
HOSTOS = Windows
endif
