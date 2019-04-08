from pybindgen import Module, FileCodeSink, param, retval, cppclass, typehandlers


import pybindgen.settings
import warnings

class ErrorHandler(pybindgen.settings.ErrorHandler):
    def handle_error(self, wrapper, exception, traceback_):
        warnings.warn("exception %r in wrapper %s" % (exception, wrapper))
        return True
pybindgen.settings.error_handler = ErrorHandler()


import sys

def module_init():
    root_module = Module('ns.quic', cpp_namespace='::ns3')
    return root_module

def register_types(module):
    root_module = module.get_root()
    
    ## address.h (module 'network'): ns3::Address [class]
    module.add_class('Address', import_from_module='ns.network')
    ## address.h (module 'network'): ns3::Address::MaxSize_e [enumeration]
    module.add_enum('MaxSize_e', ['MAX_SIZE'], outer_class=root_module['ns3::Address'], import_from_module='ns.network')
    ## trace-helper.h (module 'network'): ns3::AsciiTraceHelper [class]
    module.add_class('AsciiTraceHelper', import_from_module='ns.network')
    ## trace-helper.h (module 'network'): ns3::AsciiTraceHelperForDevice [class]
    module.add_class('AsciiTraceHelperForDevice', allow_subclassing=True, import_from_module='ns.network')
    ## internet-trace-helper.h (module 'internet'): ns3::AsciiTraceHelperForIpv4 [class]
    module.add_class('AsciiTraceHelperForIpv4', allow_subclassing=True, import_from_module='ns.internet')
    ## internet-trace-helper.h (module 'internet'): ns3::AsciiTraceHelperForIpv6 [class]
    module.add_class('AsciiTraceHelperForIpv6', allow_subclassing=True, import_from_module='ns.internet')
    ## attribute-construction-list.h (module 'core'): ns3::AttributeConstructionList [class]
    module.add_class('AttributeConstructionList', import_from_module='ns.core')
    ## attribute-construction-list.h (module 'core'): ns3::AttributeConstructionList::Item [struct]
    module.add_class('Item', import_from_module='ns.core', outer_class=root_module['ns3::AttributeConstructionList'])
    typehandlers.add_type_alias(u'std::list< ns3::AttributeConstructionList::Item > const_iterator', u'ns3::AttributeConstructionList::CIterator')
    typehandlers.add_type_alias(u'std::list< ns3::AttributeConstructionList::Item > const_iterator*', u'ns3::AttributeConstructionList::CIterator*')
    typehandlers.add_type_alias(u'std::list< ns3::AttributeConstructionList::Item > const_iterator&', u'ns3::AttributeConstructionList::CIterator&')
    ## buffer.h (module 'network'): ns3::Buffer [class]
    module.add_class('Buffer', import_from_module='ns.network')
    ## buffer.h (module 'network'): ns3::Buffer::Iterator [class]
    module.add_class('Iterator', import_from_module='ns.network', outer_class=root_module['ns3::Buffer'])
    ## packet.h (module 'network'): ns3::ByteTagIterator [class]
    module.add_class('ByteTagIterator', import_from_module='ns.network')
    ## packet.h (module 'network'): ns3::ByteTagIterator::Item [class]
    module.add_class('Item', import_from_module='ns.network', outer_class=root_module['ns3::ByteTagIterator'])
    ## byte-tag-list.h (module 'network'): ns3::ByteTagList [class]
    module.add_class('ByteTagList', import_from_module='ns.network')
    ## byte-tag-list.h (module 'network'): ns3::ByteTagList::Iterator [class]
    module.add_class('Iterator', import_from_module='ns.network', outer_class=root_module['ns3::ByteTagList'])
    ## byte-tag-list.h (module 'network'): ns3::ByteTagList::Iterator::Item [struct]
    module.add_class('Item', import_from_module='ns.network', outer_class=root_module['ns3::ByteTagList::Iterator'])
    ## callback.h (module 'core'): ns3::CallbackBase [class]
    module.add_class('CallbackBase', import_from_module='ns.core')
    ## data-rate.h (module 'network'): ns3::DataRate [class]
    module.add_class('DataRate', import_from_module='ns.network')
    ## default-deleter.h (module 'core'): ns3::DefaultDeleter<ns3::AttributeAccessor> [struct]
    module.add_class('DefaultDeleter', import_from_module='ns.core', template_parameters=['ns3::AttributeAccessor'])
    ## default-deleter.h (module 'core'): ns3::DefaultDeleter<ns3::AttributeChecker> [struct]
    module.add_class('DefaultDeleter', import_from_module='ns.core', template_parameters=['ns3::AttributeChecker'])
    ## default-deleter.h (module 'core'): ns3::DefaultDeleter<ns3::AttributeValue> [struct]
    module.add_class('DefaultDeleter', import_from_module='ns.core', template_parameters=['ns3::AttributeValue'])
    ## default-deleter.h (module 'core'): ns3::DefaultDeleter<ns3::CallbackImplBase> [struct]
    module.add_class('DefaultDeleter', import_from_module='ns.core', template_parameters=['ns3::CallbackImplBase'])
    ## default-deleter.h (module 'core'): ns3::DefaultDeleter<ns3::EventImpl> [struct]
    module.add_class('DefaultDeleter', import_from_module='ns.core', template_parameters=['ns3::EventImpl'])
    ## default-deleter.h (module 'core'): ns3::DefaultDeleter<ns3::Hash::Implementation> [struct]
    module.add_class('DefaultDeleter', import_from_module='ns.core', template_parameters=['ns3::Hash::Implementation'])
    ## default-deleter.h (module 'core'): ns3::DefaultDeleter<ns3::Ipv4Route> [struct]
    module.add_class('DefaultDeleter', import_from_module='ns.core', template_parameters=['ns3::Ipv4Route'])
    ## default-deleter.h (module 'core'): ns3::DefaultDeleter<ns3::NixVector> [struct]
    module.add_class('DefaultDeleter', import_from_module='ns.core', template_parameters=['ns3::NixVector'])
    ## default-deleter.h (module 'core'): ns3::DefaultDeleter<ns3::OutputStreamWrapper> [struct]
    module.add_class('DefaultDeleter', import_from_module='ns.core', template_parameters=['ns3::OutputStreamWrapper'])
    ## default-deleter.h (module 'core'): ns3::DefaultDeleter<ns3::Packet> [struct]
    module.add_class('DefaultDeleter', import_from_module='ns.core', template_parameters=['ns3::Packet'])
    ## default-deleter.h (module 'core'): ns3::DefaultDeleter<ns3::TraceSourceAccessor> [struct]
    module.add_class('DefaultDeleter', import_from_module='ns.core', template_parameters=['ns3::TraceSourceAccessor'])
    ## event-id.h (module 'core'): ns3::EventId [class]
    module.add_class('EventId', import_from_module='ns.core')
    ## hash.h (module 'core'): ns3::Hasher [class]
    module.add_class('Hasher', import_from_module='ns.core')
    ## inet6-socket-address.h (module 'network'): ns3::Inet6SocketAddress [class]
    module.add_class('Inet6SocketAddress', import_from_module='ns.network')
    ## inet6-socket-address.h (module 'network'): ns3::Inet6SocketAddress [class]
    root_module['ns3::Inet6SocketAddress'].implicitly_converts_to(root_module['ns3::Address'])
    ## inet-socket-address.h (module 'network'): ns3::InetSocketAddress [class]
    module.add_class('InetSocketAddress', import_from_module='ns.network')
    ## inet-socket-address.h (module 'network'): ns3::InetSocketAddress [class]
    root_module['ns3::InetSocketAddress'].implicitly_converts_to(root_module['ns3::Address'])
    ## int-to-type.h (module 'core'): ns3::IntToType<0> [struct]
    module.add_class('IntToType', import_from_module='ns.core', template_parameters=['0'])
    ## int-to-type.h (module 'core'): ns3::IntToType<0>::v_e [enumeration]
    module.add_enum('v_e', ['value'], outer_class=root_module['ns3::IntToType< 0 >'], import_from_module='ns.core')
    ## int-to-type.h (module 'core'): ns3::IntToType<1> [struct]
    module.add_class('IntToType', import_from_module='ns.core', template_parameters=['1'])
    ## int-to-type.h (module 'core'): ns3::IntToType<1>::v_e [enumeration]
    module.add_enum('v_e', ['value'], outer_class=root_module['ns3::IntToType< 1 >'], import_from_module='ns.core')
    ## int-to-type.h (module 'core'): ns3::IntToType<2> [struct]
    module.add_class('IntToType', import_from_module='ns.core', template_parameters=['2'])
    ## int-to-type.h (module 'core'): ns3::IntToType<2>::v_e [enumeration]
    module.add_enum('v_e', ['value'], outer_class=root_module['ns3::IntToType< 2 >'], import_from_module='ns.core')
    ## int-to-type.h (module 'core'): ns3::IntToType<3> [struct]
    module.add_class('IntToType', import_from_module='ns.core', template_parameters=['3'])
    ## int-to-type.h (module 'core'): ns3::IntToType<3>::v_e [enumeration]
    module.add_enum('v_e', ['value'], outer_class=root_module['ns3::IntToType< 3 >'], import_from_module='ns.core')
    ## int-to-type.h (module 'core'): ns3::IntToType<4> [struct]
    module.add_class('IntToType', import_from_module='ns.core', template_parameters=['4'])
    ## int-to-type.h (module 'core'): ns3::IntToType<4>::v_e [enumeration]
    module.add_enum('v_e', ['value'], outer_class=root_module['ns3::IntToType< 4 >'], import_from_module='ns.core')
    ## int-to-type.h (module 'core'): ns3::IntToType<5> [struct]
    module.add_class('IntToType', import_from_module='ns.core', template_parameters=['5'])
    ## int-to-type.h (module 'core'): ns3::IntToType<5>::v_e [enumeration]
    module.add_enum('v_e', ['value'], outer_class=root_module['ns3::IntToType< 5 >'], import_from_module='ns.core')
    ## int-to-type.h (module 'core'): ns3::IntToType<6> [struct]
    module.add_class('IntToType', import_from_module='ns.core', template_parameters=['6'])
    ## int-to-type.h (module 'core'): ns3::IntToType<6>::v_e [enumeration]
    module.add_enum('v_e', ['value'], outer_class=root_module['ns3::IntToType< 6 >'], import_from_module='ns.core')
    ## ipv4-address.h (module 'network'): ns3::Ipv4Address [class]
    module.add_class('Ipv4Address', import_from_module='ns.network')
    ## ipv4-address.h (module 'network'): ns3::Ipv4Address [class]
    root_module['ns3::Ipv4Address'].implicitly_converts_to(root_module['ns3::Address'])
    ## ipv4-interface-address.h (module 'internet'): ns3::Ipv4InterfaceAddress [class]
    module.add_class('Ipv4InterfaceAddress', import_from_module='ns.internet')
    ## ipv4-interface-address.h (module 'internet'): ns3::Ipv4InterfaceAddress::InterfaceAddressScope_e [enumeration]
    module.add_enum('InterfaceAddressScope_e', ['HOST', 'LINK', 'GLOBAL'], outer_class=root_module['ns3::Ipv4InterfaceAddress'], import_from_module='ns.internet')
    ## ipv4-interface-container.h (module 'internet'): ns3::Ipv4InterfaceContainer [class]
    module.add_class('Ipv4InterfaceContainer', import_from_module='ns.internet')
    typehandlers.add_type_alias(u'std::vector< std::pair< ns3::Ptr< ns3::Ipv4 >, unsigned int > > const_iterator', u'ns3::Ipv4InterfaceContainer::Iterator')
    typehandlers.add_type_alias(u'std::vector< std::pair< ns3::Ptr< ns3::Ipv4 >, unsigned int > > const_iterator*', u'ns3::Ipv4InterfaceContainer::Iterator*')
    typehandlers.add_type_alias(u'std::vector< std::pair< ns3::Ptr< ns3::Ipv4 >, unsigned int > > const_iterator&', u'ns3::Ipv4InterfaceContainer::Iterator&')
    ## ipv4-address.h (module 'network'): ns3::Ipv4Mask [class]
    module.add_class('Ipv4Mask', import_from_module='ns.network')
    ## ipv6-address.h (module 'network'): ns3::Ipv6Address [class]
    module.add_class('Ipv6Address', import_from_module='ns.network')
    ## ipv6-address.h (module 'network'): ns3::Ipv6Address [class]
    root_module['ns3::Ipv6Address'].implicitly_converts_to(root_module['ns3::Address'])
    ## ipv6-interface-address.h (module 'internet'): ns3::Ipv6InterfaceAddress [class]
    module.add_class('Ipv6InterfaceAddress', import_from_module='ns.internet')
    ## ipv6-interface-address.h (module 'internet'): ns3::Ipv6InterfaceAddress::State_e [enumeration]
    module.add_enum('State_e', ['TENTATIVE', 'DEPRECATED', 'PREFERRED', 'PERMANENT', 'HOMEADDRESS', 'TENTATIVE_OPTIMISTIC', 'INVALID'], outer_class=root_module['ns3::Ipv6InterfaceAddress'], import_from_module='ns.internet')
    ## ipv6-interface-address.h (module 'internet'): ns3::Ipv6InterfaceAddress::Scope_e [enumeration]
    module.add_enum('Scope_e', ['HOST', 'LINKLOCAL', 'GLOBAL'], outer_class=root_module['ns3::Ipv6InterfaceAddress'], import_from_module='ns.internet')
    ## ipv6-interface-container.h (module 'internet'): ns3::Ipv6InterfaceContainer [class]
    module.add_class('Ipv6InterfaceContainer', import_from_module='ns.internet')
    typehandlers.add_type_alias(u'std::vector< std::pair< ns3::Ptr< ns3::Ipv6 >, unsigned int > > const_iterator', u'ns3::Ipv6InterfaceContainer::Iterator')
    typehandlers.add_type_alias(u'std::vector< std::pair< ns3::Ptr< ns3::Ipv6 >, unsigned int > > const_iterator*', u'ns3::Ipv6InterfaceContainer::Iterator*')
    typehandlers.add_type_alias(u'std::vector< std::pair< ns3::Ptr< ns3::Ipv6 >, unsigned int > > const_iterator&', u'ns3::Ipv6InterfaceContainer::Iterator&')
    ## ipv6-address.h (module 'network'): ns3::Ipv6Prefix [class]
    module.add_class('Ipv6Prefix', import_from_module='ns.network')
    ## mac48-address.h (module 'network'): ns3::Mac48Address [class]
    module.add_class('Mac48Address', import_from_module='ns.network')
    typehandlers.add_type_alias(u'void ( * ) ( ns3::Mac48Address )', u'ns3::Mac48Address::TracedCallback')
    typehandlers.add_type_alias(u'void ( * ) ( ns3::Mac48Address )*', u'ns3::Mac48Address::TracedCallback*')
    typehandlers.add_type_alias(u'void ( * ) ( ns3::Mac48Address )&', u'ns3::Mac48Address::TracedCallback&')
    ## mac48-address.h (module 'network'): ns3::Mac48Address [class]
    root_module['ns3::Mac48Address'].implicitly_converts_to(root_module['ns3::Address'])
    ## mac8-address.h (module 'network'): ns3::Mac8Address [class]
    module.add_class('Mac8Address', import_from_module='ns.network')
    ## mac8-address.h (module 'network'): ns3::Mac8Address [class]
    root_module['ns3::Mac8Address'].implicitly_converts_to(root_module['ns3::Address'])
    ## net-device-container.h (module 'network'): ns3::NetDeviceContainer [class]
    module.add_class('NetDeviceContainer', import_from_module='ns.network')
    typehandlers.add_type_alias(u'std::vector< ns3::Ptr< ns3::NetDevice > > const_iterator', u'ns3::NetDeviceContainer::Iterator')
    typehandlers.add_type_alias(u'std::vector< ns3::Ptr< ns3::NetDevice > > const_iterator*', u'ns3::NetDeviceContainer::Iterator*')
    typehandlers.add_type_alias(u'std::vector< ns3::Ptr< ns3::NetDevice > > const_iterator&', u'ns3::NetDeviceContainer::Iterator&')
    ## node-container.h (module 'network'): ns3::NodeContainer [class]
    module.add_class('NodeContainer', import_from_module='ns.network')
    typehandlers.add_type_alias(u'std::vector< ns3::Ptr< ns3::Node > > const_iterator', u'ns3::NodeContainer::Iterator')
    typehandlers.add_type_alias(u'std::vector< ns3::Ptr< ns3::Node > > const_iterator*', u'ns3::NodeContainer::Iterator*')
    typehandlers.add_type_alias(u'std::vector< ns3::Ptr< ns3::Node > > const_iterator&', u'ns3::NodeContainer::Iterator&')
    ## object-base.h (module 'core'): ns3::ObjectBase [class]
    module.add_class('ObjectBase', allow_subclassing=True, import_from_module='ns.core')
    ## object.h (module 'core'): ns3::ObjectDeleter [struct]
    module.add_class('ObjectDeleter', import_from_module='ns.core')
    ## object-factory.h (module 'core'): ns3::ObjectFactory [class]
    module.add_class('ObjectFactory', import_from_module='ns.core')
    ## packet-metadata.h (module 'network'): ns3::PacketMetadata [class]
    module.add_class('PacketMetadata', import_from_module='ns.network')
    ## packet-metadata.h (module 'network'): ns3::PacketMetadata::Item [struct]
    module.add_class('Item', import_from_module='ns.network', outer_class=root_module['ns3::PacketMetadata'])
    ## packet-metadata.h (module 'network'): ns3::PacketMetadata::Item::ItemType [enumeration]
    module.add_enum('ItemType', ['PAYLOAD', 'HEADER', 'TRAILER'], outer_class=root_module['ns3::PacketMetadata::Item'], import_from_module='ns.network')
    ## packet-metadata.h (module 'network'): ns3::PacketMetadata::ItemIterator [class]
    module.add_class('ItemIterator', import_from_module='ns.network', outer_class=root_module['ns3::PacketMetadata'])
    ## packet.h (module 'network'): ns3::PacketTagIterator [class]
    module.add_class('PacketTagIterator', import_from_module='ns.network')
    ## packet.h (module 'network'): ns3::PacketTagIterator::Item [class]
    module.add_class('Item', import_from_module='ns.network', outer_class=root_module['ns3::PacketTagIterator'])
    ## packet-tag-list.h (module 'network'): ns3::PacketTagList [class]
    module.add_class('PacketTagList', import_from_module='ns.network')
    ## packet-tag-list.h (module 'network'): ns3::PacketTagList::TagData [struct]
    module.add_class('TagData', import_from_module='ns.network', outer_class=root_module['ns3::PacketTagList'])
    ## pcap-file.h (module 'network'): ns3::PcapFile [class]
    module.add_class('PcapFile', import_from_module='ns.network')
    ## trace-helper.h (module 'network'): ns3::PcapHelper [class]
    module.add_class('PcapHelper', import_from_module='ns.network')
    ## trace-helper.h (module 'network'): ns3::PcapHelper::DataLinkType [enumeration]
    module.add_enum('DataLinkType', ['DLT_NULL', 'DLT_EN10MB', 'DLT_PPP', 'DLT_RAW', 'DLT_IEEE802_11', 'DLT_LINUX_SLL', 'DLT_PRISM_HEADER', 'DLT_IEEE802_11_RADIO', 'DLT_IEEE802_15_4', 'DLT_NETLINK'], outer_class=root_module['ns3::PcapHelper'], import_from_module='ns.network')
    ## trace-helper.h (module 'network'): ns3::PcapHelperForDevice [class]
    module.add_class('PcapHelperForDevice', allow_subclassing=True, import_from_module='ns.network')
    ## internet-trace-helper.h (module 'internet'): ns3::PcapHelperForIpv4 [class]
    module.add_class('PcapHelperForIpv4', allow_subclassing=True, import_from_module='ns.internet')
    ## internet-trace-helper.h (module 'internet'): ns3::PcapHelperForIpv6 [class]
    module.add_class('PcapHelperForIpv6', allow_subclassing=True, import_from_module='ns.internet')
    ## quic-socket-rx-buffer.h (module 'quic'): ns3::QuicSocketRxItem [class]
    module.add_class('QuicSocketRxItem')
    ## quic-socket-tx-buffer.h (module 'quic'): ns3::QuicSocketTxItem [class]
    module.add_class('QuicSocketTxItem')
    ## quic-stream-rx-buffer.h (module 'quic'): ns3::QuicStreamRxItem [class]
    module.add_class('QuicStreamRxItem')
    ## quic-stream-tx-buffer.h (module 'quic'): ns3::QuicStreamTxItem [class]
    module.add_class('QuicStreamTxItem')
    ## tcp-socket-base.h (module 'internet'): ns3::RttHistory [class]
    module.add_class('RttHistory', import_from_module='ns.internet')
    ## sequence-number.h (module 'network'): ns3::SequenceNumber<unsigned int, int> [class]
    module.add_class('SequenceNumber32', import_from_module='ns.network')
    ## simple-ref-count.h (module 'core'): ns3::SimpleRefCount<ns3::Object, ns3::ObjectBase, ns3::ObjectDeleter> [class]
    module.add_class('SimpleRefCount', automatic_type_narrowing=True, import_from_module='ns.core', template_parameters=['ns3::Object', 'ns3::ObjectBase', 'ns3::ObjectDeleter'], parent=root_module['ns3::ObjectBase'], memory_policy=cppclass.ReferenceCountingMethodsPolicy(incref_method='Ref', decref_method='Unref', peekref_method='GetReferenceCount'))
    ## simulator.h (module 'core'): ns3::Simulator [class]
    module.add_class('Simulator', destructor_visibility='private', import_from_module='ns.core')
    ## simulator.h (module 'core'): ns3::Simulator [enumeration]
    module.add_enum('', ['NO_CONTEXT'], outer_class=root_module['ns3::Simulator'], import_from_module='ns.core')
    ## tag.h (module 'network'): ns3::Tag [class]
    module.add_class('Tag', import_from_module='ns.network', parent=root_module['ns3::ObjectBase'])
    ## tag-buffer.h (module 'network'): ns3::TagBuffer [class]
    module.add_class('TagBuffer', import_from_module='ns.network')
    ## nstime.h (module 'core'): ns3::TimeWithUnit [class]
    module.add_class('TimeWithUnit', import_from_module='ns.core')
    ## timer.h (module 'core'): ns3::Timer [class]
    module.add_class('Timer', import_from_module='ns.core')
    ## timer.h (module 'core'): ns3::Timer::DestroyPolicy [enumeration]
    module.add_enum('DestroyPolicy', ['CANCEL_ON_DESTROY', 'REMOVE_ON_DESTROY', 'CHECK_ON_DESTROY'], outer_class=root_module['ns3::Timer'], import_from_module='ns.core')
    ## timer.h (module 'core'): ns3::Timer::State [enumeration]
    module.add_enum('State', ['RUNNING', 'EXPIRED', 'SUSPENDED'], outer_class=root_module['ns3::Timer'], import_from_module='ns.core')
    ## timer-impl.h (module 'core'): ns3::TimerImpl [class]
    module.add_class('TimerImpl', allow_subclassing=True, import_from_module='ns.core')
    ## traced-value.h (module 'core'): ns3::TracedValue<ns3::QuicSocket::QuicStates_t> [class]
    module.add_class('TracedValue', import_from_module='ns.core', template_parameters=['ns3::QuicSocket::QuicStates_t'])
    ## traced-value.h (module 'core'): ns3::TracedValue<ns3::SequenceNumber<unsigned int, int> > [class]
    module.add_class('TracedValue', import_from_module='ns.core', template_parameters=['ns3::SequenceNumber<unsigned int, int>'])
    ## traced-value.h (module 'core'): ns3::TracedValue<ns3::SequenceNumber<unsigned int, int> > [class]
    root_module['ns3::TracedValue< ns3::SequenceNumber<unsigned int, int> >'].implicitly_converts_to(root_module['ns3::SequenceNumber32'])
    ## traced-value.h (module 'core'): ns3::TracedValue<ns3::TcpSocket::TcpStates_t> [class]
    module.add_class('TracedValue', import_from_module='ns.core', template_parameters=['ns3::TcpSocket::TcpStates_t'])
    ## traced-value.h (module 'core'): ns3::TracedValue<ns3::TcpSocketState::EcnState_t> [class]
    module.add_class('TracedValue', import_from_module='ns.core', template_parameters=['ns3::TcpSocketState::EcnState_t'])
    ## traced-value.h (module 'core'): ns3::TracedValue<ns3::TcpSocketState::TcpCongState_t> [class]
    module.add_class('TracedValue', import_from_module='ns.core', template_parameters=['ns3::TcpSocketState::TcpCongState_t'])
    ## traced-value.h (module 'core'): ns3::TracedValue<unsigned int> [class]
    module.add_class('TracedValue', import_from_module='ns.core', template_parameters=['unsigned int'])
    ## type-id.h (module 'core'): ns3::TypeId [class]
    module.add_class('TypeId', import_from_module='ns.core')
    ## type-id.h (module 'core'): ns3::TypeId::AttributeFlag [enumeration]
    module.add_enum('AttributeFlag', ['ATTR_GET', 'ATTR_SET', 'ATTR_CONSTRUCT', 'ATTR_SGC'], outer_class=root_module['ns3::TypeId'], import_from_module='ns.core')
    ## type-id.h (module 'core'): ns3::TypeId::SupportLevel [enumeration]
    module.add_enum('SupportLevel', ['SUPPORTED', 'DEPRECATED', 'OBSOLETE'], outer_class=root_module['ns3::TypeId'], import_from_module='ns.core')
    ## type-id.h (module 'core'): ns3::TypeId::AttributeInformation [struct]
    module.add_class('AttributeInformation', import_from_module='ns.core', outer_class=root_module['ns3::TypeId'])
    ## type-id.h (module 'core'): ns3::TypeId::TraceSourceInformation [struct]
    module.add_class('TraceSourceInformation', import_from_module='ns.core', outer_class=root_module['ns3::TypeId'])
    typehandlers.add_type_alias(u'uint32_t', u'ns3::TypeId::hash_t')
    typehandlers.add_type_alias(u'uint32_t*', u'ns3::TypeId::hash_t*')
    typehandlers.add_type_alias(u'uint32_t&', u'ns3::TypeId::hash_t&')
    ## empty.h (module 'core'): ns3::empty [class]
    module.add_class('empty', import_from_module='ns.core')
    ## int64x64-128.h (module 'core'): ns3::int64x64_t [class]
    module.add_class('int64x64_t', import_from_module='ns.core')
    ## int64x64-128.h (module 'core'): ns3::int64x64_t::impl_type [enumeration]
    module.add_enum('impl_type', ['int128_impl', 'cairo_impl', 'ld_impl'], outer_class=root_module['ns3::int64x64_t'], import_from_module='ns.core')
    ## chunk.h (module 'network'): ns3::Chunk [class]
    module.add_class('Chunk', import_from_module='ns.network', parent=root_module['ns3::ObjectBase'])
    ## header.h (module 'network'): ns3::Header [class]
    module.add_class('Header', import_from_module='ns.network', parent=root_module['ns3::Chunk'])
    ## internet-stack-helper.h (module 'internet'): ns3::InternetStackHelper [class]
    module.add_class('InternetStackHelper', import_from_module='ns.internet', parent=[root_module['ns3::PcapHelperForIpv4'], root_module['ns3::PcapHelperForIpv6'], root_module['ns3::AsciiTraceHelperForIpv4'], root_module['ns3::AsciiTraceHelperForIpv6']])
    ## ipv4-header.h (module 'internet'): ns3::Ipv4Header [class]
    module.add_class('Ipv4Header', import_from_module='ns.internet', parent=root_module['ns3::Header'])
    ## ipv4-header.h (module 'internet'): ns3::Ipv4Header::DscpType [enumeration]
    module.add_enum('DscpType', ['DscpDefault', 'DSCP_CS1', 'DSCP_AF11', 'DSCP_AF12', 'DSCP_AF13', 'DSCP_CS2', 'DSCP_AF21', 'DSCP_AF22', 'DSCP_AF23', 'DSCP_CS3', 'DSCP_AF31', 'DSCP_AF32', 'DSCP_AF33', 'DSCP_CS4', 'DSCP_AF41', 'DSCP_AF42', 'DSCP_AF43', 'DSCP_CS5', 'DSCP_EF', 'DSCP_CS6', 'DSCP_CS7'], outer_class=root_module['ns3::Ipv4Header'], import_from_module='ns.internet')
    ## ipv4-header.h (module 'internet'): ns3::Ipv4Header::EcnType [enumeration]
    module.add_enum('EcnType', ['ECN_NotECT', 'ECN_ECT1', 'ECN_ECT0', 'ECN_CE'], outer_class=root_module['ns3::Ipv4Header'], import_from_module='ns.internet')
    ## ipv6-header.h (module 'internet'): ns3::Ipv6Header [class]
    module.add_class('Ipv6Header', import_from_module='ns.internet', parent=root_module['ns3::Header'])
    ## ipv6-header.h (module 'internet'): ns3::Ipv6Header::DscpType [enumeration]
    module.add_enum('DscpType', ['DscpDefault', 'DSCP_CS1', 'DSCP_AF11', 'DSCP_AF12', 'DSCP_AF13', 'DSCP_CS2', 'DSCP_AF21', 'DSCP_AF22', 'DSCP_AF23', 'DSCP_CS3', 'DSCP_AF31', 'DSCP_AF32', 'DSCP_AF33', 'DSCP_CS4', 'DSCP_AF41', 'DSCP_AF42', 'DSCP_AF43', 'DSCP_CS5', 'DSCP_EF', 'DSCP_CS6', 'DSCP_CS7'], outer_class=root_module['ns3::Ipv6Header'], import_from_module='ns.internet')
    ## ipv6-header.h (module 'internet'): ns3::Ipv6Header::NextHeader_e [enumeration]
    module.add_enum('NextHeader_e', ['IPV6_EXT_HOP_BY_HOP', 'IPV6_IPV4', 'IPV6_TCP', 'IPV6_UDP', 'IPV6_IPV6', 'IPV6_EXT_ROUTING', 'IPV6_EXT_FRAGMENTATION', 'IPV6_EXT_CONFIDENTIALITY', 'IPV6_EXT_AUTHENTIFICATION', 'IPV6_ICMPV6', 'IPV6_EXT_END', 'IPV6_EXT_DESTINATION', 'IPV6_SCTP', 'IPV6_EXT_MOBILITY', 'IPV6_UDP_LITE'], outer_class=root_module['ns3::Ipv6Header'], import_from_module='ns.internet')
    ## ipv6-header.h (module 'internet'): ns3::Ipv6Header::EcnType [enumeration]
    module.add_enum('EcnType', ['ECN_NotECT', 'ECN_ECT1', 'ECN_ECT0', 'ECN_CE'], outer_class=root_module['ns3::Ipv6Header'], import_from_module='ns.internet')
    ## object.h (module 'core'): ns3::Object [class]
    module.add_class('Object', import_from_module='ns.core', parent=root_module['ns3::SimpleRefCount< ns3::Object, ns3::ObjectBase, ns3::ObjectDeleter >'])
    ## object.h (module 'core'): ns3::Object::AggregateIterator [class]
    module.add_class('AggregateIterator', import_from_module='ns.core', outer_class=root_module['ns3::Object'])
    ## pcap-file-wrapper.h (module 'network'): ns3::PcapFileWrapper [class]
    module.add_class('PcapFileWrapper', import_from_module='ns.network', parent=root_module['ns3::Object'])
    ## quic-header.h (module 'quic'): ns3::QuicHeader [class]
    module.add_class('QuicHeader', parent=root_module['ns3::Header'])
    ## quic-header.h (module 'quic'): ns3::QuicHeader::TypeFormat_t [enumeration]
    module.add_enum('TypeFormat_t', ['SHORT', 'LONG'], outer_class=root_module['ns3::QuicHeader'])
    ## quic-header.h (module 'quic'): ns3::QuicHeader::TypeLong_t [enumeration]
    module.add_enum('TypeLong_t', ['VERSION_NEGOTIATION', 'INITIAL', 'RETRY', 'HANDSHAKE', 'ZRTT_PROTECTED', 'NONE'], outer_class=root_module['ns3::QuicHeader'])
    ## quic-header.h (module 'quic'): ns3::QuicHeader::KeyPhase_t [enumeration]
    module.add_enum('KeyPhase_t', ['PHASE_ZERO', 'PHASE_ONE'], outer_class=root_module['ns3::QuicHeader'])
    ## quic-header.h (module 'quic'): ns3::QuicHeader::TypeShort_t [enumeration]
    module.add_enum('TypeShort_t', ['ONE_OCTECT', 'TWO_OCTECTS', 'FOUR_OCTECTS'], outer_class=root_module['ns3::QuicHeader'])
    typehandlers.add_type_alias(u'ns3::QuicHeader::TypeFormat_t', u'ns3::QuicHeader::TypeFormat_t')
    typehandlers.add_type_alias(u'ns3::QuicHeader::TypeFormat_t*', u'ns3::QuicHeader::TypeFormat_t*')
    typehandlers.add_type_alias(u'ns3::QuicHeader::TypeFormat_t&', u'ns3::QuicHeader::TypeFormat_t&')
    typehandlers.add_type_alias(u'ns3::QuicHeader::TypeLong_t', u'ns3::QuicHeader::TypeLong_t')
    typehandlers.add_type_alias(u'ns3::QuicHeader::TypeLong_t*', u'ns3::QuicHeader::TypeLong_t*')
    typehandlers.add_type_alias(u'ns3::QuicHeader::TypeLong_t&', u'ns3::QuicHeader::TypeLong_t&')
    typehandlers.add_type_alias(u'ns3::QuicHeader::KeyPhase_t', u'ns3::QuicHeader::KeyPhase_t')
    typehandlers.add_type_alias(u'ns3::QuicHeader::KeyPhase_t*', u'ns3::QuicHeader::KeyPhase_t*')
    typehandlers.add_type_alias(u'ns3::QuicHeader::KeyPhase_t&', u'ns3::QuicHeader::KeyPhase_t&')
    typehandlers.add_type_alias(u'ns3::QuicHeader::TypeShort_t', u'ns3::QuicHeader::TypeShort_t')
    typehandlers.add_type_alias(u'ns3::QuicHeader::TypeShort_t*', u'ns3::QuicHeader::TypeShort_t*')
    typehandlers.add_type_alias(u'ns3::QuicHeader::TypeShort_t&', u'ns3::QuicHeader::TypeShort_t&')
    ## quic-helper.h (module 'quic'): ns3::QuicHelper [class]
    module.add_class('QuicHelper', parent=root_module['ns3::InternetStackHelper'])
    ## quic-l5-protocol.h (module 'quic'): ns3::QuicL5Protocol [class]
    module.add_class('QuicL5Protocol', parent=root_module['ns3::Object'])
    ## quic-socket-rx-buffer.h (module 'quic'): ns3::QuicSocketRxBuffer [class]
    module.add_class('QuicSocketRxBuffer', parent=root_module['ns3::Object'])
    ## quic-socket-tx-buffer.h (module 'quic'): ns3::QuicSocketTxBuffer [class]
    module.add_class('QuicSocketTxBuffer', parent=root_module['ns3::Object'])
    ## quic-stream.h (module 'quic'): ns3::QuicStream [class]
    module.add_class('QuicStream', parent=root_module['ns3::Object'])
    ## quic-stream.h (module 'quic'): ns3::QuicStream::QuicStreamTypes_t [enumeration]
    module.add_enum('QuicStreamTypes_t', ['CLIENT_INITIATED_BIDIRECTIONAL', 'SERVER_INITIATED_BIDIRECTIONAL', 'CLIENT_INITIATED_UNIDIRECTIONAL', 'SERVER_INITIATED_UNIDIRECTIONAL', 'NONE'], outer_class=root_module['ns3::QuicStream'])
    ## quic-stream.h (module 'quic'): ns3::QuicStream::QuicStreamDirectionTypes_t [enumeration]
    module.add_enum('QuicStreamDirectionTypes_t', ['SENDER', 'RECEIVER', 'BIDIRECTIONAL', 'UNKNOWN'], outer_class=root_module['ns3::QuicStream'])
    ## quic-stream.h (module 'quic'): ns3::QuicStream::QuicStreamStates_t [enumeration]
    module.add_enum('QuicStreamStates_t', ['IDLE', 'OPEN', 'SEND', 'RECV', 'SIZE_KNOWN', 'DATA_SENT', 'DATA_RECVD', 'DATA_READ', 'RESET_SENT', 'RESET_RECVD', 'RESET_READ', 'LAST_STATE'], outer_class=root_module['ns3::QuicStream'])
    typehandlers.add_type_alias(u'ns3::QuicStream::QuicStreamTypes_t', u'ns3::QuicStream::QuicStreamTypes_t')
    typehandlers.add_type_alias(u'ns3::QuicStream::QuicStreamTypes_t*', u'ns3::QuicStream::QuicStreamTypes_t*')
    typehandlers.add_type_alias(u'ns3::QuicStream::QuicStreamTypes_t&', u'ns3::QuicStream::QuicStreamTypes_t&')
    typehandlers.add_type_alias(u'ns3::QuicStream::QuicStreamDirectionTypes_t', u'ns3::QuicStream::QuicStreamDirectionTypes_t')
    typehandlers.add_type_alias(u'ns3::QuicStream::QuicStreamDirectionTypes_t*', u'ns3::QuicStream::QuicStreamDirectionTypes_t*')
    typehandlers.add_type_alias(u'ns3::QuicStream::QuicStreamDirectionTypes_t&', u'ns3::QuicStream::QuicStreamDirectionTypes_t&')
    typehandlers.add_type_alias(u'ns3::QuicStream::QuicStreamStates_t', u'ns3::QuicStream::QuicStreamStates_t')
    typehandlers.add_type_alias(u'ns3::QuicStream::QuicStreamStates_t*', u'ns3::QuicStream::QuicStreamStates_t*')
    typehandlers.add_type_alias(u'ns3::QuicStream::QuicStreamStates_t&', u'ns3::QuicStream::QuicStreamStates_t&')
    ## quic-stream-base.h (module 'quic'): ns3::QuicStreamBase [class]
    module.add_class('QuicStreamBase', parent=root_module['ns3::QuicStream'])
    ## quic-stream-rx-buffer.h (module 'quic'): ns3::QuicStreamRxBuffer [class]
    module.add_class('QuicStreamRxBuffer', parent=root_module['ns3::Object'])
    ## quic-stream-tx-buffer.h (module 'quic'): ns3::QuicStreamTxBuffer [class]
    module.add_class('QuicStreamTxBuffer', parent=root_module['ns3::Object'])
    ## quic-subheader.h (module 'quic'): ns3::QuicSubheader [class]
    module.add_class('QuicSubheader', parent=root_module['ns3::Header'])
    ## quic-subheader.h (module 'quic'): ns3::QuicSubheader::TypeFrame_t [enumeration]
    module.add_enum('TypeFrame_t', ['PADDING', 'RST_STREAM', 'CONNECTION_CLOSE', 'APPLICATION_CLOSE', 'MAX_DATA', 'MAX_STREAM_DATA', 'MAX_STREAM_ID', 'PING', 'BLOCKED', 'STREAM_BLOCKED', 'STREAM_ID_BLOCKED', 'NEW_CONNECTION_ID', 'STOP_SENDING', 'ACK', 'PATH_CHALLENGE', 'PATH_RESPONSE', 'STREAM000', 'STREAM001', 'STREAM010', 'STREAM011', 'STREAM100', 'STREAM101', 'STREAM110', 'STREAM111'], outer_class=root_module['ns3::QuicSubheader'])
    ## quic-subheader.h (module 'quic'): ns3::QuicSubheader::TransportErrorCodes_t [enumeration]
    module.add_enum('TransportErrorCodes_t', ['NO_ERROR', 'INTERNAL_ERROR', 'SERVER_BUSY', 'FLOW_CONTROL_ERROR', 'STREAM_ID_ERROR', 'STREAM_STATE_ERROR', 'FINAL_OFFSET_ERROR', 'FRAME_FORMAT_ERROR', 'TRANSPORT_PARAMETER_ERROR', 'VERSION_NEGOTIATION_ERROR', 'PROTOCOL_VIOLATION', 'UNSOLICITED_PATH_ERROR', 'FRAME_ERROR'], outer_class=root_module['ns3::QuicSubheader'])
    typehandlers.add_type_alias(u'ns3::QuicSubheader::TypeFrame_t', u'ns3::QuicSubheader::TypeFrame_t')
    typehandlers.add_type_alias(u'ns3::QuicSubheader::TypeFrame_t*', u'ns3::QuicSubheader::TypeFrame_t*')
    typehandlers.add_type_alias(u'ns3::QuicSubheader::TypeFrame_t&', u'ns3::QuicSubheader::TypeFrame_t&')
    typehandlers.add_type_alias(u'ns3::QuicSubheader::TransportErrorCodes_t', u'ns3::QuicSubheader::TransportErrorCodes_t')
    typehandlers.add_type_alias(u'ns3::QuicSubheader::TransportErrorCodes_t*', u'ns3::QuicSubheader::TransportErrorCodes_t*')
    typehandlers.add_type_alias(u'ns3::QuicSubheader::TransportErrorCodes_t&', u'ns3::QuicSubheader::TransportErrorCodes_t&')
    ## quic-transport-parameters.h (module 'quic'): ns3::QuicTransportParameters [class]
    module.add_class('QuicTransportParameters', parent=root_module['ns3::Header'])
    ## quic-l4-protocol.h (module 'quic'): ns3::QuicUdpBinding [class]
    module.add_class('QuicUdpBinding', parent=root_module['ns3::Object'])
    ## simple-ref-count.h (module 'core'): ns3::SimpleRefCount<ns3::AttributeAccessor, ns3::empty, ns3::DefaultDeleter<ns3::AttributeAccessor> > [class]
    module.add_class('SimpleRefCount', automatic_type_narrowing=True, import_from_module='ns.core', template_parameters=['ns3::AttributeAccessor', 'ns3::empty', 'ns3::DefaultDeleter<ns3::AttributeAccessor>'], parent=root_module['ns3::empty'], memory_policy=cppclass.ReferenceCountingMethodsPolicy(incref_method='Ref', decref_method='Unref', peekref_method='GetReferenceCount'))
    ## simple-ref-count.h (module 'core'): ns3::SimpleRefCount<ns3::AttributeChecker, ns3::empty, ns3::DefaultDeleter<ns3::AttributeChecker> > [class]
    module.add_class('SimpleRefCount', automatic_type_narrowing=True, import_from_module='ns.core', template_parameters=['ns3::AttributeChecker', 'ns3::empty', 'ns3::DefaultDeleter<ns3::AttributeChecker>'], parent=root_module['ns3::empty'], memory_policy=cppclass.ReferenceCountingMethodsPolicy(incref_method='Ref', decref_method='Unref', peekref_method='GetReferenceCount'))
    ## simple-ref-count.h (module 'core'): ns3::SimpleRefCount<ns3::AttributeValue, ns3::empty, ns3::DefaultDeleter<ns3::AttributeValue> > [class]
    module.add_class('SimpleRefCount', automatic_type_narrowing=True, import_from_module='ns.core', template_parameters=['ns3::AttributeValue', 'ns3::empty', 'ns3::DefaultDeleter<ns3::AttributeValue>'], parent=root_module['ns3::empty'], memory_policy=cppclass.ReferenceCountingMethodsPolicy(incref_method='Ref', decref_method='Unref', peekref_method='GetReferenceCount'))
    ## simple-ref-count.h (module 'core'): ns3::SimpleRefCount<ns3::CallbackImplBase, ns3::empty, ns3::DefaultDeleter<ns3::CallbackImplBase> > [class]
    module.add_class('SimpleRefCount', automatic_type_narrowing=True, import_from_module='ns.core', template_parameters=['ns3::CallbackImplBase', 'ns3::empty', 'ns3::DefaultDeleter<ns3::CallbackImplBase>'], parent=root_module['ns3::empty'], memory_policy=cppclass.ReferenceCountingMethodsPolicy(incref_method='Ref', decref_method='Unref', peekref_method='GetReferenceCount'))
    ## simple-ref-count.h (module 'core'): ns3::SimpleRefCount<ns3::EventImpl, ns3::empty, ns3::DefaultDeleter<ns3::EventImpl> > [class]
    module.add_class('SimpleRefCount', automatic_type_narrowing=True, import_from_module='ns.core', template_parameters=['ns3::EventImpl', 'ns3::empty', 'ns3::DefaultDeleter<ns3::EventImpl>'], parent=root_module['ns3::empty'], memory_policy=cppclass.ReferenceCountingMethodsPolicy(incref_method='Ref', decref_method='Unref', peekref_method='GetReferenceCount'))
    ## simple-ref-count.h (module 'core'): ns3::SimpleRefCount<ns3::Hash::Implementation, ns3::empty, ns3::DefaultDeleter<ns3::Hash::Implementation> > [class]
    module.add_class('SimpleRefCount', automatic_type_narrowing=True, import_from_module='ns.core', template_parameters=['ns3::Hash::Implementation', 'ns3::empty', 'ns3::DefaultDeleter<ns3::Hash::Implementation>'], parent=root_module['ns3::empty'], memory_policy=cppclass.ReferenceCountingMethodsPolicy(incref_method='Ref', decref_method='Unref', peekref_method='GetReferenceCount'))
    ## simple-ref-count.h (module 'core'): ns3::SimpleRefCount<ns3::Ipv4MulticastRoute, ns3::empty, ns3::DefaultDeleter<ns3::Ipv4MulticastRoute> > [class]
    module.add_class('SimpleRefCount', automatic_type_narrowing=True, import_from_module='ns.core', template_parameters=['ns3::Ipv4MulticastRoute', 'ns3::empty', 'ns3::DefaultDeleter<ns3::Ipv4MulticastRoute>'], parent=root_module['ns3::empty'], memory_policy=cppclass.ReferenceCountingMethodsPolicy(incref_method='Ref', decref_method='Unref', peekref_method='GetReferenceCount'))
    ## simple-ref-count.h (module 'core'): ns3::SimpleRefCount<ns3::Ipv4Route, ns3::empty, ns3::DefaultDeleter<ns3::Ipv4Route> > [class]
    module.add_class('SimpleRefCount', automatic_type_narrowing=True, import_from_module='ns.core', template_parameters=['ns3::Ipv4Route', 'ns3::empty', 'ns3::DefaultDeleter<ns3::Ipv4Route>'], parent=root_module['ns3::empty'], memory_policy=cppclass.ReferenceCountingMethodsPolicy(incref_method='Ref', decref_method='Unref', peekref_method='GetReferenceCount'))
    ## simple-ref-count.h (module 'core'): ns3::SimpleRefCount<ns3::NixVector, ns3::empty, ns3::DefaultDeleter<ns3::NixVector> > [class]
    module.add_class('SimpleRefCount', automatic_type_narrowing=True, import_from_module='ns.core', template_parameters=['ns3::NixVector', 'ns3::empty', 'ns3::DefaultDeleter<ns3::NixVector>'], parent=root_module['ns3::empty'], memory_policy=cppclass.ReferenceCountingMethodsPolicy(incref_method='Ref', decref_method='Unref', peekref_method='GetReferenceCount'))
    ## simple-ref-count.h (module 'core'): ns3::SimpleRefCount<ns3::OutputStreamWrapper, ns3::empty, ns3::DefaultDeleter<ns3::OutputStreamWrapper> > [class]
    module.add_class('SimpleRefCount', automatic_type_narrowing=True, import_from_module='ns.core', template_parameters=['ns3::OutputStreamWrapper', 'ns3::empty', 'ns3::DefaultDeleter<ns3::OutputStreamWrapper>'], parent=root_module['ns3::empty'], memory_policy=cppclass.ReferenceCountingMethodsPolicy(incref_method='Ref', decref_method='Unref', peekref_method='GetReferenceCount'))
    ## simple-ref-count.h (module 'core'): ns3::SimpleRefCount<ns3::Packet, ns3::empty, ns3::DefaultDeleter<ns3::Packet> > [class]
    module.add_class('SimpleRefCount', automatic_type_narrowing=True, import_from_module='ns.core', template_parameters=['ns3::Packet', 'ns3::empty', 'ns3::DefaultDeleter<ns3::Packet>'], parent=root_module['ns3::empty'], memory_policy=cppclass.ReferenceCountingMethodsPolicy(incref_method='Ref', decref_method='Unref', peekref_method='GetReferenceCount'))
    ## simple-ref-count.h (module 'core'): ns3::SimpleRefCount<ns3::TraceSourceAccessor, ns3::empty, ns3::DefaultDeleter<ns3::TraceSourceAccessor> > [class]
    module.add_class('SimpleRefCount', automatic_type_narrowing=True, import_from_module='ns.core', template_parameters=['ns3::TraceSourceAccessor', 'ns3::empty', 'ns3::DefaultDeleter<ns3::TraceSourceAccessor>'], parent=root_module['ns3::empty'], memory_policy=cppclass.ReferenceCountingMethodsPolicy(incref_method='Ref', decref_method='Unref', peekref_method='GetReferenceCount'))
    ## socket.h (module 'network'): ns3::Socket [class]
    module.add_class('Socket', import_from_module='ns.network', parent=root_module['ns3::Object'])
    ## socket.h (module 'network'): ns3::Socket::SocketErrno [enumeration]
    module.add_enum('SocketErrno', ['ERROR_NOTERROR', 'ERROR_ISCONN', 'ERROR_NOTCONN', 'ERROR_MSGSIZE', 'ERROR_AGAIN', 'ERROR_SHUTDOWN', 'ERROR_OPNOTSUPP', 'ERROR_AFNOSUPPORT', 'ERROR_INVAL', 'ERROR_BADF', 'ERROR_NOROUTETOHOST', 'ERROR_NODEV', 'ERROR_ADDRNOTAVAIL', 'ERROR_ADDRINUSE', 'SOCKET_ERRNO_LAST'], outer_class=root_module['ns3::Socket'], import_from_module='ns.network')
    ## socket.h (module 'network'): ns3::Socket::SocketType [enumeration]
    module.add_enum('SocketType', ['NS3_SOCK_STREAM', 'NS3_SOCK_SEQPACKET', 'NS3_SOCK_DGRAM', 'NS3_SOCK_RAW'], outer_class=root_module['ns3::Socket'], import_from_module='ns.network')
    ## socket.h (module 'network'): ns3::Socket::SocketPriority [enumeration]
    module.add_enum('SocketPriority', ['NS3_PRIO_BESTEFFORT', 'NS3_PRIO_FILLER', 'NS3_PRIO_BULK', 'NS3_PRIO_INTERACTIVE_BULK', 'NS3_PRIO_INTERACTIVE', 'NS3_PRIO_CONTROL'], outer_class=root_module['ns3::Socket'], import_from_module='ns.network')
    ## socket.h (module 'network'): ns3::Socket::Ipv6MulticastFilterMode [enumeration]
    module.add_enum('Ipv6MulticastFilterMode', ['INCLUDE', 'EXCLUDE'], outer_class=root_module['ns3::Socket'], import_from_module='ns.network')
    ## socket-factory.h (module 'network'): ns3::SocketFactory [class]
    module.add_class('SocketFactory', import_from_module='ns.network', parent=root_module['ns3::Object'])
    ## socket.h (module 'network'): ns3::SocketIpTosTag [class]
    module.add_class('SocketIpTosTag', import_from_module='ns.network', parent=root_module['ns3::Tag'])
    ## socket.h (module 'network'): ns3::SocketIpTtlTag [class]
    module.add_class('SocketIpTtlTag', import_from_module='ns.network', parent=root_module['ns3::Tag'])
    ## socket.h (module 'network'): ns3::SocketIpv6HopLimitTag [class]
    module.add_class('SocketIpv6HopLimitTag', import_from_module='ns.network', parent=root_module['ns3::Tag'])
    ## socket.h (module 'network'): ns3::SocketIpv6TclassTag [class]
    module.add_class('SocketIpv6TclassTag', import_from_module='ns.network', parent=root_module['ns3::Tag'])
    ## socket.h (module 'network'): ns3::SocketPriorityTag [class]
    module.add_class('SocketPriorityTag', import_from_module='ns.network', parent=root_module['ns3::Tag'])
    ## socket.h (module 'network'): ns3::SocketSetDontFragmentTag [class]
    module.add_class('SocketSetDontFragmentTag', import_from_module='ns.network', parent=root_module['ns3::Tag'])
    ## tcp-congestion-ops.h (module 'internet'): ns3::TcpCongestionOps [class]
    module.add_class('TcpCongestionOps', import_from_module='ns.internet', parent=root_module['ns3::Object'])
    ## tcp-congestion-ops.h (module 'internet'): ns3::TcpNewReno [class]
    module.add_class('TcpNewReno', import_from_module='ns.internet', parent=root_module['ns3::TcpCongestionOps'])
    ## tcp-socket.h (module 'internet'): ns3::TcpSocket [class]
    module.add_class('TcpSocket', import_from_module='ns.internet', parent=root_module['ns3::Socket'])
    ## tcp-socket.h (module 'internet'): ns3::TcpSocket::TcpStates_t [enumeration]
    module.add_enum('TcpStates_t', ['CLOSED', 'LISTEN', 'SYN_SENT', 'SYN_RCVD', 'ESTABLISHED', 'CLOSE_WAIT', 'LAST_ACK', 'FIN_WAIT_1', 'FIN_WAIT_2', 'CLOSING', 'TIME_WAIT', 'LAST_STATE'], outer_class=root_module['ns3::TcpSocket'], import_from_module='ns.internet')
    typehandlers.add_type_alias(u'ns3::TcpSocket::TcpStates_t', u'ns3::TcpSocket::TcpStates_t')
    typehandlers.add_type_alias(u'ns3::TcpSocket::TcpStates_t*', u'ns3::TcpSocket::TcpStates_t*')
    typehandlers.add_type_alias(u'ns3::TcpSocket::TcpStates_t&', u'ns3::TcpSocket::TcpStates_t&')
    ## tcp-socket-base.h (module 'internet'): ns3::TcpSocketBase [class]
    module.add_class('TcpSocketBase', import_from_module='ns.internet', parent=root_module['ns3::TcpSocket'])
    ## tcp-socket-base.h (module 'internet'): ns3::TcpSocketBase::EcnMode_t [enumeration]
    module.add_enum('EcnMode_t', ['NoEcn', 'ClassicEcn'], outer_class=root_module['ns3::TcpSocketBase'], import_from_module='ns.internet')
    typehandlers.add_type_alias(u'ns3::TcpSocketBase::EcnMode_t', u'ns3::TcpSocketBase::EcnMode_t')
    typehandlers.add_type_alias(u'ns3::TcpSocketBase::EcnMode_t*', u'ns3::TcpSocketBase::EcnMode_t*')
    typehandlers.add_type_alias(u'ns3::TcpSocketBase::EcnMode_t&', u'ns3::TcpSocketBase::EcnMode_t&')
    typehandlers.add_type_alias(u'void ( * ) ( ns3::Ptr< ns3::Packet const > const, ns3::TcpHeader const &, ns3::Ptr< ns3::TcpSocketBase const > const )', u'ns3::TcpSocketBase::TcpTxRxTracedCallback')
    typehandlers.add_type_alias(u'void ( * ) ( ns3::Ptr< ns3::Packet const > const, ns3::TcpHeader const &, ns3::Ptr< ns3::TcpSocketBase const > const )*', u'ns3::TcpSocketBase::TcpTxRxTracedCallback*')
    typehandlers.add_type_alias(u'void ( * ) ( ns3::Ptr< ns3::Packet const > const, ns3::TcpHeader const &, ns3::Ptr< ns3::TcpSocketBase const > const )&', u'ns3::TcpSocketBase::TcpTxRxTracedCallback&')
    ## tcp-socket-state.h (module 'internet'): ns3::TcpSocketState [class]
    module.add_class('TcpSocketState', import_from_module='ns.internet', parent=root_module['ns3::Object'])
    ## tcp-socket-state.h (module 'internet'): ns3::TcpSocketState::TcpCongState_t [enumeration]
    module.add_enum('TcpCongState_t', ['CA_OPEN', 'CA_DISORDER', 'CA_CWR', 'CA_RECOVERY', 'CA_LOSS', 'CA_LAST_STATE'], outer_class=root_module['ns3::TcpSocketState'], import_from_module='ns.internet')
    ## tcp-socket-state.h (module 'internet'): ns3::TcpSocketState::TcpCAEvent_t [enumeration]
    module.add_enum('TcpCAEvent_t', ['CA_EVENT_TX_START', 'CA_EVENT_CWND_RESTART', 'CA_EVENT_COMPLETE_CWR', 'CA_EVENT_LOSS', 'CA_EVENT_ECN_NO_CE', 'CA_EVENT_ECN_IS_CE', 'CA_EVENT_DELAYED_ACK', 'CA_EVENT_NON_DELAYED_ACK'], outer_class=root_module['ns3::TcpSocketState'], import_from_module='ns.internet')
    ## tcp-socket-state.h (module 'internet'): ns3::TcpSocketState::EcnState_t [enumeration]
    module.add_enum('EcnState_t', ['ECN_DISABLED', 'ECN_IDLE', 'ECN_CE_RCVD', 'ECN_SENDING_ECE', 'ECN_ECE_RCVD', 'ECN_CWR_SENT'], outer_class=root_module['ns3::TcpSocketState'], import_from_module='ns.internet')
    typehandlers.add_type_alias(u'ns3::TcpSocketState::TcpCongState_t', u'ns3::TcpSocketState::TcpCongState_t')
    typehandlers.add_type_alias(u'ns3::TcpSocketState::TcpCongState_t*', u'ns3::TcpSocketState::TcpCongState_t*')
    typehandlers.add_type_alias(u'ns3::TcpSocketState::TcpCongState_t&', u'ns3::TcpSocketState::TcpCongState_t&')
    typehandlers.add_type_alias(u'ns3::TcpSocketState::TcpCAEvent_t', u'ns3::TcpSocketState::TcpCAEvent_t')
    typehandlers.add_type_alias(u'ns3::TcpSocketState::TcpCAEvent_t*', u'ns3::TcpSocketState::TcpCAEvent_t*')
    typehandlers.add_type_alias(u'ns3::TcpSocketState::TcpCAEvent_t&', u'ns3::TcpSocketState::TcpCAEvent_t&')
    typehandlers.add_type_alias(u'ns3::TcpSocketState::EcnState_t', u'ns3::TcpSocketState::EcnState_t')
    typehandlers.add_type_alias(u'ns3::TcpSocketState::EcnState_t*', u'ns3::TcpSocketState::EcnState_t*')
    typehandlers.add_type_alias(u'ns3::TcpSocketState::EcnState_t&', u'ns3::TcpSocketState::EcnState_t&')
    ## nstime.h (module 'core'): ns3::Time [class]
    module.add_class('Time', import_from_module='ns.core')
    ## nstime.h (module 'core'): ns3::Time::Unit [enumeration]
    module.add_enum('Unit', ['Y', 'D', 'H', 'MIN', 'S', 'MS', 'US', 'NS', 'PS', 'FS', 'LAST'], outer_class=root_module['ns3::Time'], import_from_module='ns.core')
    typehandlers.add_type_alias(u'void ( * ) ( ns3::Time )', u'ns3::Time::TracedCallback')
    typehandlers.add_type_alias(u'void ( * ) ( ns3::Time )*', u'ns3::Time::TracedCallback*')
    typehandlers.add_type_alias(u'void ( * ) ( ns3::Time )&', u'ns3::Time::TracedCallback&')
    ## nstime.h (module 'core'): ns3::Time [class]
    root_module['ns3::Time'].implicitly_converts_to(root_module['ns3::int64x64_t'])
    ## trace-source-accessor.h (module 'core'): ns3::TraceSourceAccessor [class]
    module.add_class('TraceSourceAccessor', import_from_module='ns.core', parent=root_module['ns3::SimpleRefCount< ns3::TraceSourceAccessor, ns3::empty, ns3::DefaultDeleter<ns3::TraceSourceAccessor> >'])
    ## traced-value.h (module 'core'): ns3::TracedValue<ns3::Time> [class]
    module.add_class('TracedValue', import_from_module='ns.core', template_parameters=['ns3::Time'])
    ## traced-value.h (module 'core'): ns3::TracedValue<ns3::Time> [class]
    root_module['ns3::TracedValue< ns3::Time >'].implicitly_converts_to(root_module['ns3::Time'])
    ## trailer.h (module 'network'): ns3::Trailer [class]
    module.add_class('Trailer', import_from_module='ns.network', parent=root_module['ns3::Chunk'])
    ## attribute.h (module 'core'): ns3::AttributeAccessor [class]
    module.add_class('AttributeAccessor', import_from_module='ns.core', parent=root_module['ns3::SimpleRefCount< ns3::AttributeAccessor, ns3::empty, ns3::DefaultDeleter<ns3::AttributeAccessor> >'])
    ## attribute.h (module 'core'): ns3::AttributeChecker [class]
    module.add_class('AttributeChecker', allow_subclassing=False, automatic_type_narrowing=True, import_from_module='ns.core', parent=root_module['ns3::SimpleRefCount< ns3::AttributeChecker, ns3::empty, ns3::DefaultDeleter<ns3::AttributeChecker> >'])
    ## attribute.h (module 'core'): ns3::AttributeValue [class]
    module.add_class('AttributeValue', allow_subclassing=False, automatic_type_narrowing=True, import_from_module='ns.core', parent=root_module['ns3::SimpleRefCount< ns3::AttributeValue, ns3::empty, ns3::DefaultDeleter<ns3::AttributeValue> >'])
    ## boolean.h (module 'core'): ns3::BooleanChecker [class]
    module.add_class('BooleanChecker', import_from_module='ns.core', parent=root_module['ns3::AttributeChecker'])
    ## boolean.h (module 'core'): ns3::BooleanValue [class]
    module.add_class('BooleanValue', import_from_module='ns.core', parent=root_module['ns3::AttributeValue'])
    ## callback.h (module 'core'): ns3::CallbackChecker [class]
    module.add_class('CallbackChecker', import_from_module='ns.core', parent=root_module['ns3::AttributeChecker'])
    ## callback.h (module 'core'): ns3::CallbackImplBase [class]
    module.add_class('CallbackImplBase', import_from_module='ns.core', parent=root_module['ns3::SimpleRefCount< ns3::CallbackImplBase, ns3::empty, ns3::DefaultDeleter<ns3::CallbackImplBase> >'])
    ## callback.h (module 'core'): ns3::CallbackValue [class]
    module.add_class('CallbackValue', import_from_module='ns.core', parent=root_module['ns3::AttributeValue'])
    ## data-rate.h (module 'network'): ns3::DataRateChecker [class]
    module.add_class('DataRateChecker', import_from_module='ns.network', parent=root_module['ns3::AttributeChecker'])
    ## data-rate.h (module 'network'): ns3::DataRateValue [class]
    module.add_class('DataRateValue', import_from_module='ns.network', parent=root_module['ns3::AttributeValue'])
    ## double.h (module 'core'): ns3::DoubleValue [class]
    module.add_class('DoubleValue', import_from_module='ns.core', parent=root_module['ns3::AttributeValue'])
    ## attribute.h (module 'core'): ns3::EmptyAttributeAccessor [class]
    module.add_class('EmptyAttributeAccessor', import_from_module='ns.core', parent=root_module['ns3::AttributeAccessor'])
    ## attribute.h (module 'core'): ns3::EmptyAttributeChecker [class]
    module.add_class('EmptyAttributeChecker', import_from_module='ns.core', parent=root_module['ns3::AttributeChecker'])
    ## attribute.h (module 'core'): ns3::EmptyAttributeValue [class]
    module.add_class('EmptyAttributeValue', import_from_module='ns.core', parent=root_module['ns3::AttributeValue'])
    ## enum.h (module 'core'): ns3::EnumChecker [class]
    module.add_class('EnumChecker', import_from_module='ns.core', parent=root_module['ns3::AttributeChecker'])
    ## enum.h (module 'core'): ns3::EnumValue [class]
    module.add_class('EnumValue', import_from_module='ns.core', parent=root_module['ns3::AttributeValue'])
    ## event-impl.h (module 'core'): ns3::EventImpl [class]
    module.add_class('EventImpl', import_from_module='ns.core', parent=root_module['ns3::SimpleRefCount< ns3::EventImpl, ns3::empty, ns3::DefaultDeleter<ns3::EventImpl> >'])
    ## integer.h (module 'core'): ns3::IntegerValue [class]
    module.add_class('IntegerValue', import_from_module='ns.core', parent=root_module['ns3::AttributeValue'])
    ## ip-l4-protocol.h (module 'internet'): ns3::IpL4Protocol [class]
    module.add_class('IpL4Protocol', import_from_module='ns.internet', parent=root_module['ns3::Object'])
    ## ip-l4-protocol.h (module 'internet'): ns3::IpL4Protocol::RxStatus [enumeration]
    module.add_enum('RxStatus', ['RX_OK', 'RX_CSUM_FAILED', 'RX_ENDPOINT_CLOSED', 'RX_ENDPOINT_UNREACH'], outer_class=root_module['ns3::IpL4Protocol'], import_from_module='ns.internet')
    typehandlers.add_type_alias(u'ns3::Callback< void, ns3::Ptr< ns3::Packet >, ns3::Ipv4Address, ns3::Ipv4Address, unsigned char, ns3::Ptr< ns3::Ipv4Route >, ns3::empty, ns3::empty, ns3::empty, ns3::empty >', u'ns3::IpL4Protocol::DownTargetCallback')
    typehandlers.add_type_alias(u'ns3::Callback< void, ns3::Ptr< ns3::Packet >, ns3::Ipv4Address, ns3::Ipv4Address, unsigned char, ns3::Ptr< ns3::Ipv4Route >, ns3::empty, ns3::empty, ns3::empty, ns3::empty >*', u'ns3::IpL4Protocol::DownTargetCallback*')
    typehandlers.add_type_alias(u'ns3::Callback< void, ns3::Ptr< ns3::Packet >, ns3::Ipv4Address, ns3::Ipv4Address, unsigned char, ns3::Ptr< ns3::Ipv4Route >, ns3::empty, ns3::empty, ns3::empty, ns3::empty >&', u'ns3::IpL4Protocol::DownTargetCallback&')
    typehandlers.add_type_alias(u'ns3::Callback< void, ns3::Ptr< ns3::Packet >, ns3::Ipv6Address, ns3::Ipv6Address, unsigned char, ns3::Ptr< ns3::Ipv6Route >, ns3::empty, ns3::empty, ns3::empty, ns3::empty >', u'ns3::IpL4Protocol::DownTargetCallback6')
    typehandlers.add_type_alias(u'ns3::Callback< void, ns3::Ptr< ns3::Packet >, ns3::Ipv6Address, ns3::Ipv6Address, unsigned char, ns3::Ptr< ns3::Ipv6Route >, ns3::empty, ns3::empty, ns3::empty, ns3::empty >*', u'ns3::IpL4Protocol::DownTargetCallback6*')
    typehandlers.add_type_alias(u'ns3::Callback< void, ns3::Ptr< ns3::Packet >, ns3::Ipv6Address, ns3::Ipv6Address, unsigned char, ns3::Ptr< ns3::Ipv6Route >, ns3::empty, ns3::empty, ns3::empty, ns3::empty >&', u'ns3::IpL4Protocol::DownTargetCallback6&')
    ## ipv4.h (module 'internet'): ns3::Ipv4 [class]
    module.add_class('Ipv4', import_from_module='ns.internet', parent=root_module['ns3::Object'])
    ## ipv4-address.h (module 'network'): ns3::Ipv4AddressChecker [class]
    module.add_class('Ipv4AddressChecker', import_from_module='ns.network', parent=root_module['ns3::AttributeChecker'])
    ## ipv4-address.h (module 'network'): ns3::Ipv4AddressValue [class]
    module.add_class('Ipv4AddressValue', import_from_module='ns.network', parent=root_module['ns3::AttributeValue'])
    ## ipv4-l3-protocol.h (module 'internet'): ns3::Ipv4L3Protocol [class]
    module.add_class('Ipv4L3Protocol', import_from_module='ns.internet', parent=root_module['ns3::Ipv4'])
    ## ipv4-l3-protocol.h (module 'internet'): ns3::Ipv4L3Protocol::DropReason [enumeration]
    module.add_enum('DropReason', ['DROP_TTL_EXPIRED', 'DROP_NO_ROUTE', 'DROP_BAD_CHECKSUM', 'DROP_INTERFACE_DOWN', 'DROP_ROUTE_ERROR', 'DROP_FRAGMENT_TIMEOUT'], outer_class=root_module['ns3::Ipv4L3Protocol'], import_from_module='ns.internet')
    typehandlers.add_type_alias(u'void ( * ) ( ns3::Ipv4Header const &, ns3::Ptr< ns3::Packet const >, uint32_t )', u'ns3::Ipv4L3Protocol::SentTracedCallback')
    typehandlers.add_type_alias(u'void ( * ) ( ns3::Ipv4Header const &, ns3::Ptr< ns3::Packet const >, uint32_t )*', u'ns3::Ipv4L3Protocol::SentTracedCallback*')
    typehandlers.add_type_alias(u'void ( * ) ( ns3::Ipv4Header const &, ns3::Ptr< ns3::Packet const >, uint32_t )&', u'ns3::Ipv4L3Protocol::SentTracedCallback&')
    typehandlers.add_type_alias(u'void ( * ) ( ns3::Ptr< ns3::Packet const >, ns3::Ptr< ns3::Ipv4 >, uint32_t )', u'ns3::Ipv4L3Protocol::TxRxTracedCallback')
    typehandlers.add_type_alias(u'void ( * ) ( ns3::Ptr< ns3::Packet const >, ns3::Ptr< ns3::Ipv4 >, uint32_t )*', u'ns3::Ipv4L3Protocol::TxRxTracedCallback*')
    typehandlers.add_type_alias(u'void ( * ) ( ns3::Ptr< ns3::Packet const >, ns3::Ptr< ns3::Ipv4 >, uint32_t )&', u'ns3::Ipv4L3Protocol::TxRxTracedCallback&')
    typehandlers.add_type_alias(u'void ( * ) ( ns3::Ipv4Header const &, ns3::Ptr< ns3::Packet const >, ns3::Ipv4L3Protocol::DropReason, ns3::Ptr< ns3::Ipv4 >, uint32_t )', u'ns3::Ipv4L3Protocol::DropTracedCallback')
    typehandlers.add_type_alias(u'void ( * ) ( ns3::Ipv4Header const &, ns3::Ptr< ns3::Packet const >, ns3::Ipv4L3Protocol::DropReason, ns3::Ptr< ns3::Ipv4 >, uint32_t )*', u'ns3::Ipv4L3Protocol::DropTracedCallback*')
    typehandlers.add_type_alias(u'void ( * ) ( ns3::Ipv4Header const &, ns3::Ptr< ns3::Packet const >, ns3::Ipv4L3Protocol::DropReason, ns3::Ptr< ns3::Ipv4 >, uint32_t )&', u'ns3::Ipv4L3Protocol::DropTracedCallback&')
    ## ipv4-address.h (module 'network'): ns3::Ipv4MaskChecker [class]
    module.add_class('Ipv4MaskChecker', import_from_module='ns.network', parent=root_module['ns3::AttributeChecker'])
    ## ipv4-address.h (module 'network'): ns3::Ipv4MaskValue [class]
    module.add_class('Ipv4MaskValue', import_from_module='ns.network', parent=root_module['ns3::AttributeValue'])
    ## ipv4-route.h (module 'internet'): ns3::Ipv4MulticastRoute [class]
    module.add_class('Ipv4MulticastRoute', import_from_module='ns.internet', parent=root_module['ns3::SimpleRefCount< ns3::Ipv4MulticastRoute, ns3::empty, ns3::DefaultDeleter<ns3::Ipv4MulticastRoute> >'])
    ## ipv4-route.h (module 'internet'): ns3::Ipv4Route [class]
    module.add_class('Ipv4Route', import_from_module='ns.internet', parent=root_module['ns3::SimpleRefCount< ns3::Ipv4Route, ns3::empty, ns3::DefaultDeleter<ns3::Ipv4Route> >'])
    ## ipv4-routing-protocol.h (module 'internet'): ns3::Ipv4RoutingProtocol [class]
    module.add_class('Ipv4RoutingProtocol', import_from_module='ns.internet', parent=root_module['ns3::Object'])
    typehandlers.add_type_alias(u'ns3::Callback< void, ns3::Ptr< ns3::Ipv4Route >, ns3::Ptr< ns3::Packet const >, ns3::Ipv4Header const &, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty >', u'ns3::Ipv4RoutingProtocol::UnicastForwardCallback')
    typehandlers.add_type_alias(u'ns3::Callback< void, ns3::Ptr< ns3::Ipv4Route >, ns3::Ptr< ns3::Packet const >, ns3::Ipv4Header const &, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty >*', u'ns3::Ipv4RoutingProtocol::UnicastForwardCallback*')
    typehandlers.add_type_alias(u'ns3::Callback< void, ns3::Ptr< ns3::Ipv4Route >, ns3::Ptr< ns3::Packet const >, ns3::Ipv4Header const &, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty >&', u'ns3::Ipv4RoutingProtocol::UnicastForwardCallback&')
    typehandlers.add_type_alias(u'ns3::Callback< void, ns3::Ptr< ns3::Ipv4MulticastRoute >, ns3::Ptr< ns3::Packet const >, ns3::Ipv4Header const &, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty >', u'ns3::Ipv4RoutingProtocol::MulticastForwardCallback')
    typehandlers.add_type_alias(u'ns3::Callback< void, ns3::Ptr< ns3::Ipv4MulticastRoute >, ns3::Ptr< ns3::Packet const >, ns3::Ipv4Header const &, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty >*', u'ns3::Ipv4RoutingProtocol::MulticastForwardCallback*')
    typehandlers.add_type_alias(u'ns3::Callback< void, ns3::Ptr< ns3::Ipv4MulticastRoute >, ns3::Ptr< ns3::Packet const >, ns3::Ipv4Header const &, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty >&', u'ns3::Ipv4RoutingProtocol::MulticastForwardCallback&')
    typehandlers.add_type_alias(u'ns3::Callback< void, ns3::Ptr< ns3::Packet const >, ns3::Ipv4Header const &, unsigned int, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty >', u'ns3::Ipv4RoutingProtocol::LocalDeliverCallback')
    typehandlers.add_type_alias(u'ns3::Callback< void, ns3::Ptr< ns3::Packet const >, ns3::Ipv4Header const &, unsigned int, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty >*', u'ns3::Ipv4RoutingProtocol::LocalDeliverCallback*')
    typehandlers.add_type_alias(u'ns3::Callback< void, ns3::Ptr< ns3::Packet const >, ns3::Ipv4Header const &, unsigned int, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty >&', u'ns3::Ipv4RoutingProtocol::LocalDeliverCallback&')
    typehandlers.add_type_alias(u'ns3::Callback< void, ns3::Ptr< ns3::Packet const >, ns3::Ipv4Header const &, ns3::Socket::SocketErrno, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty >', u'ns3::Ipv4RoutingProtocol::ErrorCallback')
    typehandlers.add_type_alias(u'ns3::Callback< void, ns3::Ptr< ns3::Packet const >, ns3::Ipv4Header const &, ns3::Socket::SocketErrno, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty >*', u'ns3::Ipv4RoutingProtocol::ErrorCallback*')
    typehandlers.add_type_alias(u'ns3::Callback< void, ns3::Ptr< ns3::Packet const >, ns3::Ipv4Header const &, ns3::Socket::SocketErrno, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty >&', u'ns3::Ipv4RoutingProtocol::ErrorCallback&')
    ## ipv6.h (module 'internet'): ns3::Ipv6 [class]
    module.add_class('Ipv6', import_from_module='ns.internet', parent=root_module['ns3::Object'])
    ## ipv6-address.h (module 'network'): ns3::Ipv6AddressChecker [class]
    module.add_class('Ipv6AddressChecker', import_from_module='ns.network', parent=root_module['ns3::AttributeChecker'])
    ## ipv6-address.h (module 'network'): ns3::Ipv6AddressValue [class]
    module.add_class('Ipv6AddressValue', import_from_module='ns.network', parent=root_module['ns3::AttributeValue'])
    ## ipv6-l3-protocol.h (module 'internet'): ns3::Ipv6L3Protocol [class]
    module.add_class('Ipv6L3Protocol', import_from_module='ns.internet', parent=root_module['ns3::Ipv6'])
    ## ipv6-l3-protocol.h (module 'internet'): ns3::Ipv6L3Protocol::DropReason [enumeration]
    module.add_enum('DropReason', ['DROP_TTL_EXPIRED', 'DROP_NO_ROUTE', 'DROP_INTERFACE_DOWN', 'DROP_ROUTE_ERROR', 'DROP_UNKNOWN_PROTOCOL', 'DROP_UNKNOWN_OPTION', 'DROP_MALFORMED_HEADER', 'DROP_FRAGMENT_TIMEOUT'], outer_class=root_module['ns3::Ipv6L3Protocol'], import_from_module='ns.internet')
    typehandlers.add_type_alias(u'void ( * ) ( ns3::Ipv6Header const &, ns3::Ptr< ns3::Packet const >, uint32_t )', u'ns3::Ipv6L3Protocol::SentTracedCallback')
    typehandlers.add_type_alias(u'void ( * ) ( ns3::Ipv6Header const &, ns3::Ptr< ns3::Packet const >, uint32_t )*', u'ns3::Ipv6L3Protocol::SentTracedCallback*')
    typehandlers.add_type_alias(u'void ( * ) ( ns3::Ipv6Header const &, ns3::Ptr< ns3::Packet const >, uint32_t )&', u'ns3::Ipv6L3Protocol::SentTracedCallback&')
    typehandlers.add_type_alias(u'void ( * ) ( ns3::Ptr< ns3::Packet const >, ns3::Ptr< ns3::Ipv6 >, uint32_t )', u'ns3::Ipv6L3Protocol::TxRxTracedCallback')
    typehandlers.add_type_alias(u'void ( * ) ( ns3::Ptr< ns3::Packet const >, ns3::Ptr< ns3::Ipv6 >, uint32_t )*', u'ns3::Ipv6L3Protocol::TxRxTracedCallback*')
    typehandlers.add_type_alias(u'void ( * ) ( ns3::Ptr< ns3::Packet const >, ns3::Ptr< ns3::Ipv6 >, uint32_t )&', u'ns3::Ipv6L3Protocol::TxRxTracedCallback&')
    typehandlers.add_type_alias(u'void ( * ) ( ns3::Ipv6Header const &, ns3::Ptr< ns3::Packet const >, ns3::Ipv6L3Protocol::DropReason, ns3::Ptr< ns3::Ipv6 >, uint32_t )', u'ns3::Ipv6L3Protocol::DropTracedCallback')
    typehandlers.add_type_alias(u'void ( * ) ( ns3::Ipv6Header const &, ns3::Ptr< ns3::Packet const >, ns3::Ipv6L3Protocol::DropReason, ns3::Ptr< ns3::Ipv6 >, uint32_t )*', u'ns3::Ipv6L3Protocol::DropTracedCallback*')
    typehandlers.add_type_alias(u'void ( * ) ( ns3::Ipv6Header const &, ns3::Ptr< ns3::Packet const >, ns3::Ipv6L3Protocol::DropReason, ns3::Ptr< ns3::Ipv6 >, uint32_t )&', u'ns3::Ipv6L3Protocol::DropTracedCallback&')
    ## ipv6-pmtu-cache.h (module 'internet'): ns3::Ipv6PmtuCache [class]
    module.add_class('Ipv6PmtuCache', import_from_module='ns.internet', parent=root_module['ns3::Object'])
    ## ipv6-address.h (module 'network'): ns3::Ipv6PrefixChecker [class]
    module.add_class('Ipv6PrefixChecker', import_from_module='ns.network', parent=root_module['ns3::AttributeChecker'])
    ## ipv6-address.h (module 'network'): ns3::Ipv6PrefixValue [class]
    module.add_class('Ipv6PrefixValue', import_from_module='ns.network', parent=root_module['ns3::AttributeValue'])
    ## mac48-address.h (module 'network'): ns3::Mac48AddressChecker [class]
    module.add_class('Mac48AddressChecker', import_from_module='ns.network', parent=root_module['ns3::AttributeChecker'])
    ## mac48-address.h (module 'network'): ns3::Mac48AddressValue [class]
    module.add_class('Mac48AddressValue', import_from_module='ns.network', parent=root_module['ns3::AttributeValue'])
    ## net-device.h (module 'network'): ns3::NetDevice [class]
    module.add_class('NetDevice', import_from_module='ns.network', parent=root_module['ns3::Object'])
    ## net-device.h (module 'network'): ns3::NetDevice::PacketType [enumeration]
    module.add_enum('PacketType', ['PACKET_HOST', 'NS3_PACKET_HOST', 'PACKET_BROADCAST', 'NS3_PACKET_BROADCAST', 'PACKET_MULTICAST', 'NS3_PACKET_MULTICAST', 'PACKET_OTHERHOST', 'NS3_PACKET_OTHERHOST'], outer_class=root_module['ns3::NetDevice'], import_from_module='ns.network')
    typehandlers.add_type_alias(u'void ( * ) (  )', u'ns3::NetDevice::LinkChangeTracedCallback')
    typehandlers.add_type_alias(u'void ( * ) (  )*', u'ns3::NetDevice::LinkChangeTracedCallback*')
    typehandlers.add_type_alias(u'void ( * ) (  )&', u'ns3::NetDevice::LinkChangeTracedCallback&')
    typehandlers.add_type_alias(u'ns3::Callback< bool, ns3::Ptr< ns3::NetDevice >, ns3::Ptr< ns3::Packet const >, unsigned short, ns3::Address const &, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty >', u'ns3::NetDevice::ReceiveCallback')
    typehandlers.add_type_alias(u'ns3::Callback< bool, ns3::Ptr< ns3::NetDevice >, ns3::Ptr< ns3::Packet const >, unsigned short, ns3::Address const &, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty >*', u'ns3::NetDevice::ReceiveCallback*')
    typehandlers.add_type_alias(u'ns3::Callback< bool, ns3::Ptr< ns3::NetDevice >, ns3::Ptr< ns3::Packet const >, unsigned short, ns3::Address const &, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty >&', u'ns3::NetDevice::ReceiveCallback&')
    typehandlers.add_type_alias(u'ns3::Callback< bool, ns3::Ptr< ns3::NetDevice >, ns3::Ptr< ns3::Packet const >, unsigned short, ns3::Address const &, ns3::Address const &, ns3::NetDevice::PacketType, ns3::empty, ns3::empty, ns3::empty >', u'ns3::NetDevice::PromiscReceiveCallback')
    typehandlers.add_type_alias(u'ns3::Callback< bool, ns3::Ptr< ns3::NetDevice >, ns3::Ptr< ns3::Packet const >, unsigned short, ns3::Address const &, ns3::Address const &, ns3::NetDevice::PacketType, ns3::empty, ns3::empty, ns3::empty >*', u'ns3::NetDevice::PromiscReceiveCallback*')
    typehandlers.add_type_alias(u'ns3::Callback< bool, ns3::Ptr< ns3::NetDevice >, ns3::Ptr< ns3::Packet const >, unsigned short, ns3::Address const &, ns3::Address const &, ns3::NetDevice::PacketType, ns3::empty, ns3::empty, ns3::empty >&', u'ns3::NetDevice::PromiscReceiveCallback&')
    ## nix-vector.h (module 'network'): ns3::NixVector [class]
    module.add_class('NixVector', import_from_module='ns.network', parent=root_module['ns3::SimpleRefCount< ns3::NixVector, ns3::empty, ns3::DefaultDeleter<ns3::NixVector> >'])
    ## node.h (module 'network'): ns3::Node [class]
    module.add_class('Node', import_from_module='ns.network', parent=root_module['ns3::Object'])
    typehandlers.add_type_alias(u'ns3::Callback< void, ns3::Ptr< ns3::NetDevice >, ns3::Ptr< ns3::Packet const >, unsigned short, ns3::Address const &, ns3::Address const &, ns3::NetDevice::PacketType, ns3::empty, ns3::empty, ns3::empty >', u'ns3::Node::ProtocolHandler')
    typehandlers.add_type_alias(u'ns3::Callback< void, ns3::Ptr< ns3::NetDevice >, ns3::Ptr< ns3::Packet const >, unsigned short, ns3::Address const &, ns3::Address const &, ns3::NetDevice::PacketType, ns3::empty, ns3::empty, ns3::empty >*', u'ns3::Node::ProtocolHandler*')
    typehandlers.add_type_alias(u'ns3::Callback< void, ns3::Ptr< ns3::NetDevice >, ns3::Ptr< ns3::Packet const >, unsigned short, ns3::Address const &, ns3::Address const &, ns3::NetDevice::PacketType, ns3::empty, ns3::empty, ns3::empty >&', u'ns3::Node::ProtocolHandler&')
    typehandlers.add_type_alias(u'ns3::Callback< void, ns3::Ptr< ns3::NetDevice >, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty >', u'ns3::Node::DeviceAdditionListener')
    typehandlers.add_type_alias(u'ns3::Callback< void, ns3::Ptr< ns3::NetDevice >, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty >*', u'ns3::Node::DeviceAdditionListener*')
    typehandlers.add_type_alias(u'ns3::Callback< void, ns3::Ptr< ns3::NetDevice >, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty >&', u'ns3::Node::DeviceAdditionListener&')
    ## object-factory.h (module 'core'): ns3::ObjectFactoryChecker [class]
    module.add_class('ObjectFactoryChecker', import_from_module='ns.core', parent=root_module['ns3::AttributeChecker'])
    ## object-factory.h (module 'core'): ns3::ObjectFactoryValue [class]
    module.add_class('ObjectFactoryValue', import_from_module='ns.core', parent=root_module['ns3::AttributeValue'])
    ## output-stream-wrapper.h (module 'network'): ns3::OutputStreamWrapper [class]
    module.add_class('OutputStreamWrapper', import_from_module='ns.network', parent=root_module['ns3::SimpleRefCount< ns3::OutputStreamWrapper, ns3::empty, ns3::DefaultDeleter<ns3::OutputStreamWrapper> >'])
    ## packet.h (module 'network'): ns3::Packet [class]
    module.add_class('Packet', import_from_module='ns.network', parent=root_module['ns3::SimpleRefCount< ns3::Packet, ns3::empty, ns3::DefaultDeleter<ns3::Packet> >'])
    typehandlers.add_type_alias(u'void ( * ) ( ns3::Ptr< ns3::Packet const > )', u'ns3::Packet::TracedCallback')
    typehandlers.add_type_alias(u'void ( * ) ( ns3::Ptr< ns3::Packet const > )*', u'ns3::Packet::TracedCallback*')
    typehandlers.add_type_alias(u'void ( * ) ( ns3::Ptr< ns3::Packet const > )&', u'ns3::Packet::TracedCallback&')
    typehandlers.add_type_alias(u'void ( * ) ( ns3::Ptr< ns3::Packet const >, ns3::Address const & )', u'ns3::Packet::AddressTracedCallback')
    typehandlers.add_type_alias(u'void ( * ) ( ns3::Ptr< ns3::Packet const >, ns3::Address const & )*', u'ns3::Packet::AddressTracedCallback*')
    typehandlers.add_type_alias(u'void ( * ) ( ns3::Ptr< ns3::Packet const >, ns3::Address const & )&', u'ns3::Packet::AddressTracedCallback&')
    typehandlers.add_type_alias(u'void ( * ) ( ns3::Ptr< ns3::Packet const > const, ns3::Address const &, ns3::Address const & )', u'ns3::Packet::TwoAddressTracedCallback')
    typehandlers.add_type_alias(u'void ( * ) ( ns3::Ptr< ns3::Packet const > const, ns3::Address const &, ns3::Address const & )*', u'ns3::Packet::TwoAddressTracedCallback*')
    typehandlers.add_type_alias(u'void ( * ) ( ns3::Ptr< ns3::Packet const > const, ns3::Address const &, ns3::Address const & )&', u'ns3::Packet::TwoAddressTracedCallback&')
    typehandlers.add_type_alias(u'void ( * ) ( ns3::Ptr< ns3::Packet const >, ns3::Mac48Address )', u'ns3::Packet::Mac48AddressTracedCallback')
    typehandlers.add_type_alias(u'void ( * ) ( ns3::Ptr< ns3::Packet const >, ns3::Mac48Address )*', u'ns3::Packet::Mac48AddressTracedCallback*')
    typehandlers.add_type_alias(u'void ( * ) ( ns3::Ptr< ns3::Packet const >, ns3::Mac48Address )&', u'ns3::Packet::Mac48AddressTracedCallback&')
    typehandlers.add_type_alias(u'void ( * ) ( uint32_t, uint32_t )', u'ns3::Packet::SizeTracedCallback')
    typehandlers.add_type_alias(u'void ( * ) ( uint32_t, uint32_t )*', u'ns3::Packet::SizeTracedCallback*')
    typehandlers.add_type_alias(u'void ( * ) ( uint32_t, uint32_t )&', u'ns3::Packet::SizeTracedCallback&')
    typehandlers.add_type_alias(u'void ( * ) ( ns3::Ptr< ns3::Packet const >, double )', u'ns3::Packet::SinrTracedCallback')
    typehandlers.add_type_alias(u'void ( * ) ( ns3::Ptr< ns3::Packet const >, double )*', u'ns3::Packet::SinrTracedCallback*')
    typehandlers.add_type_alias(u'void ( * ) ( ns3::Ptr< ns3::Packet const >, double )&', u'ns3::Packet::SinrTracedCallback&')
    ## quic-congestion-ops.h (module 'quic'): ns3::QuicCongestionOps [class]
    module.add_class('QuicCongestionOps', parent=root_module['ns3::TcpNewReno'])
    ## quic-l4-protocol.h (module 'quic'): ns3::QuicL4Protocol [class]
    module.add_class('QuicL4Protocol', parent=root_module['ns3::IpL4Protocol'])
    ## quic-socket.h (module 'quic'): ns3::QuicSocket [class]
    module.add_class('QuicSocket', parent=root_module['ns3::Socket'])
    ## quic-socket.h (module 'quic'): ns3::QuicSocket::QuicSocketTypes_t [enumeration]
    module.add_enum('QuicSocketTypes_t', ['CLIENT', 'SERVER', 'NONE'], outer_class=root_module['ns3::QuicSocket'])
    ## quic-socket.h (module 'quic'): ns3::QuicSocket::QuicStates_t [enumeration]
    module.add_enum('QuicStates_t', ['IDLE', 'LISTENING', 'CONNECTING_SVR', 'CONNECTING_CLT', 'OPEN', 'CLOSING', 'LAST_STATE'], outer_class=root_module['ns3::QuicSocket'])
    typehandlers.add_type_alias(u'ns3::QuicSocket::QuicSocketTypes_t', u'ns3::QuicSocket::QuicSocketTypes_t')
    typehandlers.add_type_alias(u'ns3::QuicSocket::QuicSocketTypes_t*', u'ns3::QuicSocket::QuicSocketTypes_t*')
    typehandlers.add_type_alias(u'ns3::QuicSocket::QuicSocketTypes_t&', u'ns3::QuicSocket::QuicSocketTypes_t&')
    typehandlers.add_type_alias(u'ns3::QuicSocket::QuicStates_t', u'ns3::QuicSocket::QuicStates_t')
    typehandlers.add_type_alias(u'ns3::QuicSocket::QuicStates_t*', u'ns3::QuicSocket::QuicStates_t*')
    typehandlers.add_type_alias(u'ns3::QuicSocket::QuicStates_t&', u'ns3::QuicSocket::QuicStates_t&')
    ## quic-socket-base.h (module 'quic'): ns3::QuicSocketBase [class]
    module.add_class('QuicSocketBase', parent=root_module['ns3::QuicSocket'])
    typehandlers.add_type_alias(u'void ( * ) ( ns3::Ptr< ns3::Packet const > const, ns3::QuicHeader const &, ns3::Ptr< ns3::QuicSocketBase const > const )', u'ns3::QuicSocketBase::QuicTxRxTracedCallback')
    typehandlers.add_type_alias(u'void ( * ) ( ns3::Ptr< ns3::Packet const > const, ns3::QuicHeader const &, ns3::Ptr< ns3::QuicSocketBase const > const )*', u'ns3::QuicSocketBase::QuicTxRxTracedCallback*')
    typehandlers.add_type_alias(u'void ( * ) ( ns3::Ptr< ns3::Packet const > const, ns3::QuicHeader const &, ns3::Ptr< ns3::QuicSocketBase const > const )&', u'ns3::QuicSocketBase::QuicTxRxTracedCallback&')
    ## quic-socket-factory.h (module 'quic'): ns3::QuicSocketFactory [class]
    module.add_class('QuicSocketFactory', parent=root_module['ns3::SocketFactory'])
    ## quic-socket-base.h (module 'quic'): ns3::QuicSocketState [class]
    module.add_class('QuicSocketState', parent=root_module['ns3::TcpSocketState'])
    ## nstime.h (module 'core'): ns3::TimeValue [class]
    module.add_class('TimeValue', import_from_module='ns.core', parent=root_module['ns3::AttributeValue'])
    ## type-id.h (module 'core'): ns3::TypeIdChecker [class]
    module.add_class('TypeIdChecker', import_from_module='ns.core', parent=root_module['ns3::AttributeChecker'])
    ## type-id.h (module 'core'): ns3::TypeIdValue [class]
    module.add_class('TypeIdValue', import_from_module='ns.core', parent=root_module['ns3::AttributeValue'])
    ## uinteger.h (module 'core'): ns3::UintegerValue [class]
    module.add_class('UintegerValue', import_from_module='ns.core', parent=root_module['ns3::AttributeValue'])
    ## address.h (module 'network'): ns3::AddressChecker [class]
    module.add_class('AddressChecker', import_from_module='ns.network', parent=root_module['ns3::AttributeChecker'])
    ## address.h (module 'network'): ns3::AddressValue [class]
    module.add_class('AddressValue', import_from_module='ns.network', parent=root_module['ns3::AttributeValue'])
    ## callback.h (module 'core'): ns3::CallbackImpl<bool, ns3::Ptr<ns3::Socket>, const ns3::Address &, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty> [class]
    module.add_class('CallbackImpl', import_from_module='ns.core', template_parameters=['bool', 'ns3::Ptr<ns3::Socket>', 'const ns3::Address &', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'], parent=root_module['ns3::CallbackImplBase'])
    ## callback.h (module 'core'): ns3::CallbackImpl<ns3::ObjectBase *, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty> [class]
    module.add_class('CallbackImpl', import_from_module='ns.core', template_parameters=['ns3::ObjectBase *', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'], parent=root_module['ns3::CallbackImplBase'])
    ## callback.h (module 'core'): ns3::CallbackImpl<void, const ns3::Ipv4Header &, ns3::Ptr<const ns3::Packet>, ns3::Ipv4L3Protocol::DropReason, ns3::Ptr<ns3::Ipv4>, unsigned int, ns3::empty, ns3::empty, ns3::empty, ns3::empty> [class]
    module.add_class('CallbackImpl', import_from_module='ns.core', template_parameters=['void', 'const ns3::Ipv4Header &', 'ns3::Ptr<const ns3::Packet>', 'ns3::Ipv4L3Protocol::DropReason', 'ns3::Ptr<ns3::Ipv4>', 'unsigned int', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'], parent=root_module['ns3::CallbackImplBase'])
    ## callback.h (module 'core'): ns3::CallbackImpl<void, const ns3::Ipv4Header &, ns3::Ptr<const ns3::Packet>, unsigned int, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty> [class]
    module.add_class('CallbackImpl', import_from_module='ns.core', template_parameters=['void', 'const ns3::Ipv4Header &', 'ns3::Ptr<const ns3::Packet>', 'unsigned int', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'], parent=root_module['ns3::CallbackImplBase'])
    ## callback.h (module 'core'): ns3::CallbackImpl<void, const ns3::Ipv6Header &, ns3::Ptr<const ns3::Packet>, ns3::Ipv6L3Protocol::DropReason, ns3::Ptr<ns3::Ipv6>, unsigned int, ns3::empty, ns3::empty, ns3::empty, ns3::empty> [class]
    module.add_class('CallbackImpl', import_from_module='ns.core', template_parameters=['void', 'const ns3::Ipv6Header &', 'ns3::Ptr<const ns3::Packet>', 'ns3::Ipv6L3Protocol::DropReason', 'ns3::Ptr<ns3::Ipv6>', 'unsigned int', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'], parent=root_module['ns3::CallbackImplBase'])
    ## callback.h (module 'core'): ns3::CallbackImpl<void, const ns3::Ipv6Header &, ns3::Ptr<const ns3::Packet>, unsigned int, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty> [class]
    module.add_class('CallbackImpl', import_from_module='ns.core', template_parameters=['void', 'const ns3::Ipv6Header &', 'ns3::Ptr<const ns3::Packet>', 'unsigned int', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'], parent=root_module['ns3::CallbackImplBase'])
    ## callback.h (module 'core'): ns3::CallbackImpl<void, ns3::Ptr<const ns3::Packet>, const ns3::QuicHeader &, ns3::Ptr<const ns3::QuicSocketBase>, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty> [class]
    module.add_class('CallbackImpl', import_from_module='ns.core', template_parameters=['void', 'ns3::Ptr<const ns3::Packet>', 'const ns3::QuicHeader &', 'ns3::Ptr<const ns3::QuicSocketBase>', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'], parent=root_module['ns3::CallbackImplBase'])
    ## callback.h (module 'core'): ns3::CallbackImpl<void, ns3::Ptr<const ns3::Packet>, const ns3::TcpHeader &, ns3::Ptr<const ns3::TcpSocketBase>, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty> [class]
    module.add_class('CallbackImpl', import_from_module='ns.core', template_parameters=['void', 'ns3::Ptr<const ns3::Packet>', 'const ns3::TcpHeader &', 'ns3::Ptr<const ns3::TcpSocketBase>', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'], parent=root_module['ns3::CallbackImplBase'])
    ## callback.h (module 'core'): ns3::CallbackImpl<void, ns3::Ptr<const ns3::Packet>, ns3::Ptr<ns3::Ipv4>, unsigned int, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty> [class]
    module.add_class('CallbackImpl', import_from_module='ns.core', template_parameters=['void', 'ns3::Ptr<const ns3::Packet>', 'ns3::Ptr<ns3::Ipv4>', 'unsigned int', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'], parent=root_module['ns3::CallbackImplBase'])
    ## callback.h (module 'core'): ns3::CallbackImpl<void, ns3::Ptr<const ns3::Packet>, ns3::Ptr<ns3::Ipv6>, unsigned int, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty> [class]
    module.add_class('CallbackImpl', import_from_module='ns.core', template_parameters=['void', 'ns3::Ptr<const ns3::Packet>', 'ns3::Ptr<ns3::Ipv6>', 'unsigned int', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'], parent=root_module['ns3::CallbackImplBase'])
    ## callback.h (module 'core'): ns3::CallbackImpl<void, ns3::Ptr<ns3::NetDevice>, ns3::Ptr<const ns3::Packet>, unsigned short, const ns3::Address &, const ns3::Address &, ns3::NetDevice::PacketType, ns3::empty, ns3::empty, ns3::empty> [class]
    module.add_class('CallbackImpl', import_from_module='ns.core', template_parameters=['void', 'ns3::Ptr<ns3::NetDevice>', 'ns3::Ptr<const ns3::Packet>', 'unsigned short', 'const ns3::Address &', 'const ns3::Address &', 'ns3::NetDevice::PacketType', 'ns3::empty', 'ns3::empty', 'ns3::empty'], parent=root_module['ns3::CallbackImplBase'])
    ## callback.h (module 'core'): ns3::CallbackImpl<void, ns3::Ptr<ns3::NetDevice>, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty> [class]
    module.add_class('CallbackImpl', import_from_module='ns.core', template_parameters=['void', 'ns3::Ptr<ns3::NetDevice>', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'], parent=root_module['ns3::CallbackImplBase'])
    ## callback.h (module 'core'): ns3::CallbackImpl<void, ns3::Ptr<ns3::Packet>, const ns3::QuicHeader &, ns3::Address &, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty> [class]
    module.add_class('CallbackImpl', import_from_module='ns.core', template_parameters=['void', 'ns3::Ptr<ns3::Packet>', 'const ns3::QuicHeader &', 'ns3::Address &', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'], parent=root_module['ns3::CallbackImplBase'])
    ## callback.h (module 'core'): ns3::CallbackImpl<void, ns3::Ptr<ns3::Packet>, ns3::Ipv4Address, ns3::Ipv4Address, unsigned char, ns3::Ptr<ns3::Ipv4Route>, ns3::empty, ns3::empty, ns3::empty, ns3::empty> [class]
    module.add_class('CallbackImpl', import_from_module='ns.core', template_parameters=['void', 'ns3::Ptr<ns3::Packet>', 'ns3::Ipv4Address', 'ns3::Ipv4Address', 'unsigned char', 'ns3::Ptr<ns3::Ipv4Route>', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'], parent=root_module['ns3::CallbackImplBase'])
    ## callback.h (module 'core'): ns3::CallbackImpl<void, ns3::Ptr<ns3::Packet>, ns3::Ipv6Address, ns3::Ipv6Address, unsigned char, ns3::Ptr<ns3::Ipv6Route>, ns3::empty, ns3::empty, ns3::empty, ns3::empty> [class]
    module.add_class('CallbackImpl', import_from_module='ns.core', template_parameters=['void', 'ns3::Ptr<ns3::Packet>', 'ns3::Ipv6Address', 'ns3::Ipv6Address', 'unsigned char', 'ns3::Ptr<ns3::Ipv6Route>', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'], parent=root_module['ns3::CallbackImplBase'])
    ## callback.h (module 'core'): ns3::CallbackImpl<void, ns3::Ptr<ns3::Socket>, const ns3::Address &, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty> [class]
    module.add_class('CallbackImpl', import_from_module='ns.core', template_parameters=['void', 'ns3::Ptr<ns3::Socket>', 'const ns3::Address &', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'], parent=root_module['ns3::CallbackImplBase'])
    ## callback.h (module 'core'): ns3::CallbackImpl<void, ns3::Ptr<ns3::Socket>, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty> [class]
    module.add_class('CallbackImpl', import_from_module='ns.core', template_parameters=['void', 'ns3::Ptr<ns3::Socket>', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'], parent=root_module['ns3::CallbackImplBase'])
    ## callback.h (module 'core'): ns3::CallbackImpl<void, ns3::Ptr<ns3::Socket>, unsigned int, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty> [class]
    module.add_class('CallbackImpl', import_from_module='ns.core', template_parameters=['void', 'ns3::Ptr<ns3::Socket>', 'unsigned int', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'], parent=root_module['ns3::CallbackImplBase'])
    ## callback.h (module 'core'): ns3::CallbackImpl<void, ns3::QuicSocket::QuicStates_t, ns3::QuicSocket::QuicStates_t, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty> [class]
    module.add_class('CallbackImpl', import_from_module='ns.core', template_parameters=['void', 'ns3::QuicSocket::QuicStates_t', 'ns3::QuicSocket::QuicStates_t', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'], parent=root_module['ns3::CallbackImplBase'])
    ## callback.h (module 'core'): ns3::CallbackImpl<void, ns3::SequenceNumber<unsigned int, int>, ns3::SequenceNumber<unsigned int, int>, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty> [class]
    module.add_class('CallbackImpl', import_from_module='ns.core', template_parameters=['void', 'ns3::SequenceNumber<unsigned int, int>', 'ns3::SequenceNumber<unsigned int, int>', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'], parent=root_module['ns3::CallbackImplBase'])
    ## callback.h (module 'core'): ns3::CallbackImpl<void, ns3::TcpSocket::TcpStates_t, ns3::TcpSocket::TcpStates_t, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty> [class]
    module.add_class('CallbackImpl', import_from_module='ns.core', template_parameters=['void', 'ns3::TcpSocket::TcpStates_t', 'ns3::TcpSocket::TcpStates_t', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'], parent=root_module['ns3::CallbackImplBase'])
    ## callback.h (module 'core'): ns3::CallbackImpl<void, ns3::TcpSocketState::EcnState_t, ns3::TcpSocketState::EcnState_t, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty> [class]
    module.add_class('CallbackImpl', import_from_module='ns.core', template_parameters=['void', 'ns3::TcpSocketState::EcnState_t', 'ns3::TcpSocketState::EcnState_t', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'], parent=root_module['ns3::CallbackImplBase'])
    ## callback.h (module 'core'): ns3::CallbackImpl<void, ns3::TcpSocketState::TcpCongState_t, ns3::TcpSocketState::TcpCongState_t, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty> [class]
    module.add_class('CallbackImpl', import_from_module='ns.core', template_parameters=['void', 'ns3::TcpSocketState::TcpCongState_t', 'ns3::TcpSocketState::TcpCongState_t', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'], parent=root_module['ns3::CallbackImplBase'])
    ## callback.h (module 'core'): ns3::CallbackImpl<void, ns3::Time, ns3::Time, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty> [class]
    module.add_class('CallbackImpl', import_from_module='ns.core', template_parameters=['void', 'ns3::Time', 'ns3::Time', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'], parent=root_module['ns3::CallbackImplBase'])
    ## callback.h (module 'core'): ns3::CallbackImpl<void, unsigned int, unsigned int, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty, ns3::empty> [class]
    module.add_class('CallbackImpl', import_from_module='ns.core', template_parameters=['void', 'unsigned int', 'unsigned int', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'], parent=root_module['ns3::CallbackImplBase'])
    module.add_container('std::vector< unsigned int >', 'unsigned int', container_type=u'vector')
    module.add_container('std::vector< ns3::Ptr< ns3::Packet > >', 'ns3::Ptr< ns3::Packet >', container_type=u'vector')
    module.add_container('std::vector< std::pair< ns3::Ptr< ns3::Packet >, ns3::QuicSubheader > >', 'std::pair< ns3::Ptr< ns3::Packet >, ns3::QuicSubheader >', container_type=u'vector')
    module.add_container('std::vector< ns3::QuicSocketTxItem * >', 'ns3::QuicSocketTxItem *', container_type=u'vector')
