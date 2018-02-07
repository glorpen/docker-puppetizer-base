FROM centos:centos7.4.1708
LABEL maintainer="Arkadiusz Dzięgiel <arkadiusz.dziegiel@glorpen.pl>"

ADD opt /opt/puppetizer/

RUN /bin/bash /opt/puppetizer/share/provision.sh
ADD puppet /var/opt/puppetizer/vendor/puppetizer

#ssl

ENTRYPOINT ["/opt/puppetizer/bin/puppetizerd"]

HEALTHCHECK CMD ["/opt/puppetizer/bin/health"]
