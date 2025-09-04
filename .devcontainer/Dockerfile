# Base Ubuntu con utente "vscode" già configurato
FROM mcr.microsoft.com/devcontainers/base:ubuntu-22.04


ARG DEBIAN_FRONTEND=noninteractive


RUN apt-get update \
&& apt-get install -y --no-install-recommends \
build-essential \
cmake \
ninja-build \
pkg-config \
git \
curl \
ca-certificates \
zip unzip \
# Toolchain MinGW-w64 64 bit
mingw-w64 \
mingw-w64-tools \
binutils-mingw-w64 \
gcc-mingw-w64 \
g++-mingw-w64 \
# (opzionale) test degli .exe con Wine
wine64 \
imagemagick \
librsvg2-bin \
&& rm -rf /var/lib/apt/lists/*


# Seleziona il runtime POSIX per MinGW (threading più moderno)
# Questi "update-alternatives" possono non essere presenti su alcune distro: usa "|| true" per non fallire.
RUN update-alternatives --set x86_64-w64-mingw32-gcc /usr/bin/x86_64-w64-mingw32-gcc-posix || true \
&& update-alternatives --set x86_64-w64-mingw32-g++ /usr/bin/x86_64-w64-mingw32-g++-posix || true


# (Opzionale ma utile) vcpkg per gestire dipendenze native MinGW
RUN git clone --depth 1 https://github.com/microsoft/vcpkg.git /opt/vcpkg \
&& /opt/vcpkg/bootstrap-vcpkg.sh -disableMetrics
ENV VCPKG_ROOT=/opt/vcpkg
ENV PATH=$VCPKG_ROOT:$PATH


# Imposta una directory di lavoro di default (VS Code la sovrascrive con /workspaces/...)
WORKDIR /workspaces