Name:		beirdobot
Version:	0.6.3
Release:	0%{?dist}
Summary:	BeirdoBot

Group:		daemons
License:	GPL
URL:		https://github.com/MythTV/beirdobot
Source0:	beirdobot-%{version}.tar.gz

BuildRequires:  gcc, gcc-c++
BuildRequires:  automake, cmake
BuildRequires:  curl-devel
BuildRequires:  pcre-devel
BuildRequires:  mariadb-devel
BuildRequires:  mysql-libs
BuildRequires:  ncurses-devel
BuildRequires:  glibc-devel

Requires(post): systemd
Requires(preun): systemd
Requires(postun): systemd
BuildRequires:  systemd

#Patch0: clucene-narrowing-conversions.patch

%description
BeirdoBot IRC logger

%prep
%setup -q
#pushd bot/src/clucene
#%patch0 -p0
#popd

%build
aclocal
autoconf
automake --add-missing
%configure --disable-plugin-luascript  --disable-plugin-mailbox --disable-plugin-perl --disable-plugin-rssfeed --disable-plugin-trac --disable-plugin-url
cd bot
make %{?_smp_mflags}

%install
make install DESTDIR=%{buildroot}
mkdir -p %{buildroot}/usr/share/beirdobot/web/data
mkdir -p %{buildroot}/%{_unitdir}
cp bot/beirdobot.service %{buildroot}/%{_unitdir}

%post
%systemd_post beirdobot.service

%preun
%systemd_preun beirdobot.service

%postun
%systemd_postun_with_restart beirdobot.service

%files
%defattr(755,root,apache)
/usr/bin/beirdobot
/usr/bin/beirdobot-webserviced
%attr(0770,root,apache) /usr/share/beirdobot/clucene
/usr/share/beirdobot/plugins
/usr/share/beirdobot/web/.htaccess
/usr/share/beirdobot/web/beirdobot.php
%attr(0770,root,apache) /usr/share/beirdobot/web/data
/usr/share/beirdobot/web/includes
/usr/share/beirdobot/web/js
/usr/share/beirdobot/web/modules
/usr/share/beirdobot/web/skins
/usr/share/beirdobot/web/templates
/usr/share/beirdobot/weblogs
%config %{_unitdir}/beirdobot.service


%changelog
* Mon May 21 2018 Stuart Auchterlonie <stuarta@mythtv.org>
- Rebuild for f28
- Apply patches to support gcc7 and updated glibc

* Thu Jun 11 2015 Stuart Auchterlonie <stuarta@mythtv.org>
- Fix default permissions on directories to run as non root user

* Wed Jun 10 2015 Stuart Auchterlonie <stuarta@mythtv.org>
- Fixes for f22 compiler
- Adding systemd unit script
- Fix crash when running under systemd

* Mon May 11 2015 Stuart Auchterlonie <stuarta@mythtv.org>
- Rewritten spec file, now builds locally and in mock
