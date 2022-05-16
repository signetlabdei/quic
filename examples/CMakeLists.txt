build_lib_example(
  NAME quic-variants-comparison-bulksend
  SOURCE_FILES quic-variants-comparison-bulksend.cc
  LIBRARIES_TO_LINK
    ${libcore}
    ${libquic}
    ${libinternet}
    ${libapplications}
    ${libflow-monitor}
    ${libpoint-to-point}
)