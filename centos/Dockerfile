FROM centos:centos7.4.1708
LABEL maintainer="Arkadiusz Dzięgiel <arkadiusz.dziegiel@glorpen.pl>"

ADD opt /opt/puppetizer/

ARG BOLT=no
ARG PUPPETDB=no

RUN /bin/sh /opt/puppetizer/share/provision.sh bolt=$BOLT puppetdb=$PUPPETDB os=centos
ADD puppet /var/opt/puppetizer/vendor/puppetizer

#ssl

ENTRYPOINT ["/opt/puppetizer/bin/puppetizerd"]

HEALTHCHECK CMD ["/opt/puppetizer/bin/health"]
