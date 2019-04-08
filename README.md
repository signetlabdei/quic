QUIC implementation for ns-3
================================

## QUIC code base
This repository contains in the code for a native IETF QUIC implementation in ns-3.

The implementation is described in [this paper](https://arxiv.org/abs/1902.06121).

Please use this [issue tracker](https://github.com/signetlabdei/quic-ns-3/issues) for bugs/questions.

## Install

### Prerequisites ###

To run simulations using this module, you will need to install ns-3, clone
this repository inside the `src` directory and patch the `wscript` file of the internet module. 
Required dependencies include git and a build environment.

#### Installing dependencies ####

Please refer to [the ns-3 wiki](https://www.nsnam.org/wiki/Installation) for instructions on how to set up your system to install ns-3.

#### Downloading #####

First, clone the main ns-3 repository:

```bash
git clone https://gitlab.com/nsnam/ns-3-dev ns-3-dev
cd ns-3-dev/src
```

Then, clone the quic module:
```bash
git clone https://github.com/signetlabdei/quic quic
```

Finally, edit the `wscript` file of the internet module and add
```python
        'model/ipv4-end-point.h',
        'model/ipv4-end-point-demux.h',
        'model/ipv6-end-point.h',
        'model/ipv6-end-point-demux.h',
```
to the `headers.source` list

### Compilation ###

Configure and build ns-3 from the `ns-3-dev` folder:

```bash
./waf configure --enable-tests --enable-examples
./waf build
```

If you are not interested in using the Python bindings, use
```bash
./waf configure --enable-tests --enable-examples --disable-python
./waf build
```
