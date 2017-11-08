FROM ubuntu:xenial
MAINTAINER Alneos <contact@alneos.fr>
RUN sed -i "s/stretch main/stretch main contrib non-free/" /etc/apt/sources.list && apt update -y -qq && apt install -y -qq g++ valgrind cmake ccache git libmedc1v5 libmedc-dev libhdf5-dev libboost-all-dev code-aster nastran calculix-ccx sudo vim less libmed-tools
ENV VEGA_DIR=/usr/local/src/vegapp
RUN git clone https://github.com/Alneos/vega.git $VEGA_DIR
WORKDIR $VEGA_DIR/build
RUN addgroup --group --gid 1777 vega && useradd --shell /bin/bash --uid 1777 --gid 1777 --create-home --groups sudo vega && echo "vega ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/vega && chown vega:vega $VEGA_DIR/build
RUN sed -i 's/mpi_get_procid_cmd : echo \$OMPI_MCA_orte_ess_vpid/mpi_get_procid_cmd : echo \$OMPI_COMM_WORLD_RANK/g' /etc/codeaster/asrun
USER vega
RUN cmake -DCMAKE_BUILD_TYPE=Debug .. && make -j
RUN ctest .