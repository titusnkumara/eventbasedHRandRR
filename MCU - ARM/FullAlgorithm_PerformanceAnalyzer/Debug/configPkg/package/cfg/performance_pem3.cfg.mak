# invoke SourceDir generated makefile for performance.pem3
performance.pem3: .libraries,performance.pem3
.libraries,performance.pem3: package/cfg/performance_pem3.xdl
	$(MAKE) -f C:\Users\Titus.000\Desktop\PHD\Workspaces\PaperPulseBand\FullAlgorithm_PerformanceAnalyzer/src/makefile.libs

clean::
	$(MAKE) -f C:\Users\Titus.000\Desktop\PHD\Workspaces\PaperPulseBand\FullAlgorithm_PerformanceAnalyzer/src/makefile.libs clean

