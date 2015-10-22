# StrusAnalyzer spec file

# Set distribution based on some OpenSuse and distribution macros
# this is only relevant when building on https://build.opensuse.org
###

%define rhel 0
%define rhel5 0
%define rhel6 0
%define rhel7 0
%if 0%{?rhel_version} >= 500 && 0%{?rhel_version} <= 599
%define dist rhel5
%define rhel 1
%define rhel5 1
%endif
%if 0%{?rhel_version} >= 600 && 0%{?rhel_version} <= 699
%define dist rhel6
%define rhel 1
%define rhel6 1
%endif
%if 0%{?rhel_version} >= 700 && 0%{?rhel_version} <= 799
%define dist rhel7
%define rhel 1
%define rhel7 1
%endif

%define centos 0
%define centos5 0
%define centos6 0
%define centos7 0
%if 0%{?centos_version} >= 500 && 0%{?centos_version} <= 599
%define dist centos5
%define centos 1
%define centos5 1
%endif
%if 0%{?centos_version} >= 600 && 0%{?centos_version} <= 699
%define dist centos6
%define centos 1
%define centos6 1
%endif
%if 0%{?centos_version} >= 700 && 0%{?centos_version} <= 799
%define dist centos7
%define centos 1
%define centos7 1
%endif

%define scilin 0
%define scilin5 0
%define scilin6 0
%define scilin7 0
%if 0%{?scilin_version} >= 500 && 0%{?scilin_version} <= 599
%define dist scilin5
%define scilin 1
%define scilin5 1
%endif
%if 0%{?scilin_version} >= 600 && 0%{?scilin_version} <= 699
%define dist scilin6
%define scilin 1
%define scilin6 1
%endif
%if 0%{?scilin_version} >= 700 && 0%{?scilin_version} <= 799
%define dist scilin7
%define scilin 1
%define scilin7 1
%endif

%define fedora 0
%define fc21 0
%define fc22 0
%if 0%{?fedora_version} == 21
%define dist fc21
%define fc21 1
%define fedora 1
%endif
%if 0%{?fedora_version} == 22
%define dist fc22
%define fc22 1
%define fedora 1
%endif

%define suse 0
%define osu131 0
%define osu132 0
%define osufactory 0
%if 0%{?suse_version} == 1310
%define dist osu131
%define osu131 1
%define suse 1
%endif
%if 0%{?suse_version} == 1320
%define dist osu132
%define osu132 1
%define suse 1
%endif
%if 0%{?suse_version} > 1320
%define dist osufactory
%define osufactory 1
%define suse 1
%endif

%define sles 0
%define sles11 0
%define sles12 0
%if 0%{?suse_version} == 1110
%define dist sle11
%define sles11 1
%define sles 1
%endif
%if 0%{?suse_version} == 1315 
%define dist sle12
%define sles12 1
%define sles 1
%endif

Summary: Library implementing the document and query analysis for a text search engine
Name: strusanalyzer
%define main_version 0.1.6
Version: %{main_version}
Release: 0.1
License: GPLv3
Group: Development/Libraries/C++

Source: %{name}_%{main_version}.tar.gz

URL: http://project-strus.net

BuildRoot: %{_tmppath}/%{name}-root

# Build dependencies
###

# OBS doesn't install the minimal set of build tools automatically
BuildRequires: gcc
BuildRequires: gcc-c++
BuildRequires: cmake

# LinuxDistribution.cmake depends depends on the Linux release files in '/etc' or
# LSB files
%if %{rhel}
BuildRequires: redhat-release
%endif
%if %{centos}
BuildRequires: centos-release
%endif
%if %{scilin}
BuildRequires: sl-release
%endif
%if %{fedora} && !0%{?opensuse_bs}
BuildRequires: fedora-release
%endif
%if %{fedora} && 0%{?opensuse_bs}
BuildRequires: generic-release
%endif
%if %{suse}
BuildRequires: openSUSE-release
%endif
%if %{sles}
%if %{sles12}
#exists in sles12, missing on OBS!
#BuildRequires: sles-release
%else
BuildRequires: sles-release
%endif
%endif

%if %{rhel} || %{centos} || %{scilin} || %{fedora}
%if %{rhel5} || %{rhel6} || %{centos5} || %{centos6} || %{scilin5} || %{scilin6}
Requires: boost153 >= 1.53.0
BuildRequires: boost153-devel >= 1.53.0
%else
Requires: boost >= 1.53.0
Requires: boost-thread >= 1.53.0
Requires: boost-system >= 1.53.0
Requires: boost-date-time >= 1.53.0
BuildRequires: boost-devel
%endif
%endif

%if %{suse} || %{sles}
%if %{sles11}
Requires: boost153 >= 1.53.0
BuildRequires: boost153-devel >= 1.53.0
%endif
%if %{osu131}
Requires: libboost_thread1_53_0 >= 1.53.0
Requires: libboost_atomic1_53_0 >= 1.53.0
Requires: libboost_system1_53_0 >= 1.53.0
Requires: libboost_date_time1_53_0 >= 1.53.0
BuildRequires: boost-devel
# for some reason OBS doesn't pull in libboost_atomic1_53_0 automatically!?
BuildRequires: libboost_atomic1_53_0 >= 1.53.0
%endif
%if %{osu132} || %{sles12}
Requires: libboost_thread1_54_0 >= 1.54.0
Requires: libboost_atomic1_54_0 >= 1.54.0
Requires: libboost_system1_54_0 >= 1.54.0
Requires: libboost_date_time1_54_0 >= 1.54.0
BuildRequires: boost-devel
%endif
%if %{osufactory}
Requires: libboost_thread1_58_0 >= 1.58.0
Requires: libboost_atomic1_58_0 >= 1.58.0
Requires: libboost_system1_58_0 >= 1.58.0
Requires: libboost_date_time1_58_0 >= 1.58.0
BuildRequires: boost-devel
%endif
%endif

BuildRequires: strus-devel >= %{main_version}
Requires: strus >= %{main_version}

# Check if 'Distribution' is really set by OBS (as mentioned in bacula)
%if ! 0%{?opensuse_bs}
Distribution: %{dist}
%endif

Packager: Patrick Frey <patrickpfrey@yahoo.com>

%description
Library implementing the document and query analysis for a text search engine.

%package devel
Summary: strusanalyzer development files
Group: Development/Libraries/C++

%description devel
The libraries and header files used for development with strusanalyzer.

Requires: %{name} >= %{main_version}-%{release}

%prep
%setup -n %{name}-%{main_version}

%build

mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release -DLIB_INSTALL_DIR=%{_lib} ..
make %{?_smp_mflags}

%install

cd build
make DESTDIR=$RPM_BUILD_ROOT install

# TODO: avoid building this stuff in cmake. how?
rm -rf $RPM_BUILD_ROOT%{_libdir}/debug
rm -rf $RPM_BUILD_ROOT%{_prefix}/src/debug

%clean
rm -rf $RPM_BUILD_ROOT

%check
cd build
make test

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr( -, root, root )
%dir %{_libdir}/strus
%{_libdir}/strus/libstrus_stemmer.so.0.1
%{_libdir}/strus/libstrus_stemmer.so.0.1.6
%{_libdir}/strus/libstrus_tokenizer_word.so.0.1
%{_libdir}/strus/libstrus_tokenizer_word.so.0.1.6
%{_libdir}/strus/libstrus_tokenizer_punctuation.so.0.1
%{_libdir}/strus/libstrus_tokenizer_punctuation.so.0.1.6
%{_libdir}/strus/libstrus_segmenter_textwolf.so.0.1
%{_libdir}/strus/libstrus_segmenter_textwolf.so.0.1.6
%{_libdir}/strus/libstrus_analyzer.so.0.1
%{_libdir}/strus/libstrus_analyzer.so.0.1.6
%{_libdir}/strus/libstrus_normalizer_dictmap.so.0.1
%{_libdir}/strus/libstrus_normalizer_dictmap.so.0.1.6
%{_libdir}/strus/libstrus_normalizer_charconv.so.0.1
%{_libdir}/strus/libstrus_normalizer_charconv.so.0.1.6
%{_libdir}/strus/libstrus_textproc.so.0.1
%{_libdir}/strus/libstrus_textproc.so.0.1.6
%{_libdir}/strus/libstrus_normalizer_snowball.so.0.1
%{_libdir}/strus/libstrus_normalizer_snowball.so.0.1.6
%{_libdir}/strus/libstrus_detector_std.so.0.1
%{_libdir}/strus/libstrus_detector_std.so.0.1.6
%{_libdir}/strus/libstrus_normalizer_dateconv.so.0.1
%{_libdir}/strus/libstrus_normalizer_dateconv.so.0.1.6

%files devel
%defattr( -, root, root )
%{_libdir}/strus/libstrus_stemmer.so
%{_libdir}/strus/libstrus_tokenizer_word.so
%{_libdir}/strus/libstrus_tokenizer_punctuation.so
%{_libdir}/strus/libstrus_segmenter_textwolf.so
%{_libdir}/strus/libstrus_analyzer.so
%{_libdir}/strus/libstrus_normalizer_dictmap.so
%{_libdir}/strus/libstrus_normalizer_charconv.so
%{_libdir}/strus/libstrus_textproc.so
%{_libdir}/strus/libstrus_normalizer_snowball.so
%{_libdir}/strus/libstrus_detector_std.so
%{_libdir}/strus/libstrus_normalizer_dateconv.so
%dir %{_includedir}/strus
%{_includedir}/strus/*.hpp
%dir %{_includedir}/strus/lib
%{_includedir}/strus/lib/*.hpp
%dir %{_includedir}/strus/analyzer
%{_includedir}/strus/analyzer/*.hpp

%changelog
* Fri Mar 20 2015 Patrick Frey <patrickpfrey@yahoo.com> 0.1.6-0.1
- preliminary release
