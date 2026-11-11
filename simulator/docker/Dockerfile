# -------------------------------------------------------------------------------
# シミュレータビルド用ステージ
# -------------------------------------------------------------------------------
ARG BASE_IMAGE=ubuntu
ARG BASE_TAG=22.04

FROM ${BASE_IMAGE}:${BASE_TAG} as base

ARG ADDITIONAL_APT_GET_OPTS=""
ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get upgrade -y\
    && apt-get dist-upgrade -y\
    && apt-get autoremove \
    && apt-get autoclean \
    && apt-get install -y $ADDITIONAL_APT_GET_OPTS \
      curl \
      wget \
      build-essential \
      cmake \
      gedit \
      git \
      nano \
      vim \
      python3.10-dev \
      python3.10-venv \
      python3-pip \
      python3-pip-whl \
      freeglut3-dev \
      graphviz \
      pdf2svg \
      texlive-lang-japanese \
      texlive-latex-extra \
      texlive-luatex \
      evince \
    && apt-get clean \
    && rm -rf /var/cache/apt/archives/* \
    && rm -rf /var/lib/apt/lists/*

RUN curl https://github.com/doxygen/doxygen/releases/download/Release_1_14_0/doxygen-1.14.0.linux.bin.tar.gz -o doxygen-1.14.0.linux.bin.tar.gz -L
RUN tar -xf doxygen-1.14.0.linux.bin.tar.gz \
    && cd doxygen-1.14.0 \
    && make \
    && make install

# -------------------------------------------------------------------------------
# シミュレータビルド用ステージ
# -------------------------------------------------------------------------------
FROM base as build

ARG ASRC_BUILD_OPTIONS

# 仮想環境を作成
RUN mkdir /opt/data \
    && cd /opt/data \
    && python3 -m venv venv \
    && touch /etc/profile \
    && sed -i '$asource /opt/data/venv/bin/activate' /etc/profile

COPY  . /opt/data
RUN bash -c '\
  mapfile -t ARGS <<< "${ASRC_BUILD_OPTIONS}" && \
  bash /opt/data/docker/build_sim.sh "${ARGS[@]}" -d /opt/data/dist \
'

# -------------------------------------------------------------------------------
# 学習・評価用ステージ
# -------------------------------------------------------------------------------
FROM base as work

ARG GROUP_ID=1000
ARG USER_ID=1000
ARG REMOTE_GROUP=developer
ARG REMOTE_USER=developer

# ユーザを作成
RUN groupadd -g $GROUP_ID $REMOTE_GROUP && \
useradd -d /home/${REMOTE_USER} -m -s /bin/bash -u $USER_ID -g $GROUP_ID $REMOTE_USER
USER $REMOTE_USER:$REMOTE_GROUP

WORKDIR /workspace
