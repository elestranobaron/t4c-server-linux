# Multi-stage: compile T4CServer, run with staged runtime assets (build/ layout).
# Base Arch pour coller a l'environnement de dev (SDL3, ODBC MariaDB).

FROM archlinux:latest AS builder

RUN pacman -Sy --noconfirm \
    base-devel \
    cmake \
    sdl3 \
    openssl \
    unixodbc \
    mariadb-libs \
  && pacman -Scc --noconfirm

WORKDIR /src
COPY . .

RUN cmake -S . -B build -DCMAKE_CXX_STANDARD=17 -DCMAKE_BUILD_TYPE=Release \
  && cmake --build build -j"$(nproc)"

# --- runtime ---
FROM archlinux:latest AS runtime

RUN pacman -Sy --noconfirm \
    sdl3 \
    openssl \
    unixodbc \
    mariadb-connector-odbc \
    gettext \
  && pacman -Scc --noconfirm

WORKDIR /srv/t4c

COPY --from=builder /src/build/T4CServer ./T4CServer
COPY docker/runtime/ /opt/t4c-assets/
COPY docker/odbc/ /opt/t4c-odbc/
COPY docker/entrypoint.sh /usr/local/bin/t4c-entrypoint.sh

RUN chmod +x /usr/local/bin/t4c-entrypoint.sh ./T4CServer \
  && mkdir -p logs WDA

ENV T4C_APP_DIR=/srv/t4c \
    T4C_ASSETS_DIR=/opt/t4c-assets \
    T4C_VERIFY_WDA_MD5=1

EXPOSE 11677/udp

ENTRYPOINT ["/usr/local/bin/t4c-entrypoint.sh"]
