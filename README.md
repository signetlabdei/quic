QUIC implementation for QUIC in ns-3
================================

## QUIC code base

This repository is a fork of [signetlabdei/quic](https://github.com/signetlabdei/quic) which contains in the code for a native IETF QUIC implementation in ns-3.

The implementation is described in [this paper](https://arxiv.org/abs/1902.06121).

Please use this [issue tracker](https://github.com/signetlabdei/quic/issues) for bugs/questions.


## BBR implementation

The implementation provided in this repository is an adaptation of the TCP one provided in [Vivek-anand-jain/ns-3-dev-git/bbr-dev](https://github.com/Vivek-anand-jain/ns-3-dev-git/tree/bbr-dev).

## Install

### Prerequisites ###

To run simulations using this module, you will need to install ns-3, clone
this repository inside the `src` directory and patch the `wscript` file of the internet module. 
Required dependencies include git and a build environment.

#### Installing dependencies ####

Please refer to [the ns-3 wiki](https://www.nsnam.org/wiki/Installation) for instructions on how to set up your system to install ns-3. 
This module was implemented and tested on ns-3.29, and forward compatibility is not guaranteed.

#### Downloading #####

First, download the main ns-3 repository following the instructions on [the ns-3 website](https://www.nsnam.org/releases/ns-3-29/).

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

Configure and build ns-3 from the `ns-3.29` folder:

```bash
./waf configure --enable-tests --enable-examples
./waf build
```

If you are not interested in using the Python bindings, use
```bash
./waf configure --enable-tests --enable-examples --disable-python
./waf build
```
