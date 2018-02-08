FROM alpine:3.4
LABEL maintainer="Arkadiusz Dzięgiel <arkadiusz.dziegiel@glorpen.pl>"

ADD opt /opt/puppetizer/

RUN /bin/sh /opt/puppetizer/share/provision.sh bolt=no puppetdb=no os=alpine
ADD puppet /var/opt/puppetizer/vendor/puppetizer

ENTRYPOINT ["/opt/puppetizer/bin/puppetizerd"]
HEALTHCHECK CMD ["/opt/puppetizer/bin/health"]
