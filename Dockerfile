FROM ubuntu:xenial
MAINTAINER Alneos <contact@alneos.fr>
RUN apt update -y -qq && apt install -y -qq g++ valgrind cmake ccache git libmedc1v5 libmedc-dev libhdf5-dev libboost-all-dev nastran calculix-ccx sudo vim less wget curl libmed-tools strace

#RUN curl -SL https://www.googleapis.com/drive/v3/files/0B9OX-2kDhO9SV0pXWVJSQ1RfclU?alt=media | tar -xzC /opt
#COPY ./dockerdata/aster_smeca_2017.tgz /root
#ENV ASTER_INSTALL=/opt/aster_smeca_2017
#RUN tar xzf /root/aster_smeca_2017.tgz -C /opt && rm /root/aster_smeca_2017.tgz && echo "vers : STABLE:/opt/aster/13.4/share/aster" >> $ASTER_INSTALL/etc/codeaster/aster

#RUN apt install -y gettext gfortran g++ python-dev zlib1g-dev bison flex checkinstall cmake hdf5-tools swig xterm grace libopenblas-dev python-numpy
#RUN curl -SL https://www.code-aster.org/FICHIERS/aster-full-src-13.4.0-3.noarch.tar.gz | tar -xzC /root
#COPY ./dockerdata/aster-full-src-13.4.0-3.noarch.tar.gz /root
#WORKDIR /root
#RUN tar -xzf /root/aster-full-src-13.4.0-3.noarch.tar.gz && rm /root/aster-full-src-13.4.0-3.noarch.tar.gz
#ENV ASTER_BUILD=/root/aster-full-src-13.4.0
#ENV ASTER_INSTALL=/opt/aster
#WORKDIR $ASTER_BUILD
#COPY ./dockerdata/util.py $ASTER_BUILD/SRC/configuration/util.py
#RUN gzip -d $ASTER_BUILD/SRC/astk-1.13.10-1.tar.gz && tar -uvf $ASTER_BUILD/SRC/astk-1.13.10-1.tar ./SRC/configuration/util.py && gzip $ASTER_BUILD/SRC/astk-1.13.10-1.tar
#RUN python setup.py install --prefix=$ASTER_INSTALL --noprompt
#RUN echo "vers : STABLE:/opt/aster/13.4/share/aster" >> /opt/aster/etc/codeaster/aster
#RUN rm -Rf $ASTER_BUILD

RUN apt install -y code-aster && sed -i 's/mpi_get_procid_cmd : echo \$OMPI_MCA_orte_ess_vpid/mpi_get_procid_cmd : echo \$OMPI_COMM_WORLD_RANK/g' /etc/codeaster/asrun

#ENV VEGA_DIR=/usr/local/src/vegapp
#RUN git clone https://github.com/Alneos/vega.git $VEGA_DIR
#WORKDIR $VEGA_DIR/build
RUN addgroup --group --gid 1777 vega && useradd --shell /bin/bash --uid 1777 --gid 1777 --create-home --groups sudo vega && echo "vega ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/vega
#RUN chown -R vega:vega $VEGA_DIR
#RUN echo 'export PATH=$ASTER_INSTALL/bin:$PATH' >> /home/vega/.bashrc
USER vega
#RUN cmake -DCMAKE_BUILD_TYPE=Debug .. && make -j