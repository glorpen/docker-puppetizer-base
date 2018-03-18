FROM glorpen/puppetizer-base:centos-latest

ARG IMAGE_VERSION=centos-db-latest

LABEL org.label-schema.version=$IMAGE_VERSION

RUN /bin/sh /opt/puppetizer/share/provision.sh puppet=no bolt=no puppetdb=yes
