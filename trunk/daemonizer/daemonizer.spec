%define version 0.9.3
%define release 0
%define name    daemonizer

Summary:   daemonizer is program to start a process and detach it from the terminal
Name:      %{name}
Version:   %{version}
Release:   %{release}
License:   Apache
Packager:  Matteo Corti <matteo.corti@gmail.com>
Group:     Applications/System
BuildRoot: %{_tmppath}/%{name}-%{version}-root
Source:    %{name}-%{version}.tar.gz

%description
daemonizer is program to start a process and detach it from the terminal

%prep
%setup

%build
%configure %{_prefix}/usr
%{__make} %{?_smp_mflags}

%install
%{__rm} -rf %{buildroot}
%makeinstall

%clean
%{__rm} -rf %{buildroot}

%files
%defattr(-, root, root, 0644)
%{_mandir}/man1/daemonizer.1.gz
%doc AUTHORS ChangeLog NEWS README INSTALL TODO COPYING VERSION  COPYRIGHT LICENSE
%attr(0755, root, root) %{_prefix}/bin/daemonizer

%changelog
* Thu Apr 17 2008 Matteo Corti <matteo.corti@id.ethz.ch> - 0.9.0
- Initial package

