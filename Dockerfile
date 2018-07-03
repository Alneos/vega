FROM alpine:latest
#FROM frolvlad/alpine-glibc:alpine-3.6

MAINTAINER Alter Ego Engineering <contact@aego.ai>

RUN echo "@testing http://nl.alpinelinux.org/alpine/edge/testing" >> /etc/apk/repositories && apk add --update --no-cache libexecinfo-dev make gettext gfortran g++ py-pip python-dev zlib-dev bison flex cmake swig lapack-dev curl xterm vim ca-certificates tk boost-dev bash openmpi@testing openmpi-dev@testing && pip install numpy==1.9

WORKDIR /root
#COPY ./dockerdata/aster-full-src-13.4.0-3.noarch.tar.gz /root
#RUN tar -xzf /root/aster-full-src-13.4.0-3.noarch.tar.gz && rm /root/aster-full-src-13.4.0-3.noarch.tar.gz
RUN curl -SLk https://www.code-aster.org/FICHIERS/aster-full-src-13.4.0-3.noarch.tar.gz | tar -xzC /root
ENV ASTER_BUILD=/root/aster-full-src-13.4.0
ENV ASTER_INSTALL=/opt/aster
WORKDIR $ASTER_BUILD
# Fixing glibc problems
RUN tar xzf $ASTER_BUILD/SRC/mfront-3.0.0-1.tar.gz && sed -i 's/HAVE_FENV/__GLIBC__/g' mfront-3.0.0/mtest/src/MTestMain.cxx && tar czf $ASTER_BUILD/SRC/mfront-3.0.0-1.tar.gz mfront-3.0.0 && rm -Rf mfront-3.0.0
RUN tar xzf $ASTER_BUILD/SRC/aster-13.4.0.tgz && sed -i 's/GNU_LINUX/__GLIBC__/g' aster-13.4.0/bibc/utilitai/hpalloc.c aster-13.4.0/bibc/utilitai/inisig.c && sed -i 's/HAVE_BACKTRACE/__GLIBC__/g' aster-13.4.0/bibc/utilitai/debugging.c  && sed -i 's/_DISABLE_MATHLIB_FPE/__GLIBC__/g' aster-13.4.0/bibc/utilitai/matfpe.c && tar czf $ASTER_BUILD/SRC/aster-13.4.0.tgz aster-13.4.0 && rm -Rf aster-13.4.0
RUN tar xzf $ASTER_BUILD/SRC/metis-5.1.0-aster1.tar.gz && sed -i 's/HAVE_EXECINFO_H/__GLIBC__/g' metis-5.1.0/GKlib/error.c && tar czf $ASTER_BUILD/SRC/metis-5.1.0-aster1.tar.gz metis-5.1.0 && rm -Rf metis-5.1.0
RUN python setup.py install --prefix=$ASTER_INSTALL --noprompt && rm -Rf /opt/aster/13.4/share/aster/tests
ENV MFRONT_INSTALL=$ASTER_INSTALL/mfront-3.0.0
ENV MFRONT=$MFRONT_INSTALL/bin/mfront
ENV TFEL_CONFIG=$MFRONT_INSTALL/bin/tfel-config
ENV PATH=$PATH:$MFRONT_INSTALL/bin

RUN echo "vers : STABLE:/opt/aster/13.4/share/aster" >> $ASTER_INSTALL/etc/codeaster/aster

FROM alpine:latest
RUN echo "@testing http://nl.alpinelinux.org/alpine/edge/testing" >> /etc/apk/repositories &&  apk add --update --no-cache g++ git sudo libexecinfo-dev make gettext-libs libgfortran py-pip python2 python2-dev zlib-dev cmake lapack boost-dev bash openmpi@testing openmpi-dev@testing valgrind-dev && pip install numpy==1.9 && apk del py-pip python2-dev 

WORKDIR /opt/aster
COPY --from=0 /opt/aster .

ENV ASTER_INSTALL=/opt/aster
ENV MED_INSTALL=$ASTER_INSTALL/public/med-3.2.1
ENV HDF_INSTALL=$ASTER_INSTALL/public/hdf5-1.8.14
ENV CXXFLAGS="-isystem $MED_INSTALL/include -isystem $HDF_INSTALL/include"
RUN ln -s $MED_INSTALL/lib/libmed.so.1.8.0 /usr/lib/libmed.so && ln -s $MED_INSTALL/lib/libmedC.so.1.8.0 /usr/lib/libmedC.so && ln -s $MED_INSTALL/lib/libmedimport.so.1.8.0 /usr/lib/libmedimport.so && ln -s $MED_INSTALL/lib/libmed.so.1.8.0 /usr/lib/libmed.so.1 && ln -s $MED_INSTALL/lib/libmedC.so.1.8.0 /usr/lib/libmedC.so.1 && ln -s $HDF_INSTALL/lib/libhdf5.so.9.0.0 /usr/lib/libhdf5.so && ln -s $HDF_INSTALL/lib/libhdf5_hl.so.9.0.0 /usr/lib/libhdf5_hl.so && ldconfig /usr/lib
ENV HDF5_ROOT=$HDF_INSTALL

RUN addgroup -g 1777 vega && adduser -s /bin/bash -u 1777 -G vega -D vega && echo "vega ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers && echo 'export PATH=$ASTER_INSTALL/bin:$PATH' >> /home/vega/.bashrc
USER vega
WORKDIR /home/vega
