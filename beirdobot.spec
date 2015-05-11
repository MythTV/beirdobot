Name:		beirdobot
Version:	0.5
Release:	1%{?dist}
Summary:	BeirdoBot

Group:		daemons
License:	GPL
URL:		https://github.com/MythTV/beirdobot
Source0:	beirdobot-%{version}.tar.gz

BuildRequires:  automake, cmake
BuildRequires:  curl-devel
BuildRequires:  pcre-devel
BuildRequires:  mysql-devel
BuildRequires:  mysql-libs
BuildRequires:  lua-devel
BuildRequires:  ncurses-devel

Requires: lua

%description
BeirdoBot

%prep
%setup -q

%build
aclocal
autoconf
%configure --disable-plugin-luascript  --disable-plugin-mailbox --disable-plugin-perl --disable-plugin-rssfeed --disable-plugin-trac --disable-plugin-url
cd bot
make %{?_smp_mflags}

%install
make install DESTDIR=%{buildroot}

%files
/usr/bin/beirdobot
/usr/bin/beirdobot-webserviced
/usr/share/beirdobot/*

%changelog
* Mon 11 May 2015 Stuart Auchterlonie <stuarta@mythtv.org>
- Rewritten spec file, now builds locally and in mock
