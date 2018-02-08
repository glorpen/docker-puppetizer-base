FROM centos:centos7.4.1708
LABEL maintainer="Arkadiusz Dzięgiel <arkadiusz.dziegiel@glorpen.pl>"

ADD opt /opt/puppetizer/

RUN /bin/sh /opt/puppetizer/share/provision.sh bolt=no puppetdb=no os=centos
ADD puppet /var/opt/puppetizer/vendor/puppetizer

ENTRYPOINT ["/opt/puppetizer/bin/puppetizerd"]
HEALTHCHECK CMD ["/opt/puppetizer/bin/health"]
