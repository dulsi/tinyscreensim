Summary: Simulator to run Tinyscreen/Tinyduino programs
Name: tinyscreensim
Version: 0.1.0
Release: 1%{?dist}
License: MIT
Url: http://www.identicalsoftware.com/tinyscreensim
Source: http://www.identicalsoftware.com/tinyscreensim/%{name}-%{version}.tgz
BuildArch:      noarch
Requires:       glfw-devel

%description
Tinyscreensim allows you to compile Tinyscreen/Tinyduino programs to run on
the computer.

%prep
%setup -q

%build

%install
install -d %{buildroot}%{_bindir}
cp tinyscreensim %{buildroot}%{_bindir}
install -d %{buildroot}%{_datadir}/tinyscreensim
cp -r include src %{buildroot}%{_datadir}/tinyscreensim

%check

%post

%postun

%posttrans

%files
%doc README.md
%license LICENSE
%{_bindir}/*
%{_datadir}/tinyscreensim

%changelog
* Tue Sep 05 2017 Dennis Payne <dulsi@identicalsoftware.com> - 0.1.0-1
- New version of tinyscreensim released.
