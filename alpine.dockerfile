FROM alpine:3.7

ARG IMAGE_VERSION=alpine-latest

LABEL maintainer="Arkadiusz Dzięgiel <arkadiusz.dziegiel@glorpen.pl>"
      org.label-schema.name="pupetizer-base" \
      org.label-schema.description="Base for puppetizet Docker images" \
      org.label-schema.version=$IMAGE_VERSION \
      org.label-schema.vcs-url="https://github.com/glorpen/docker-puppetizer-base" \
      org.label-schema.schema-version="1.0"

ADD opt /opt/puppetizer/

RUN /bin/sh /opt/puppetizer/share/provision.sh bolt=no puppetdb=no os=alpine
ADD puppet /var/opt/puppetizer/vendor/puppetizer

ENTRYPOINT ["/opt/puppetizer/bin/puppetizerd"]
HEALTHCHECK CMD ["/opt/puppetizer/bin/health"]
