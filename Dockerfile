FROM node:20.1.0-buster-slim AS node-image
FROM python:3.11.3-slim-buster

# Install dependencies
RUN apt-get -qq update; \
    apt-get install -qqy --no-install-recommends \
        gnupg2 wget ca-certificates apt-transport-https \
        autoconf automake cmake dpkg-dev file make patch libc6-dev

# Install clang 17
RUN echo "deb https://apt.llvm.org/buster llvm-toolchain-buster-17 main" \
        > /etc/apt/sources.list.d/llvm.list && \
    wget -qO /etc/apt/trusted.gpg.d/llvm.asc \
        https://apt.llvm.org/llvm-snapshot.gpg.key && \
    apt-get -qq update && \
    apt-get install -qqy clang-17 lld-17 libclang-rt-17-dev-wasm32 && \
    for f in /usr/lib/llvm-17/bin/*; do ln -sf "$f" /usr/bin; done && \
    rm -rf /var/lib/apt/lists/*

ADD ./patches/wasi-libc-clang-17-compat.patch ./patches/emcc-emjs-dylink-externref.patch /

RUN wget https://github.com/WebAssembly/wasi-libc/archive/refs/tags/wasi-sdk-20.tar.gz && \
        tar -xf wasi-sdk-20.tar.gz && rm wasi-sdk-20.tar.gz && \
        patch -p1 -d /wasi-libc-wasi-sdk-20 -i /wasi-libc-clang-17-compat.patch && \
        INSTALL_DIR=/usr make -C /wasi-libc-wasi-sdk-20 install && \
        rm -rf /wasi-libc-wasi-sdk-20

RUN llvm-ranlib /usr/lib/llvm-17/lib/clang/17/lib/wasi/libclang_rt.builtins-wasm32.a

RUN wget https://github.com/emscripten-core/emsdk/archive/refs/tags/3.1.45.tar.gz && \
    tar -xf 3.1.45.tar.gz && rm 3.1.45.tar.gz && \
    /emsdk-3.1.45/emsdk install 3.1.45 && \
    /emsdk-3.1.45/emsdk activate 3.1.45 && \
    patch -p1 -d /emsdk-3.1.45/upstream/emscripten -i /emcc-emjs-dylink-externref.patch


ADD tests/test_requirements.txt /
WORKDIR /
RUN pip3 --no-cache-dir install -r test_requirements.txt

COPY --from=node-image /usr/local/bin/node /usr/local/bin/
COPY --from=node-image /usr/local/lib/node_modules /usr/local/lib/node_modules
RUN ln -s ../lib/node_modules/npm/bin/npm-cli.js /usr/local/bin/npm \
    && ln -s ../lib/node_modules/npm/bin/npx-cli.js /usr/local/bin/npx

CMD ["/bin/sh"]
WORKDIR /src
