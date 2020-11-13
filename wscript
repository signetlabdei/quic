# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

# def options(opt):
#     pass

# def configure(conf):
#     conf.check_nonfatal(header_name='stdint.h', define_name='HAVE_STDINT_H')

def build(bld):
    module = bld.create_ns3_module('quic', ['internet', 'applications', 'flow-monitor', 'point-to-point'])
    module.source = [
        'model/quic-congestion-ops.cc',
        'model/quic-socket.cc',
        'model/quic-socket-base.cc',
        'model/quic-socket-factory.cc',
        'model/quic-l4-protocol.cc',
        'model/quic-socket-rx-buffer.cc',
        'model/quic-socket-tx-buffer.cc',
        'model/quic-socket-tx-scheduler.cc',
        'model/quic-socket-tx-pfifo-scheduler.cc',
        'model/quic-socket-tx-edf-scheduler.cc',
        'model/quic-stream.cc',
        'model/quic-stream-base.cc',
        'model/quic-l5-protocol.cc',
        'model/quic-stream-tx-buffer.cc',
        'model/quic-stream-rx-buffer.cc',
        'model/quic-header.cc',
        'model/quic-subheader.cc',
        'model/quic-transport-parameters.cc',
        'model/quic-bbr.cc',
        'helper/quic-helper.cc'
        ]

    module_test = bld.create_ns3_module_test_library('quic')
    module_test.source = [
        'test/quic-rx-buffer-test.cc',
        'test/quic-tx-buffer-test.cc',
        'test/quic-header-test.cc',
        ]

    headers = bld(features='ns3header')
    headers.module = 'quic'
    headers.source = [
        'model/quic-congestion-ops.h',
        'model/quic-socket.h',
        'model/quic-socket-base.h',
        'model/quic-socket-factory.h',
        'model/quic-l4-protocol.h',
        'model/quic-socket-rx-buffer.h',
        'model/quic-socket-tx-buffer.h',
        'model/quic-socket-tx-scheduler.h',
        'model/quic-socket-tx-pfifo-scheduler.h',
        'model/quic-socket-tx-edf-scheduler.h',
        'model/quic-stream.h',
        'model/quic-stream-base.h',
        'model/quic-l5-protocol.h',
        'model/quic-stream-tx-buffer.h',
        'model/quic-stream-rx-buffer.h',
        'model/quic-header.h',
        'model/quic-subheader.h',
        'model/quic-transport-parameters.h',
        'model/quic-bbr.h',
        'helper/quic-helper.h',
        'model/windowed-filter.h'
        ]

    if bld.env.ENABLE_EXAMPLES:
        bld.recurse('examples')

    # bld.ns3_python_bindings()

