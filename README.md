# custom-plot-lab

The set of helpers, utilities, and wrappers for [QCustomPlot](https://www.qcustomplot.com) for using in projects [reZonator](https://github.com/orion-project/rezonator2), [Spectrum](https://github.com/orion-project/spectrum) and others.

## Usage

Add git submodule:

```bash
cd /YOUR/PROJECT/DIR
git submodule add https://github.com/orion-project/custom-plot-lab
git submodule init
git submodule update
```

Add include file in the `.pro` file of your Qt project:

```c
include("custom-plot-lab/custom-plot-lab.pri")
```
