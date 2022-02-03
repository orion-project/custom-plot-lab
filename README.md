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

This library also uses [orion-qt](https://github.com/orion-project/orion-qt) helper library so it should be added as submodule to your project too.

## Sandbox app

`sandbox.pro` application is used for adding/debugging different features to the library.

Since the library requires [orion-qt](https://github.com/orion-project/orion-qt), the sandbox requires it too. Here is no submodule though, it just expects that library is cloned into `./orion` subdir (gitignored):

```bash
git clone https://github.com/orion-project/orion-qt orion
```
