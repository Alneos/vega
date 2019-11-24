FROM alpine:20191114
#FROM frolvlad/alpine-glibc:alpine-3.6

MAINTAINER Alter Ego Engineering <contact@aego.ai>

RUN apk add --update --no-cache libexecinfo-dev make gettext gfortran g++ py3-numpy python3-dev zlib-dev bison flex cmake swig lapack-dev curl xterm vim ca-certificates tk boost-dev bash

WORKDIR /root
#COPY ./dockerdata/aster-full-src-14.4.0-1.noarch.tar.gz /root
#RUN tar -xzf /root/aster-full-src-14.4.0-1.noarch.tar.gz && rm /root/aster-full-src-14.4.0-1.noarch.tar.gz
RUN curl -SLk https://www.code-aster.org/FICHIERS/aster-full-src-14.4.0-1.noarch.tar.gz | tar -xzC /root
ENV ASTER_BUILD=/root/aster-full-src-14.4.0
ENV ASTER_INSTALL=/opt/aster
WORKDIR $ASTER_BUILD
# Fixing glibc problems
#RUN tar xzf $ASTER_BUILD/SRC/mfront-3.2.1-1.tar.gz && sed -i 's/HAVE_FENV/__GLIBC__/g' mfront-3.2.1/mtest/src/MTestMain.cxx && tar czf $ASTER_BUILD/SRC/mfront-3.2.1-1.tar.gz mfront-3.2.1 && rm -Rf mfront-3.2.1
#RUN tar xzf $ASTER_BUILD/SRC/aster-14.4.0.tgz && sed -i 's/GNU_LINUX/__GLIBC__/g' aster-14.4.0/bibc/utilitai/hpalloc.c aster-14.4.0/bibc/utilitai/inisig.c && sed -i 's/HAVE_BACKTRACE/__GLIBC__/g' aster-14.4.0/bibc/utilitai/debugging.c  && sed -i 's/_DISABLE_MATHLIB_FPE/__GLIBC__/g' aster-13.4.0/bibc/utilitai/matfpe.c && tar czf $ASTER_BUILD/SRC/aster-13.4.0.tgz aster-13.4.0 && rm -Rf aster-13.4.0
#RUN tar xzf $ASTER_BUILD/SRC/metis-5.1.0-aster1.tar.gz && sed -i 's/HAVE_EXECINFO_H/__GLIBC__/g' metis-5.1.0/GKlib/error.c && tar czf $ASTER_BUILD/SRC/metis-5.1.0-aster1.tar.gz metis-5.1.0 && rm -Rf metis-5.1.0
RUN python3 setup.py install --prefix=$ASTER_INSTALL --noprompt && rm -Rf /opt/aster/14.4/share/aster/tests
ENV MFRONT_INSTALL=$ASTER_INSTALL/mfront-3.2.1
ENV MFRONT=$MFRONT_INSTALL/bin/mfront
ENV TFEL_CONFIG=$MFRONT_INSTALL/bin/tfel-config
ENV PATH=$PATH:$MFRONT_INSTALL/bin

RUN echo "vers : stable:/opt/aster/14.4/share/aster" >> $ASTER_INSTALL/etc/codeaster/aster

RUN curl -SLk https://github.com/ldallolio/NASTRAN-95/archive/v0.1.95.tar.gz | tar -xzC /root
ENV NASTRAN_BUILD=/root/NASTRAN-95-0.1.95
ENV NASTRAN_INSTALL=/opt/nastran
WORKDIR $NASTRAN_BUILD
RUN ./bootstrap && ./configure --prefix=$NASTRAN_INSTALL && make && make install
RUN mv $NASTRAN_INSTALL/bin/nastran $NASTRAN_INSTALL/bin/nast-run && cp -R ./rf  $NASTRAN_INSTALL/share
COPY ./dockerdata/nastran $NASTRAN_INSTALL/bin

FROM alpine:20191114
RUN apk add --update --no-cache g++ git sudo libexecinfo-dev make gettext-libs libgfortran py3-numpy python3 python3-dev zlib-dev cmake lapack boost-dev bash valgrind-dev libunwind-dev doxygen graphviz ttf-ubuntu-font-family ttf-freefont && apk del python3-dev

WORKDIR /opt/aster
COPY --from=0 /opt/aster .
WORKDIR /opt/nastran
COPY --from=0 /opt/nastran .

ENV ASTER_INSTALL=/opt/aster
ENV NASTRAN_INSTALL=/opt/nastran
ENV MED_INSTALL=$ASTER_INSTALL/public/med-4.0.0
ENV HDF_INSTALL=$ASTER_INSTALL/public/hdf5-1.10.3
ENV CXXFLAGS="-isystem $MED_INSTALL/include -isystem $HDF_INSTALL/include"
RUN sudo ln -s $ASTER_INSTALL/bin/as_run /usr/local/bin
ENV HDF5_ROOT=$HDF_INSTALL

RUN addgroup -g 1777 vega && adduser -s /bin/bash -u 1777 -G vega -D vega && echo "vega ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers && echo 'export PATH=$ASTER_INSTALL/bin:$NASTRAN_INSTALL/bin:$PATH' >> /home/vega/.bashrc
USER vega
WORKDIR /home/vega
