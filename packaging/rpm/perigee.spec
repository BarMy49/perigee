Name:           perigee
Version:        6.2.0
Release:        %{?perigee_rpm_release}%{!?perigee_rpm_release:0.1}%{?dist}
Summary:        Remote desktop and game streaming client based on Moonlight Qt
License:        GPL-3.0-or-later AND LGPL-2.1-or-later AND MIT AND Zlib AND LicenseRef-ModeSeven-Font
URL:            https://github.com/BarMy49/perigee
Source0:        perigee-%{version}.tar.gz

BuildRequires:  python3
BuildRequires:  gcc-c++
BuildRequires:  make
BuildRequires:  qt6-qtbase-devel
BuildRequires:  qt6-qtdeclarative-devel
BuildRequires:  qt6-qtsvg-devel
BuildRequires:  pkgconfig(openssl)
BuildRequires:  pkgconfig(sdl2)
BuildRequires:  pkgconfig(SDL2_ttf)
BuildRequires:  pkgconfig(opus)
BuildRequires:  pkgconfig(libavcodec)
BuildRequires:  pkgconfig(libavformat)
BuildRequires:  pkgconfig(libavutil)
BuildRequires:  pkgconfig(libswscale)
BuildRequires:  pkgconfig(libva)
BuildRequires:  pkgconfig(libva-x11)
BuildRequires:  pkgconfig(libva-wayland)
BuildRequires:  pkgconfig(libva-drm)
BuildRequires:  pkgconfig(vdpau)
BuildRequires:  pkgconfig(libdrm)
BuildRequires:  pkgconfig(libplacebo)
BuildRequires:  pkgconfig(wayland-client)
BuildRequires:  pkgconfig(x11)
BuildRequires:  desktop-file-utils
BuildRequires:  appstream
Requires:       qt6-qtdeclarative
Requires:       qt6-qtsvg
Requires:       qt6-qtwayland

%description
Perigee is an independent Linux client derived from Moonlight Qt. It connects to
compatible Sunshine and Apollo hosts and keeps its settings and pairing identity
separate from Moonlight. Hardware acceleration depends on host and client drivers.
This development package has not completed the distribution acceptance matrix.

%prep
%autosetup

%build
%set_build_flags
mkdir -p rpm-build
cd rpm-build
qmake6 ../moonlight-qt.pro PREFIX=%{_prefix}
%make_build release

%install
make -C rpm-build/app -f Makefile.Release INSTALL_ROOT=%{buildroot} install
install -Dm644 SOURCE_SNAPSHOT.json %{buildroot}%{_docdir}/perigee/SOURCE_SNAPSHOT.json
install -Dm644 packaging/rpm/THIRD_PARTY.md %{buildroot}%{_docdir}/perigee/THIRD_PARTY.md
for entry in \
    moonlight-common-c/moonlight-common-c/LICENSE.txt \
    moonlight-common-c/moonlight-common-c/enet/LICENSE \
    moonlight-common-c/moonlight-common-c/nanors/LICENSE \
    qmdnsengine/qmdnsengine/LICENSE.txt \
    h264bitstream/LICENSE \
    app/SDL_GameControllerDB/LICENSE; do
    install -Dm644 "$entry" "%{buildroot}%{_licensedir}/perigee/$entry"
done
install -Dm644 packaging/rpm/ModeSeven-README %{buildroot}%{_licensedir}/perigee/ModeSeven-README
install -Dm644 LICENSE %{buildroot}%{_licensedir}/perigee/LICENSE
python3 scripts/check-linux-stage.py --rpm %{buildroot}

%check
desktop-file-validate %{buildroot}%{_datadir}/applications/io.github.BarMy49.Perigee.desktop
appstreamcli validate --no-net --pedantic %{buildroot}%{_datadir}/metainfo/io.github.BarMy49.Perigee.metainfo.xml

%files
%{_bindir}/perigee
%{_datadir}/applications/io.github.BarMy49.Perigee.desktop
%{_datadir}/icons/hicolor/scalable/apps/io.github.BarMy49.Perigee.svg
%{_datadir}/metainfo/io.github.BarMy49.Perigee.metainfo.xml
%doc %{_docdir}/perigee/
%license %{_licensedir}/perigee/

%changelog
* Tue Nov 10 2026 Perigee contributors - 6.2.0-0.1
- Initial local development RPM; distribution acceptance remains pending.
