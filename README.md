QUIC implementation for ns-3
================================

## QUIC code base
This repository contains in the code for a native IETF QUIC implementation in ns-3.

The implementation is described in [this paper](https://arxiv.org/abs/1902.06121).

Please use this [issue tracker](https://github.com/signetlabdei/quic-ns-3/issues) for bugs/questions.

## Install

### Prerequisites ###

To run simulations using this module, you will need to install ns-3, clone
this repository inside the `src` directory, copy the QUIC applications from the quic-applications folder, and patch the `wscript` file of the applications module. 
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

Thirdly, copy the QUIC applications and helpers to the applications module

```bash
cp src/quic/quic-applications/model/* src/applications/model/
cp src/quic/quic-applications/helper/* src/applications/helper/
```

Finally, edit the `wscript` file of the applications module and add

```python
        'model/quic-echo-client.h',
        'model/quic-echo-server.h',
        'model/quic-client.h',
        'model/quic-server.h',
        'helper/quic-echo-helper.h',
        'helper/quic-client-server-helper.h'
```
to the `headers.source` list and

```python
        'model/quic-echo-client.cc',
        'model/quic-echo-server.cc',
        'model/quic-client.cc',
        'model/quic-server.cc',
        'helper/quic-echo-helper.cc',
        'helper/quic-client-server-helper.cc'
```
to the `module.source` list
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
