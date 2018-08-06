FROM alpine:3.7

ARG PUPPET_VERSION=5.5.3

LABEL maintainer="Arkadiusz Dzięgiel <arkadiusz.dziegiel@glorpen.pl>"

ADD opt /opt/puppetizer/

RUN /bin/sh /opt/puppetizer/share/provision.sh bolt=no puppetdb=no os=alpine
ADD puppet /var/opt/puppetizer/vendor/puppetizer

ENTRYPOINT ["/opt/puppetizer/bin/puppetizerd"]
HEALTHCHECK CMD ["/opt/puppetizer/bin/health"]

LABEL org.label-schema.name="pupetizer-base" \
      org.label-schema.description="Base for puppetized Docker images" \
      org.label-schema.vcs-url="https://github.com/glorpen/docker-puppetizer-base" \
      org.label-schema.schema-version="1.0"