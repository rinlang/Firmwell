# FIRMWELL main analysis image.
#
# NOTE: this file is written for the legacy Docker builder as well as BuildKit,
# so it deliberately avoids BuildKit-only features (COPY --chmod, RUN --mount).
# Layers are ordered from "changes almost never" to "changes often" so that a
# source-code edit only rebuilds the last few small layers.

FROM quay.io/skopeo/stable:latest AS fetch
RUN mkdir -p /out

RUN skopeo copy --retry-times 3 \
      docker://ghcr.io/qc9c/ubuntu32:latest \
      docker-archive:/out/ubuntu32.tar:ubuntu32:latest \
 && skopeo copy --retry-times 3 \
      docker://ghcr.io/qc9c/multiarch_qemu-user-static_latest:latest \
      docker-archive:/out/multiarch_qemu-user-static_latest.tar:multiarch_qemu-user-static_latest:latest \
 && skopeo copy --retry-times 3 \
      docker://ghcr.io/qc9c/fact_extractor:latest \
      docker-archive:/out/fact_extractor.tar:fkiecad/fact_extractor:latest


FROM capysix/greenhouse-ae:latest
ENV DEBIAN_FRONTEND=noninteractive
WORKDIR /

# ---------------------------------------------------------------------------
# 1) Large, immutable assets first (3 GB of offline image tarballs).
#    Everything below this point is free to change without rewriting them.
# ---------------------------------------------------------------------------
COPY ./docker_img /docker_img
COPY --from=fetch /out/*.tar /docker_img/


# ---------------------------------------------------------------------------
# 2) Static binary assets (~480 MB): QEMU, Chromium driver, Greenhouse files.
# ---------------------------------------------------------------------------
COPY ./firmwell/qemu_system_files /qemu_system_files
COPY ./firmwell/qemu_user /qemu_user
RUN chmod +x /qemu_user/* && cp -a /qemu_user/. /usr/bin/

COPY ./firmwell/analysis /analysis
COPY ./firmwell/eval_gh /fw/firmwell/eval_gh
COPY ./firmwell/greenhouse_files /fw/firmwell/greenhouse_files
COPY ./firmwell/tools/scripts /fw/firmwell/tools/scripts
COPY ./firmwell/tools/templates /fw/firmwell/tools/templates
COPY ./firmwell/tools/get_nvram /fw/firmwell/tools/get_nvram

# Restore the executable bits the previous single "chmod -R +x /fw" provided,
# but do it here - below the application code - so code edits never repeat it.
RUN chmod -R +x /fw /analysis


# ---------------------------------------------------------------------------
# 3) Optional fuzz payload. Kept below the code so that a code edit does not
#    re-copy it; it is removed again in step 6.
# ---------------------------------------------------------------------------
COPY ./firmwell/sys_fuzz /tmp/sys_fuzz


# ---------------------------------------------------------------------------
# 4) Dependencies. One apt layer (was 6), openjdk folded in, apt lists removed.
# ---------------------------------------------------------------------------
RUN apt-get update && \
    apt-get install -y apt-utils git curl python3 python3-pip wget sudo net-tools iputils-ping iptables iproute2 build-essential && \
    apt-get install -y p7zip-full p7zip-rar zip libpq-dev vim dnsutils jq unrar ca-certificates && \
    apt-get install -y qemu-system-arm qemu-system-mips qemu-system-x86 qemu-utils && \
    apt-get install -y libssl-dev libffi-dev libxml2 libxml2-dev libxslt1-dev zlib1g-dev gcc cmake && \
    apt-get install -y openssh-server zsh rsync csvtool software-properties-common && \
    apt-get install -y openjdk-21-jdk && \
    rm -rf /var/lib/apt/lists/*

# Download, unpack and delete the Ghidra archive in one layer (the 425 MB zip
# used to be kept in an intermediate layer). curl honours the daemon proxy.
RUN curl -fsSL --retry 3 --retry-delay 5 --connect-timeout 20 -o /tmp/ghidra.zip \
        https://github.com/NationalSecurityAgency/ghidra/releases/download/Ghidra_11.2_build/ghidra_11.2_PUBLIC_20240926.zip && \
    unzip -q /tmp/ghidra.zip -d / && \
    rm -f /tmp/ghidra.zip

RUN python -m pip install --no-cache-dir jinja2 pyyaml && \
    . /root/venv/bin/activate && \
    python3 -m pip install --no-cache-dir docker==5.0.0 requests==2.24.0 selenium==3.141.0 lxml==4.7.1 networkx==2.8.4 ifaddr==0.2.0 python-magic six redis pexpect paramiko pysnmp future pyelftools pyparsing colorama psycopg2 sonyflake-py pyyaml minio pymongo jinja2 && \
    cd /work/FirmAE/binwalk-2.3.3 && /root/venv/bin/python3 setup.py install


# ---------------------------------------------------------------------------
# 5) Application code: the only layers that change on a normal code edit.
# ---------------------------------------------------------------------------
COPY ./firmwell/plugins /fw/firmwell/plugins
COPY ./firmwell/backend /fw/firmwell/backend
COPY ./firmwell/__init__.py /fw/firmwell/__init__.py
COPY ./firmwell.py /fw/firmwell.py
COPY ./entrypoint.sh /fw/entrypoint.sh
COPY ./docker_init.sh /fw/docker_init.sh
COPY ./docker_k8_extract.sh /fw/docker_k8_extract.sh
COPY ./docker_k8_run.sh /fw/docker_k8_run.sh
COPY ./run.sh /fw/run.sh
RUN chmod +x /fw/*.sh /fw/firmwell.py /fw/firmwell/__init__.py && \
    chmod -R +x /fw/firmwell/backend /fw/firmwell/plugins


# ---------------------------------------------------------------------------
# 6) Fuzz overlay: no-op unless --build-arg SYS_FUZZ=True. Runs after the code
#    so the fuzz QemuSysRunner.py still wins, exactly as before.
# ---------------------------------------------------------------------------
ARG SYS_FUZZ=False
RUN if [ "$SYS_FUZZ" = "True" ]; then \
      cp -r /tmp/sys_fuzz/fuzz /qemu_system_files/ && \
      cp /tmp/sys_fuzz/QemuSysRunner.py /fw/firmwell/backend/QemuSysRunner.py; \
    fi; \
    rm -rf /tmp/sys_fuzz
