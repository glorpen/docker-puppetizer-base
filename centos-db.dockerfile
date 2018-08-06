FROM glorpen/puppetizer-base:centos-latest

RUN /bin/sh /opt/puppetizer/share/provision.sh puppet=no bolt=no puppetdb=yes
