FROM centos:centos7.4.1708

ARG PUPPET_VERSION=5.5.4

LABEL maintainer="Arkadiusz Dzięgiel <arkadiusz.dziegiel@glorpen.pl>" \
      org.label-schema.name="pupetizer-base" \
      org.label-schema.description="Base for puppetized Docker images" \
      org.label-schema.vcs-url="https://github.com/glorpen/docker-puppetizer-base" \
      org.label-schema.schema-version="1.0"

ADD opt /opt/puppetizer/

RUN /bin/sh /opt/puppetizer/share/provision.sh bolt=no puppetdb=no os=centos
ADD puppet /var/opt/puppetizer/vendor/puppetizer

ENTRYPOINT ["/opt/puppetizer/bin/puppetizerd"]
HEALTHCHECK CMD ["/opt/puppetizer/bin/health"]
