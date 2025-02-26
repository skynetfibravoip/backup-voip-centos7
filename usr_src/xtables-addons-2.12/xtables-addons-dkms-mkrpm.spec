%{?!module_name: %{error: You did not specify a module name (%%module_name)}}
%{?!version: %{error: You did not specify a module version (%%version)}}
%{?!kernel_versions: %{error: You did not specify kernel versions (%%kernel_version)}}
%{?!packager: %define packager DKMS <dkms-devel@lists.us.dell.com>}
%{?!license: %define license Unknown}
%{?!_dkmsdir: %define _dkmsdir /var/lib/dkms}
%{?!_srcdir: %define _srcdir %_prefix/src}
%{?!_datarootdir: %define _datarootdir %{_datadir}}

Summary:	%{module_name} %{version} dkms package
Name:		%{module_name}
Version:	%{version}
License:	%license
Release:	3dkms
BuildArch:	x86_64
Group:		System/Kernel
Requires: 	dkms >= 1.95
Requires:       issabel-geoip_key
Autoreq:        0 
BuildRequires: 	dkms
BuildRoot: 	%{_tmppath}/%{name}-%{version}-%{release}-root/

%description
Kernel modules for %{module_name} %{version} in a DKMS wrapper.

%prep
if [ "%mktarball_line" != "none" ]; then
        /usr/sbin/dkms mktarball -m %module_name -v %version %mktarball_line --archive `basename %{module_name}-%{version}.dkms.tar.gz`
        cp -af %{_dkmsdir}/%{module_name}/%{version}/tarball/`basename %{module_name}-%{version}.dkms.tar.gz` %{module_name}-%{version}.dkms.tar.gz
fi
mkdir -p %{buildroot}/%{_sysconfdir}

%build
mkdir -p $RPM_BUILD_ROOT/%{_srcdir}
cp -Lpr %{_sourcedir}/%{module_name}-%{version} $RPM_BUILD_ROOT/%{_srcdir}
cd $RPM_BUILD_ROOT/%{_srcdir}/%{module_name}-%{version}/extensions
make

%install
if [ "$RPM_BUILD_ROOT" != "/" ]; then
        rm -rf $RPM_BUILD_ROOT
fi
mkdir -p $RPM_BUILD_ROOT/%{_srcdir}
mkdir -p $RPM_BUILD_ROOT/%{_datarootdir}/%{module_name}
mkdir -p $RPM_BUILD_ROOT/%{_sysconfdir}/cron.daily
mkdir -p $RPM_BUILD_ROOT/usr/share

if [ -d %{_sourcedir}/%{module_name}-%{version} ]; then
        cp -Lpr %{_sourcedir}/%{module_name}-%{version} $RPM_BUILD_ROOT/%{_srcdir}
fi

if [ -f %{module_name}-%{version}.dkms.tar.gz ]; then
        install -m 644 %{module_name}-%{version}.dkms.tar.gz $RPM_BUILD_ROOT/%{_datarootdir}/%{module_name}
fi

if [ -f %{_sourcedir}/common.postinst ]; then
        install -m 755 %{_sourcedir}/common.postinst $RPM_BUILD_ROOT/%{_datarootdir}/%{module_name}/postinst
fi

install -m 755 %{_dkmsdir}/%{module_name}/%{version}/source/issabel/geoip_update.sh $RPM_BUILD_ROOT/%{_sysconfdir}/cron.daily/geoip_update.sh 
cp -af %{_dkmsdir}/%{module_name}/%{version}/source/issabel/geoip $RPM_BUILD_ROOT/usr/share
mkdir -p $RPM_BUILD_ROOT/usr/lib64/xtables
install -m 755 $RPM_BUILD_ROOT/%{_srcdir}/%{module_name}-%{version}/extensions/libxt*so $RPM_BUILD_ROOT/usr/lib64/xtables
mkdir -p $RPM_BUILD_ROOT/usr/local/lib
cp -af $RPM_BUILD_ROOT/%{_srcdir}/%{module_name}-%{version}/extensions/ACCOUNT/libxt_ACCOUNT_cl.la $RPM_BUILD_ROOT/usr/local/lib
cp -af $RPM_BUILD_ROOT/%{_srcdir}/%{module_name}-%{version}/extensions/ACCOUNT/.libs/*so* $RPM_BUILD_ROOT/usr/local/lib

%clean
if [ "$RPM_BUILD_ROOT" != "/" ]; then
        rm -rf $RPM_BUILD_ROOT
fi

%post
for POSTINST in %{_prefix}/lib/dkms/common.postinst %{_datarootdir}/%{module_name}/postinst; do
        if [ -f $POSTINST ]; then
                $POSTINST %{module_name} %{version} %{_datarootdir}/%{module_name}
                exit $?
        fi
        echo "WARNING: $POSTINST does not exist."
done
echo -e "ERROR: DKMS version is too old and %{module_name} was not"
echo -e "built with legacy DKMS support."
echo -e "You must either rebuild %{module_name} with legacy postinst"
echo -e "support or upgrade DKMS to a more current version."
exit 1

%preun
echo -e
echo -e "Uninstall of %{module_name} module (version %{version}) beginning:"
dkms remove -m %{module_name} -v %{version} --all --rpm_safe_upgrade
exit 0

%files
%defattr(-,root,root)
%{_srcdir}
%{_datarootdir}/%{module_name}/
%{_sysconfdir}/cron.daily/geoip_update.sh
/usr/share/geoip
/usr/local/lib/libxt_ACCOUNT_cl.la
/usr/local/lib/libxt_ACCOUNT_cl.so
/usr/local/lib/libxt_ACCOUNT_cl.so.0
/usr/local/lib/libxt_ACCOUNT_cl.so.0.0.0
/usr/lib64/xtables/libxt_CHAOS.so
/usr/lib64/xtables/libxt_DELUDE.so
/usr/lib64/xtables/libxt_DHCPMAC.so
/usr/lib64/xtables/libxt_DNETMAP.so
/usr/lib64/xtables/libxt_ECHO.so
/usr/lib64/xtables/libxt_IPMARK.so
/usr/lib64/xtables/libxt_LOGMARK.so
/usr/lib64/xtables/libxt_SYSRQ.so
/usr/lib64/xtables/libxt_condition.so
/usr/lib64/xtables/libxt_dhcpmac.so
/usr/lib64/xtables/libxt_fuzzy.so
/usr/lib64/xtables/libxt_geoip.so
/usr/lib64/xtables/libxt_gradm.so
/usr/lib64/xtables/libxt_iface.so
/usr/lib64/xtables/libxt_ipp2p.so
/usr/lib64/xtables/libxt_ipv4options.so
/usr/lib64/xtables/libxt_length2.so
/usr/lib64/xtables/libxt_lscan.so
/usr/lib64/xtables/libxt_psd.so
/usr/lib64/xtables/libxt_quota2.so

%changelog
* %(date "+%a %b %d %Y") %packager %{version}-%{release}
- Automatic build by DKMS

